# Design Document

## Overview

This feature makes `DataManager` the single owner of `SettingsData`. Today the six page-enable
flags and the map tile base URL are seeded with hardcoded defaults in `TaskManager::ReadSettings()`
and threaded by reference through `BasePage` and `GUINavigator` into every page. The design moves
that state into `DataManager`, loads it lazily from the existing `settings` file, persists it with
the same `key=value` mechanism that already stores `WIFI_SSID` / `WIFI_PWD`, and lets the rider pick
the active optional pages during Wi-Fi provisioning. The reference-passing through pages is removed:
every use site reads from `DataManager::GetInstance()->Settings()`.

The storage layer, the lazy-load pattern, the HTML form POST transport, and the per-platform mutex
are all reused as-is. No new storage format, no JSON body, no new thread touches the card.

## Architecture

```
Provisioning (Wi-Fi)                       Normal run
--------------------                       ----------
ConfigurationPage (HTML form)              TaskManager::CreatePages()
   | POST ssid/password + 6 page flags        | reads page flags
Server::OnSave                                 v
   | PersistCredentials()                   DataManager::Settings()  <-- cached SettingsData
   | PersistPageSelection()  -------------->    ^  lazy load on first call
   v                                            |  LoadSettings(): ReadSettingsFile + defaults
DataManager::WriteCredentials                   |
DataManager::WritePageSelection  -------------->+  WriteSettingsFile (key=value)
                                                |
                                   PageMapSync reads Settings().mapSyncingBaseUrl
```

`DataManager` gains a cached `SettingsData` and a `Settings()` accessor. The accessor is the single
source of truth; reads and writes both go through it. The existing `SettingsFileData` / `SettingsEntry`
list and the `ReadSettingsFile` / `WriteSettingsFile` pair carry every value, so credentials and
settings share one file and one code path.

## Components and Interfaces

### DataManager

New state and methods. The accessor returns a reference so callers can both read and mutate the one
cached instance (Requirement 1.4).

```cpp
class DataManager {
private:
    SettingsData settings_;
    bool settingsLoaded_{false};
    void LoadSettings();
    void ApplyEntry(const SettingsEntry &entry);
    static bool ParseBool(const std::string &value, bool fallback);
    static const char *BoolText(bool value);
    void UpsertEntry(SettingsFileData &fileData, const char *key, const std::string &value);
public:
    SettingsData &Settings();
    bool WritePageSelection(const SettingsData &selection);
};
```

- `Settings()` — on the first call runs `LoadSettings()` into `settings_`, sets `settingsLoaded_`,
  returns the reference; later calls return `settings_` without touching the card (Requirements 1.1,
  1.2, 1.3). The load is guarded by the existing `mutex_` so the first access from either thread is
  serialized, matching how `GetInstance()` and the queue already lock.
- `LoadSettings()` — seeds `settings_` with the defaults (all six page bools `true`,
  `mapSyncingBaseUrl` = `MAP_BASE_URL_DEFAULT`), then reads the file through the existing
  `ReadSettingsFile`. Each parsed entry overrides a default via `ApplyEntry`. An absent key keeps its
  default, which is exactly Requirements 1.5 and 1.6. If `ReadSettingsFile` fails or the file is
  absent, the defaults stand.
- `ApplyEntry(entry)` — maps one key to one `settings_` field. Page keys go through
  `ParseBool(value, currentDefault)`; the URL key is copied verbatim. Keeps the key→field mapping in
  one place (SRP), so `LoadSettings` stays an orchestrator.
- `WritePageSelection(selection)` — reads the current file into a `SettingsFileData`, upserts the six
  page keys and the URL key via `UpsertEntry`, then writes it back with `WriteSettingsFile`.
  `UpsertEntry` updates an existing entry in place or appends when absent, so `WIFI_SSID` / `WIFI_PWD`
  and any unrelated line are preserved and never duplicated (Requirements 2.1-2.4). This mirrors the
  existing `WriteCredentials` structure; the two share the upsert helper to stay DRY.
- `ParseBool` / `BoolText` — the single boolean encoding. `BoolText` emits `"true"` / `"false"`;
  `ParseBool` accepts `"true"` (case-insensitive) as `true`, anything else as the supplied fallback.
  Keeping both in one spot guarantees the write/read round-trip (Requirement 2.2).

New keys, declared next to the existing `WIFI_SSID_KEY` / `WIFI_PWD_KEY` in `DataManager.cpp`:

| key | field |
|---|---|
| `PAGE_ALTIMETRY` | `pageAltimetryEnabled` |
| `PAGE_DISTANCE` | `pageDistanceEnabled` |
| `PAGE_HILLS_GRAPH` | `pageHillsGraphEnabled` |
| `PAGE_MAP` | `pageMapEnabled` |
| `PAGE_ROUTE` | `pageRouteEnabled` |
| `PAGE_SUMMARY` | `pageSummaryEnabled` |
| `MAP_BASE_URL` | `mapSyncingBaseUrl` |

`MAP_BASE_URL_DEFAULT` is the literal `https://tile.openstreetmap.org`, defined once in
`DataManager.cpp`.

### TaskManager

- `ReadSettings()` is removed, along with its hardcoded defaults and the `settings_` member
  (Requirements 3.1, 3.2). The `// temporary, move to DataManager` call in `Execute()` goes away.
- `CreatePages()` reads each flag through `DataManager::GetInstance()->Settings()`. The page-cycle
  order is unchanged. The first `Settings()` call happens here on the provisioned path, triggering the
  lazy load.
- `TaskManager.hpp` drops `#include "SettingsData.hpp"` and the `SettingsData settings_;` member.

### BasePage and the page hierarchy

- `BasePage` loses its `PedalGuru::SettingsData& settings_;` member and its
  `BasePage(SettingsData&)` constructor (Requirement 3.3). It becomes default-constructible; the
  derived pages already use `using BasePage::BasePage`, so they inherit the default constructor with no
  further change.
- `GUINavigator` loses its `SettingsData settings_;` member and constructs every page with no
  argument: `std::make_unique<PageMap>()`, etc. (Requirement 3.4). `GUINavigator.hpp` no longer needs
  `SettingsData` visible.

### PageMapSync

`DrawPageContents` currently reads `settings_.mapSyncingBaseUrl`. It instead reads
`DataManager::GetInstance()->Settings().mapSyncingBaseUrl` (Requirement 3.5). `PageMapSync.cpp` gains
`#include "DataManager.hpp"`. No other behavior changes.

### ConfigurationPage

The inline `PAGE_TEMPLATE` HTML gains a fieldset of six checkboxes, one per optional page, placed
between the password input and the Save button. There is no checkbox for `PageMapSync`
(Requirements 4.1, 4.2). The form stays a standard `application/x-www-form-urlencoded` POST — the same
transport `FormBody` already decodes — so each checked box contributes its `name` to the body and an
unchecked box contributes nothing, which is the normal HTML checkbox semantic.

Checkbox field names match the persistence keys so no translation layer is needed:
`pageAltimetry`, `pageDistance`, `pageHillsGraph`, `pageMap`, `pageRoute`, `pageSummary`. All six
default to checked, matching the DataManager defaults so a rider who changes nothing keeps every page.

Client-side "at least one" guard (Requirement 4.4): a `submit` listener counts the checked page boxes
and, when zero, calls `event.preventDefault()` and writes a message into the existing `#status`
paragraph, reusing the element already present for scan feedback. This blocks the POST before it
leaves the browser; the server-side check below is the authority for clients with scripting disabled.

### Server::OnSave

`OnSave` is split so credential persistence and page-selection persistence are distinct
responsibilities (Requirement 5.3). Two private helpers are added to `Server`:

```cpp
bool PersistCredentials(const FormBody &body, HttpResponse *response);
bool PersistPageSelection(const FormBody &body, HttpResponse *response);
```

- `PersistCredentials` holds the existing ssid/password extraction, validation and
  `DataManager::WriteCredentials` call. It returns `false` after it has already written an error
  response, `true` on success. This is the current `OnSave` body, extracted verbatim.
- `PersistPageSelection` reads the six checkbox fields via `FormBody::Has`, counts the selected pages,
  and rejects an empty selection with a `400` client error without persisting anything (Requirement
  5.2, matching the client guard in 4.4). Otherwise it fills a `SettingsData` from the checkbox
  presence, carries the current `mapSyncingBaseUrl` through from `DataManager::GetInstance()->Settings()`,
  and calls `DataManager::WritePageSelection`, persisting each flag as a boolean (Requirement 5.1).
- `OnSave` becomes an orchestrator: validate selection, persist credentials, persist selection, then
  send the success page. Ordering runs the page-selection check first so an empty selection is rejected
  before any write, keeping a rejected save side-effect free.

`SettingsData` presence maps directly: `body.Has("pageMap")` → `pageMapEnabled`, and so on.

## Data Models

`SettingsData` (unchanged shape, `src/Model/SettingsData.hpp`): six `bool` page flags plus
`std::string mapSyncingBaseUrl`. One cached instance lives in `DataManager`.

`SettingsFileData` / `SettingsEntry` (unchanged, `src/Model/SettingsEntry.hpp`): the `key=value` line
list used for both read and write. Page flags serialize as `KEY=true` / `KEY=false`; the URL
serializes verbatim as `MAP_BASE_URL=<url>`.

Memory: the only new persistent state is one `SettingsData` in the singleton (six bytes of bool plus
one short string) and the `settingsLoaded_` flag. The file is read into a transient `SettingsFileData`
only during load and write, then released — no steady-state growth.

## Error Handling

- First `Settings()` call when the file is missing or `ReadSettingsFile` fails: defaults stand, no
  error surfaced. Settings are advisory, so a missing file is a normal first-boot state, not a fault.
- `WritePageSelection` returns the `WriteSettingsFile` boolean. `PersistPageSelection` maps a `false`
  to a `500` response, matching how `WriteCredentials` failure is already reported.
- Empty page selection: rejected with `400` on both the client (blocked submit) and the server
  (no persistence), so a scripting-disabled client cannot bypass it.
- The `ReadCredentials` recovery path is unchanged (see Requirement 6 note below).

## Requirement 6: stale comment verification

The comment above the `ReadSettingsFile` call in `ReadCredentials` reads:
`/** fs.ll unmounts the volume when OpenFile fails, and later card access depends on it. */`, followed
by a `MountSdCard()` + `SelectActiveDrive()` recovery on failure.

Verification performed against the submodule: `fs.ll/src/lib/FileSystem.c` `OpenFile` calls
`f_unmount(SD_DRIVE)` when `f_open` fails, and `fs.ll/AGENTS.md` §4 documents the same ("`OpenFile`
call `f_unmount` on failure"). pedal.guru's own `AGENTS.md` records this as `TODO-B13`. The comment is
therefore **accurate**, not stale.

Per Requirement 6.3, the comment and the recovery code are retained unchanged, and the AGENTS.md note
is not removed. Requirement 6.2 (removal) does not apply because its `WHERE the comment is confirmed
stale` precondition is false. The design records the verification outcome; no code change is made for
Requirement 6.

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a
system — a formal statement about what the system should do.*

### Property 1: Cache retains values written through the reference

*For any* `SettingsData` field value assigned through the reference returned by `Settings()`, a
subsequent `Settings()` call returns a reference reporting that same value, without re-reading the
file.

**Validates: Requirements 1.3, 1.4**

### Property 2: Absent keys fall back to defaults

*For any* settings file content, after loading, every page key absent from the file yields `true`,
the map URL yields `MAP_BASE_URL_DEFAULT` when its key is absent, and every key present in the file
yields its stored value.

**Validates: Requirements 1.2, 1.5, 1.6**

### Property 3: Persistence round-trip preserves settings

*For any* `SettingsData`, persisting it through `WritePageSelection` and then loading via `LoadSettings`
produces an equal `SettingsData`, with each page flag encoded and decoded as a boolean.

**Validates: Requirements 2.1, 2.2**

### Property 4: Write upserts without duplicating or dropping entries

*For any* initial settings file, after `WritePageSelection` each written key appears exactly once, and
every unrelated entry — including `WIFI_SSID` and `WIFI_PWD` — retains its original value.

**Validates: Requirements 2.3, 2.4**

### Property 5: Form checkbox presence maps to persisted flags

*For any* save request with at least one page checkbox present, the `SettingsData` persisted by
`OnSave` has each page flag equal to the presence of its checkbox field in the submitted form body.

**Validates: Requirements 5.1**

### Property 6: Empty page selection is rejected without side effect

*For any* save request carrying no page checkbox fields, `OnSave` responds with a client-error status
and leaves the settings file unchanged.

**Validates: Requirements 5.2**
