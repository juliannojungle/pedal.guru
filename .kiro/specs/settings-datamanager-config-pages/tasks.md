# Implementation Plan: Settings in DataManager and Provisioning Page Selection

## Overview

Move `SettingsData` ownership into `DataManager` with a lazy `Settings()` accessor backed by the
existing `key=value` settings file, then migrate every use site off the by-reference passing through
`BasePage` / `GUINavigator`, and finally expose the six optional pages as checkboxes in the Wi-Fi
provisioning flow. Tasks are ordered so the Simulator build stays green at every step: DataManager
gains the owned state first, then consumers migrate to the accessor, then the provisioning side is
wired. Per AGENTS.md §8 there are no test tasks; verification is a clean Simulator build plus observed
behavior. No new code comments are added.

## Tasks

- [x] 1. Give DataManager ownership of SettingsData
  - [x] 1.1 Add settings state, keys, and defaults to DataManager
    - In `DataManager.hpp`, add `SettingsData settings_;` and `bool settingsLoaded_{false};`, and
      include `SettingsData.hpp`.
    - In `DataManager.cpp`, declare the seven new keys (`PAGE_ALTIMETRY`, `PAGE_DISTANCE`,
      `PAGE_HILLS_GRAPH`, `PAGE_MAP`, `PAGE_ROUTE`, `PAGE_SUMMARY`, `MAP_BASE_URL`) next to the
      existing `WIFI_SSID_KEY` / `WIFI_PWD_KEY`, and define `MAP_BASE_URL_DEFAULT` as the literal
      `https://tile.openstreetmap.org`.
    - _Requirements: 1.1, 2.1_

  - [x] 1.2 Add the boolean encoding and entry helpers
    - Add `static bool ParseBool(const std::string&, bool fallback)` accepting case-insensitive
      `"true"` as true and the fallback otherwise, and `static const char *BoolText(bool)` emitting
      `"true"` / `"false"`.
    - Add `void UpsertEntry(SettingsFileData&, const char *key, const std::string &value)` that
      updates an entry in place or appends when absent.
    - _Requirements: 2.2, 2.3, 2.4_

  - [x] 1.3 Implement lazy load and the Settings accessor
    - Add `void LoadSettings()` that seeds `settings_` with defaults (six page bools `true`,
      `mapSyncingBaseUrl` = `MAP_BASE_URL_DEFAULT`), reads the file via `ReadSettingsFile`, and
      overrides each default through `ApplyEntry`; defaults stand when the file is absent or the read
      fails.
    - Add `void ApplyEntry(const SettingsEntry&)` mapping one key to one `settings_` field (page keys
      via `ParseBool(value, currentDefault)`, URL copied verbatim).
    - Add `SettingsData &Settings()` that on first call runs `LoadSettings()` under `mutex_`, sets
      `settingsLoaded_`, and returns the cached reference on every call.
    - _Requirements: 1.1, 1.2, 1.3, 1.4, 1.5, 1.6_

  - [x] 1.4 Implement WritePageSelection
    - Add `bool WritePageSelection(const SettingsData &selection)` that reads the current file into a
      `SettingsFileData`, upserts the six page keys (encoded with `BoolText`) and `MAP_BASE_URL` via
      `UpsertEntry`, then writes back with `WriteSettingsFile`, returning its boolean result.
    - _Requirements: 2.1, 2.2, 2.3, 2.4_

- [x] 2. Migrate normal-run use sites to the accessor
  - [x] 2.1 Read page flags from DataManager in TaskManager
    - In `CreatePages()`, read each page-enable flag through `DataManager::GetInstance()->Settings()`,
      keeping the existing page-cycle order.
    - Remove `ReadSettings()`, the hardcoded defaults, the `settings_` member, its call in
      `Execute()`, and the `SettingsData.hpp` include from `TaskManager.hpp`.
    - _Requirements: 3.1, 3.2_

  - [x] 2.2 Drop SettingsData from BasePage
    - Remove `BasePage`'s `SettingsData& settings_` member and its `BasePage(SettingsData&)`
      constructor so the class is default-constructible; derived pages keep `using BasePage::BasePage`.
    - _Requirements: 3.3_

  - [x] 2.3 Default-construct pages in GUINavigator
    - Remove `GUINavigator`'s `SettingsData settings_` member, construct every page with no argument
      (`std::make_unique<PageMap>()`, etc.), and drop the now-unneeded `SettingsData` visibility from
      `GUINavigator.hpp`.
    - _Requirements: 3.4_

  - [x] 2.4 Read the base URL from the accessor in PageMapSync
    - In `DrawPageContents`, read `DataManager::GetInstance()->Settings().mapSyncingBaseUrl` and add
      `#include "DataManager.hpp"`.
    - _Requirements: 3.5_

- [x] 3. Checkpoint - Simulator build is green after migration
  - Run `cmake -B build -DPLATFORM_NAME=Simulator && cmake --build build`; ask the user if questions
    arise.

- [x] 4. Expose optional pages in provisioning
  - [x] 4.1 Add the page checkboxes to ConfigurationPage
    - In the inline `PAGE_TEMPLATE`, add a fieldset of six checkboxes between the password input and
      the Save button, named `pageAltimetry`, `pageDistance`, `pageHillsGraph`, `pageMap`,
      `pageRoute`, `pageSummary`, all defaulting to checked. No checkbox for `PageMapSync`.
    - _Requirements: 4.1, 4.2, 4.3_

  - [x] 4.2 Add the client-side at-least-one guard
    - Add a `submit` listener that counts checked page boxes and, when zero, calls
      `event.preventDefault()` and writes a prompt into the existing `#status` paragraph.
    - _Requirements: 4.4_

  - [x] 4.3 Split Server::OnSave into credential and selection persistence
    - Add `bool PersistCredentials(const FormBody&, HttpResponse*)` holding the existing ssid/password
      extraction, validation, and `DataManager::WriteCredentials` call verbatim.
    - Add `bool PersistPageSelection(const FormBody&, HttpResponse*)` that reads the six checkbox
      fields via `FormBody::Has`, rejects an empty selection with a `400` and no write, otherwise
      fills a `SettingsData` from checkbox presence, carries the current `mapSyncingBaseUrl` from
      `DataManager::GetInstance()->Settings()`, and calls `WritePageSelection` (mapping `false` to
      `500`).
    - Make `OnSave` an orchestrator: check selection first, persist credentials, persist selection,
      then send the success page.
    - _Requirements: 5.1, 5.2, 5.3_

- [x] 5. Record the Requirement 6 comment verification
  - [x] 5.1 Confirm the ReadCredentials comment is accurate and retain it
    - Confirm against `fs.ll/src/lib/FileSystem.c` (`OpenFile` calls `f_unmount` on `f_open` failure)
      that the comment above the `ReadSettingsFile` call in `ReadCredentials` is accurate; retain the
      comment and the recovery code unchanged and make no removal.
    - _Requirements: 6.1, 6.3_

- [x] 6. Final checkpoint - Simulator build is green and page selection observed
  - Run `cmake -B build -DPLATFORM_NAME=Simulator && cmake --build build` and verify the provisioning
    page selection round-trips; ask the user if questions arise.

## Notes

- No test tasks are included, per AGENTS.md §8. The correctness properties in design.md are
  specification-only; verification is end-to-end (clean Simulator build plus observed behavior).
- No new code comments are added while implementing (AGENTS.md §5).
- All hardware access stays in hal.ll; this feature is C++ app-layer only (AGENTS.md §6, §7).
- Each task references the specific requirement sub-clauses it satisfies for traceability.
- Requirement 6 is verification-only: the comment is accurate, so there is no removal task.

## Task Dependency Graph

```json
{
  "waves": [
    { "id": 0, "tasks": ["1.1"] },
    { "id": 1, "tasks": ["1.2"] },
    { "id": 2, "tasks": ["1.3", "1.4"] },
    { "id": 3, "tasks": ["2.1", "2.2", "2.4", "4.1", "4.3", "5.1"] },
    { "id": 4, "tasks": ["2.3", "4.2"] }
  ]
}
```
