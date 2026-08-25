# AGENTS.md

Working notes for AI agents and new contributors on **pedal.guru**.

---

## Ground rules

Read this section before anything else. These are hard rules, not preferences.

### 1. Everything written to this repository is in English

- **English only, everywhere in the repository**: code, identifiers, string literals, comments,
  commit messages, `README.md`, this file, spec documents, and any other artifact. Not a single
  variable or comment in another language.
- **The chat is separate.** Julianno, the lead dev, prefers to talk to the agent in pt-BR. That says
  nothing about what gets written to disk: the conversation can be in pt-BR while every byte
  committed stays in English.
- If you find non-English text anywhere outside the chat, **stop and ask the dev** whether it is
  intentional. Do not translate it silently, and do not assume it is a mistake.

### 2. The dev takes the decisions, never the agent

If there is a decision to be taken — an architectural choice, a naming choice, a trade-off, an
ambiguity in a requirement, a question of scope — **ask the dev**. Do not decide and proceed, and do
not present a decision as if it had already been made.

### 3. Do not make assumptions

Only two things count as true: what can be **validated in the code** and what the **dev states
explicitly**. Everything else is a question, not a fact. When this document states something that was
not verified, it says so (see §15).

### 4. Nothing here is a final architectural decision

This project is a learning exercise for the dev and the agent alike. The documentation and the design
evolve with the code. Everything can be questioned, and **should** be questioned when it looks wrong,
incoherent, or out of line with common industry practice. If you see an opportunity for improvement,
**propose it to the dev** — do not implement it unilaterally, and do not stay quiet about it either.

### 5. Do not write tests

**Confirmed by the dev.** Do not add unit tests, property-based tests, test harnesses, mocking or stub
layers, or any test build system. Do not create or expand a `test/` directory, and do not vendor a
testing library (Unity, CMocka, Theft, GoogleTest, …).

The reason is specific and worth stating plainly: the dev does not like the way unit testing is done in
C/C++. Verification here is **end-to-end**: build, run on the Simulator or on the real device, and
validate regressions manually. A clean compile plus observed behaviour is the acceptance criterion.

This is today's stance, not a dogma — the dev is open to being convinced otherwise in the future. Until
that happens, treat it as a hard rule. When a spec's task plan lists test tasks, treat them as skipped
and out of scope unless the dev says otherwise for that session.

The same policy is in force in fs.ll and gui.ll.

---

## 1. What this project is

**pedal.guru** is an open-source cycling computer. The name has two halves, and both are part of the
product goal:

- **pedal** — the ride. It shows the cyclist a general picture of the training session: altimetry,
  distance, graphs of the climbs already done and the ones still ahead, a colour navigation map, the
  planned route, an overall summary, and more screens still to come. See `src/GUI/Page` for the set
  of screens currently foreseen.
- **guru** — the coach. Beyond displaying numbers, the system is meant to act as a trainer: remind
  the rider to drink, to eat, to change cadence according to the stretch of road or the elapsed
  training time, and so on. *(Stated by the dev; not implemented yet — see §13.)*

The differentiator against off-the-shelf products is a **colour map**: pedal.guru downloads
OpenStreetMap tiles for a user-configured region ahead of time and renders them **offline** during
the ride.

pedal.guru is the **application layer**. It is meant to know nothing about the platform it runs on;
everything platform-specific belongs to the reusable submodule collection described in §2.

## 2. Position in the dot-ll-collection

The infrastructure the application needs is being split out into one repository per reusable layer.
The dev calls this set the **dot-ll-collection** ("ll" for low level). Each one is a small library
with its own repository, its own `AGENTS.md`, and its own cmake build contract.

Existing, consumed by pedal.guru today (declared in `.gitmodules`):

| submodule | path | role | reference |
|---|---|---|---|
| **fs.ll** | `src/Dependency/fs.ll` | file system on the SD card wired to the hardware (wraps FatFs) | `src/Dependency/fs.ll/AGENTS.md` |
| **gui.ll** | `src/Dependency/gui.ll` | drawing on the LCD panel or on the simulator window | `src/Dependency/gui.ll/AGENTS.md` |

**Read both of those `AGENTS.md` files.** They carry the conventions, the build-contract mechanics,
the hardware pinout decisions and the known traps that pedal.guru inherits. This file does not repeat
them.

### Why git submodules here, and not the `.cmake` contract

**Decided by the dev.** pedal.guru consumes fs.ll and gui.ll as **git submodules**, on purpose, so that
**GitHub reports the dependency between the repositories** in its reference/usage statistics. A
directory populated at configure time by a `git clone` inside a `.cmake` file is invisible to GitHub;
a submodule is not. That visibility is the deciding factor here.

This is a deliberate divergence from gui.ll's Decision 16, which drops sibling libraries from
`.gitmodules` so that several consumers can share one checkout. Both positions are valid — they
optimize for different things (shared checkout vs. visible dependency graph), and pedal.guru, being the
top-level application rather than a library, chooses visibility.

Consequence to be aware of: pedal.guru **still carries leftover copies** of `src/Dependency/fs.ll.cmake`
and `src/Dependency/gui.ll.cmake` from the earlier contract-based approach. They are still included by
`pedal.guru.cmake` and they still work, because each one's default path resolves onto the submodule
folder that is already populated, so nothing gets downloaded. They are residue of the change of
approach and the dev will clean them up later — see `TODO-C1`. Do not remove them on your own
initiative: the build currently depends on them for the source and include lists.

Planned, still living inside pedal.guru and expected to move out (stated by the dev, names not
final): `net.ll` for networking, something like `thread.ll` for timers and threading, something like
`serial.ll` for serial I/O. Today those live under `src/Platform` (§9). A good deal of code in
pedal.guru was written directly here to validate an idea quickly, or predates the submodule idea at
all, and is therefore a migration candidate — see §13.

## 3. Target hardware

pedal.guru runs on **three platforms from a single code base**:

| | Simulator | RP2040 | ESP32 |
|---|---|---|---|
| host | Linux desktop, or Windows via WSL | Waveshare [RP2040-LCD-1.28](https://www.waveshare.com/wiki/RP2040-LCD-1.28) | Waveshare [ESP32-S3-LCD-1.28](https://www.waveshare.com/wiki/ESP32-S3-LCD-1.28) |
| toolchain | host gcc/clang + SDL2 + OpenSSL | arm-none-eabi-gcc + pico-sdk | xtensa-esp32-elf-gcc + ESP-IDF |
| output | `build/pedal.guru` | `.uf2` | `.bin` |

Both boards are 1.28" round colour LCDs (240×240, GC9A01) with the MCU on the same board, which is
the point: to deploy, a user buys the device and flashes the code. Both expose the **same proprietary
2×20 pin 1.27mm expansion header**, and the LCD is internally wired on both.

The project plan (stated by the dev) is a **single, very simple open-hardware expansion board** shared
by both MCUs, carrying:

- a **microSD card slot**;
- **two reed switches**, so navigation is done with a magnet fixed to a ring that rotates around the
  round display — magnetism instead of physical buttons is what allows a fully **waterproof, sealed
  case** with no mechanical access to the electronics;
- a **GPS** module;
- for the RP2040 version only, a **WiFi + BT module** (the ESP32-S3 has wireless natively). The board
  is one single design for both; the wireless module is simply populated or not.

Because the board must work on both MCUs, **only the physical header pins that are equivalent between
the two boards may be used**. gui.ll's `AGENTS.md` (Design Decision 7) has the overlay analysis: 9
overlapping physical pins total, 5 already assigned to SD SPI + card detect, one excluded because it
lands on an ESP32-S3 strapping pin, leaving 3 fully usable plus 2 with an SWD caveat. Do not invent
pin assignments — that table and each platform's `HALConfig.h` in gui.ll are the source of truth, and
some values there are hardware-validated.

### Why no touchscreen, ever

This is a settled product decision, not an omission. In the rain a capacitive touchscreen is
unusable: every drop registers as an uncontrolled touch. The device is handlebar-mounted and will be
hit by rain, dew and the rider's sweat. Hence the two requirements: **waterproof** and **magnetic
navigation**. Do not propose touch input.

## 4. Repository layout

```
CMakeLists.txt                  root cmake, branches on PLATFORM_NAME (§10)
pedal.guru.cmake                the source/include list for the application (§10)
AGENTS.md                       this file
README.md                       user-facing: BOM, wiring, toolchain setup, build tasks
DIAGRAM.md                      mermaid class diagram (stale, see §14)
Documentation/Image/            pinout and device reference images
Toolchain/                      per-platform environment setup + RP2040 USB/flash helpers
sample/sdcard/                  content that goes into the simulator disk image
sample/sdcard.img               FAT image standing in for the physical card on Simulator
src/
  PedalGuru.cpp                 entry point (app_entry -> main or app_main)
  TaskManager.{cpp,hpp}         creates the two threads, the devices and the pages
  DataManager.{cpp,hpp}         singleton, shares sensor data between the two threads
  HIDHandler.{cpp,hpp}          user input events (magnetic ring) -> registered callbacks
  API/OpenStreetMapAPI.{cpp,hpp}  tile math, tile listing, tile download and cache path
  Device/                       one folder per integrated device (§5)
  Sensor/                       one file per supported sensor type (§5)
  GUI/
    GUIDrawer.{cpp,hpp}         owns the render loop and the current page's callbacks
    GUINavigator.{cpp,hpp}      turns HID events into page changes
    Page/                       BasePage + one class per screen (§6)
    Render/                     Window / Texture / Area / Color — the C++ face of gui.ll
  Helper/TextHelper.{cpp,hpp}   tokenizer used by the NMEA parsing
  Model/                        plain data types (GPSFixData, MapTile, MapGrid, Settings, ...)
  Platform/<Platform>/          the platform seam owned by pedal.guru (§9)
  Dependency/
    fs.ll/                      submodule
    gui.ll/                     submodule
    fs.ll.cmake                 copy of fs.ll's build contract
    gui.ll.cmake                copy of gui.ll's build contract
    pico_sdk_import.cmake       stock pico-sdk locator
```

## 5. Extensibility: Device and Sensor

The structure is designed so new features, modules and sensors drop in without touching the core.

- **`src/Sensor`** holds the supported **sensor types**. `Sensor.hpp` is the abstract base and defines
  what every sensor must implement: `Enable()`, `Disable()`, `Enabled()`, `GetData()`. Today only
  `GPS` exists.
- **`src/Device`** holds the **devices** to be integrated. `Device.hpp` is the abstract base:
  `Connect()`, `Disconnect()`, `Connected()`, `GetData()`. A device owns **at least one** sensor
  (`std::list<std::unique_ptr<Sensor>> sensors_`).

The point of the split is that sensors are **reused across devices**. The dev's example: a device
clamped to the pedal measures cadence; the same device clamped to the wheel measures speed, just by
changing its configuration — so that device would carry both a cadence sensor and a speed sensor.
*(That kind of device is not supported yet.)*

Existing:

- `Device/Generic/LocationModule/` — a generic location module; owns one `GPS` sensor. This is the
  only device `TaskManager::CreateDevices()` instantiates today.
- `Device/DIY/wired_reel_speedometer.cpp` — a mock for the classic bicycle speedometer (reed switch on
  the fork, magnet on the spokes). It is **not in the build** (absent from `pedal.guru.cmake`), it
  declares the class inside the `.cpp` with no header, and it does not implement all of `Device`'s
  pure virtuals, so it cannot be instantiated as it stands. Its file name is also `snake_case`, which
  breaks the PascalCase convention (§12). Left alone deliberately — `TODO-C7`.

## 6. Application architecture

### Two threads, because the RP2040 has two cores

The system is levelled down to the weakest target: **exactly two threads**, because the RP2040 has
two cores. Do not introduce a third.

```
app_entry()                         PedalGuru.cpp
  STDIOInitAll()
  MountSdCard() + SelectActiveDrive()      <- before any thread exists
  TaskManager::Execute()
      ReadSettings / CreateDevices / ConnectToDevices
      Thread::NewThread(GetDevicesData)  --> thread 2: read every device's sensors in a loop
      GUIDrawer + CreatePages + HIDHandler + GUINavigator
      drawer.Execute()               --> thread 1 (the caller): HID + GUI render loop
  UnMountSdCard()
```

- **Thread 2 (sensors)** — `TaskManager::GetDevicesData()`, a `while (running_)` loop that calls
  `GetData()` on each connected device, with a `Time::Delay(1000)` between passes. On the RP2040 this
  is literally core 1 (`multicore_launch_core1`).
- **Thread 1 (UI)** — `GUIDrawer::Execute()`, a `while (!window.ShouldClose())` loop that invokes the
  current page's draw callback. `GUINavigator` uses `HIDHandler` to move between pages (and, in the
  future, between items inside a page: menus, buttons).

### DataManager

`DataManager` is a **singleton** (`GetInstance()`) that centralizes data between the two threads. It
currently holds one queue, `std::list<GPSFixData>`, with `Push` (sensor thread) and `Pop` (UI thread),
both guarded by the platform `Mutex` (§9).

**Important, and it corrects an assumption recorded in fs.ll's `AGENTS.md` §12:** `DataManager` does
**not** centralize SD card access — it only holds in-RAM sensor data. Every card access in pedal.guru
today happens on the **UI thread**: `Texture::DrawPng` / `DrawPngToArea` (`GUI/Render/Texture.cpp`),
and `OpenStreetMapAPI::DownloadTile` → `HttpClient_DownloadFile` (both reached from page callbacks).
The sensor thread never touches the file system: `GPS::LogGpsData` only pushes into `DataManager`.
Mount and unmount happen in `app_entry`, before the sensor thread starts and after it is asked to
stop — which is what FatFs requires (`f_mount` is never thread-safe). So the card is single-threaded
today **by accident of where the calls live**, not by design. fs.ll §12 leaves thread safety as an
open decision to be driven from here; that decision is still open — `TODO-E2`. Ask the dev before
acting on it.

### GUI layers

```
BasePage subclass (PageMap, PageSummary, ...)      the screen
    |  registers PreDraw / Draw / PostDraw callbacks through BasePage::Setup()
GUIDrawer            owns the loop, calls the current page's callbacks
GUINavigator         HID events -> page change -> new page's Setup()
    |
Window / Texture     C++ wrappers over gui.ll (LCDSetup.h, LCDRenderer.h, Canvas.h)
    |
gui.ll  ->  GC9A01 panel over SPI, or an SDL2 window on Simulator
```

`GUI/Render` is the only place in pedal.guru that talks to gui.ll's C API. `Window` wraps
`LCDInitialize` / `LCDClear` / `LCDRenderTexture` / `LCDRenderShouldClose` / `LCDRenderClose`;
`Texture` wraps `CanvasNewTexture` and the `Canvas*` drawing calls; `Color.hpp` converts a
`PedalGuru::Color` to gui.ll's RGB565 via the `COLOR_LL` macro; `Area.hpp` holds `Point` / `Size` /
`Rectangle`.

Pages are created in `TaskManager::CreatePages()`, conditionally on `SettingsData`. **The order in
that function is the page cycle order** — it matters. `PageMapSync` is appended last and is not
optional ("settings pages aren't optional").

Screen status today: `PageMap` and `PageMapSync` have real implementations. `PageAltimetry`,
`PageDistance`, `PageHillsGraph`, `PageRoute` and `PageSummary` are empty stubs (`TODO-D3`). More
screens are planned.

### HID

`HIDHandler` keeps one callback list per event type (`Model/HIDEventType.hpp`):
`ENTER`/`EXIT` × `DOWN`/`UP`/`PRESSED`/`PRESSED_2_SECONDS`/`PRESSED_5_SECONDS`. Consumers call
`RegisterEventHandler(type, lambda)` and get back an iterator to unregister with.

Two things to know: the functions that *fire* those events (`EnterDown()`, `ExitPressed()`, …) are
private and **nothing calls them yet** — they carry a `TODO` saying they should become GPIO interrupt
callbacks (the two reed switches, §3). And `GUINavigator`'s destructor has `UnregisterEvents()`
commented out because it threw an invalid-pointer exception; the `RegisterEventHandler` switch returns
`list::end()` rather than an iterator to the element just pushed, which is very likely the cause.
Recorded, not fixed — `TODO-D1` and `TODO-B1`.

## 7. Maps

`API/OpenStreetMapAPI` is the tile layer:

- **Tile math** — `LongitudeToTileX` / `LatitudeToTileY` and their inverses (standard Web Mercator /
  slippy-map formulas), `ListTilesForArea` for a bounding box, and `MapGridForCoordinate`, which picks
  the 2×2 tile neighbourhood around a coordinate and computes the pixel offset so the rider's position
  lands at the centre of the 240×240 screen.
- **Cache path** — `XyZoomToHashPath` reproduces OpenStreetMap `mod_tile`'s hashed storage layout
  (`zoom/h4/h3/h2/h1/h0`), which clusters a 16×16 square of tiles into one directory so no directory
  ends up with too many files. The reference is in the comment above the function.
- **Download** — `DownloadTile` checks the cache with `PathOrFileExists` first, then calls the platform
  `HttpClient_DownloadFile`, then sleeps 500 ms. That delay is deliberate: the
  [OSM tile usage policy](https://operations.osmfoundation.org/policies/tiles/) caps requests. Keep it.

`PageMapSync` walks a tile list and downloads it one tile per draw pass, showing progress.
`PageMap` reacts to a coordinate change by rebuilding the 2×2 grid and blitting the four tile
intersections into a 240×240 texture, then draws the position marker (green when the GPS has a fix,
orange otherwise).

## 8. GPS

`Sensor/GPS` parses NMEA `GGA` sentences (`$GNGGA`, `$GPGGA`) into `Model/GPSFixData`, converts
NMEA `DDMM.MMMM` to decimal degrees, applies the N/S and E/W sign, and pushes the result into
`DataManager`. `GPSFixData::set` splits the sentence with `TextHelper::Tokenize`.

**The UART is not wired up yet.** Every `uart_*` call in `GPS.cpp` is commented out and
`UartGetLine` returns a hard-coded sentence, so the GPS is a stub feeding fixed coordinates. The
`L96GPS` compile definition (set unconditionally at the end of `pedal.guru.cmake`) guards the
commented-out `$PMTK` configuration commands for the Quectel L96. Serial I/O is one of the layers
planned to become a submodule (§2, §13) — `TODO-D2` and `TODO-E1`.

## 9. Platform abstraction owned by pedal.guru

`src/Platform/<PLATFORM_NAME>/` holds the pieces of platform-specific code that have not been split
into a submodule yet. `pedal.guru.cmake` interpolates `${PLATFORM_NAME}` into both the source list and
the include path, so the application includes `"Thread.hpp"` / `"Time.hpp"` / `"HttpClient.h"` with no
`#ifdef` and gets the right implementation. Same mechanism as fs.ll and gui.ll use for their own
platform folders.

Each platform folder is expected to expose the same file names:

| file | contract |
|---|---|
| `Thread.{cpp,hpp}` | `PedalGuru::Thread::NewThread(void(*)())` and `PedalGuru::Mutex` (`Lock`/`Release`) |
| `Time.{cpp,hpp}` | `PedalGuru::Time::Delay(unsigned int milliseconds)` |
| `HttpClient.{c,h}` | `bool HttpClient_DownloadFile(const char *url, const char *filePath)` |
| `CMakeLists.txt` | ESP32 only: ESP-IDF component registration |

What each one does today:

| | Simulator | RP2040 | ESP32 |
|---|---|---|---|
| `NewThread` | `std::thread` + `detach` | `multicore_launch_core1` | `xTaskCreate` |
| `Mutex` | `pthread_mutex_*` | `pico/mutex.h` `mutex_enter_blocking` | `xSemaphoreTake/Give` |
| `Delay` | gui.ll's `Delay` (SDL event pump) | `sleep_ms` | — (file is empty) |
| `HttpClient` | POSIX sockets + OpenSSL, writes through fs.ll's `WriteFile` | — (file is empty) | — (file is empty) |

Only the **Simulator** folder is complete and compiling. The RP2040 and ESP32 folders are
work-in-progress and contain code that does not compile at all — see §14 for the exact state, §16 for
the specific defects, and group **A** of the TODO list in §17 for the backlog.

## 10. Build system

Two files, mirroring the pattern the submodules use.

**`pedal.guru.cmake`** declares `PLATFORM_NAME` (cached, `Simulator` by default, one of
`Simulator`/`RP2040`/`ESP32`), adds the upper-cased platform name as a compile definition (`SIMULATOR`
/ `RP2040` / `ESP32` — `PedalGuru.cpp` `#error`s if none is set), then **appends** the application's
sources and include dirs to `SOURCES` / `INCLUDE_DIRS`, and finally includes
`src/Dependency/fs.ll.cmake` and `src/Dependency/gui.ll.cmake`.

The `fs.ll.cmake` include must come **first**, before `gui.ll.cmake` — that is a requirement of
gui.ll's contract (the last thing `gui.ll.cmake` does is remove fs.ll's `HAL.c` from `SOURCES`, and a
later include would put it back and break the link with duplicate symbols). See gui.ll's `AGENTS.md`,
"Build Contract".

There is no `add_library` anywhere in the whole stack: each library appends to the two shared list
variables and the top-level target consumes them.

**`CMakeLists.txt`** branches on `PLATFORM_NAME`:

- `Simulator` — plain `project()`, include `pedal.guru.cmake`, `add_executable`, then
  `find_package(OpenSSL)` and link `${SDL2_LIBRARIES} m OpenSSL::SSL OpenSSL::Crypto`. OpenSSL is
  needed by the Simulator `HttpClient`; SDL2 is located by `gui.ll.cmake`.
- `RP2040` — defaults `PICO_SDK_PATH` to `~/pico-sdk`, `pico_sdk_init()`, include
  `pedal.guru.cmake`, `add_executable`, link
  `pico_stdlib pico_multicore hardware_spi hardware_gpio hardware_pwm hardware_adc hardware_rtc`,
  stdio over USB (UART off), `pico_add_extra_outputs`.
- `ESP32` — does **not** include `pedal.guru.cmake` itself. It points `EXTRA_COMPONENT_DIRS` at a
  platform folder and hands the build to ESP-IDF; that folder's `CMakeLists.txt` includes
  `pedal.guru.cmake` and calls `idf_component_register`. `EXTRA_COMPONENT_DIRS` currently points at
  `src/lib/Platform/ESP32`, **a path that does not exist in this repository** — see §16.

`-DDEBUGMSGS` (no value needed) defines both `DEBUGMSGS` and `_DEBUG`; the code guards `std::cout`
traces on `_DEBUG`.

## 11. Building and running

`CMAKE_EXPORT_COMPILE_COMMANDS` is on, so **build into `build/`** for working clangd intelligence.
`.vscode/tasks.json` has the ready-made tasks (`Build: RP2040 Full`, `Build: ESP32 Full`,
`Build: Incremental`, `Build: Simulator Incremental`); they hardcode `~/pedal.guru` as the working
directory. The equivalent commands:

```bash
# Simulator (desktop)
Toolchain/Simulator/Setup.sh                      # once: SDL2, gdb, dosfstools, mtools, sdcard.img
cmake -B build -DPLATFORM_NAME=Simulator && cmake --build build
./build/pedal.guru                                # run from the repo root, see below

# RP2040 (needs pico-sdk at ~/pico-sdk or PICO_SDK_PATH)
cmake -B build -DPLATFORM_NAME=RP2040 && cmake --build build

# ESP32 (needs ESP-IDF exported in the shell)
source ~/esp-idf/export.sh && idf.py -DPLATFORM_NAME=ESP32 build
```

The Simulator reads and writes `sample/sdcard.img` through a **relative** path (fs.ll's
`SD_DISK_IMAGE`), so the binary must be started **from the repository root**, not from `build/`.
Running the program mutates the image, which then shows up as a modified binary file in
`git status`; restore it with `git checkout -- sample/sdcard.img`.

`Toolchain/Simulator/Setup.sh` rebuilds `sample/sdcard.img` from `sample/sdcard/` on every run (not
idempotent, on purpose, so sample changes are always reflected). Note that `sample/sdcard/` holds a
single `01.png`, while the map code looks tiles up by their `mod_tile` hash path (§7) — the sample
image does not contain a pre-populated tile cache.

Also note that configuring **patches fs.ll's FatFs submodule in place** (`ffconf.h`, via
`fatfs.ffconf_patch.cmake`). That dirt is expected and must never be committed. See fs.ll's
`AGENTS.md` §8.

Flashing the RP2040 from WSL is handled by `Toolchain/RP2040/Bind.sh` (one-time USB share via
`usbipd`, needs admin once) and `Toolchain/RP2040/Flash.sh` (`picotool` load of
`build/pedal.guru.uf2`). `Toolchain/wsl.sh` repairs WSL Windows interop under systemd.
`launch.json` debugs the Simulator through `Toolchain/Simulator/gdb-wrapper.sh`, which unsets
`DEBUGINFOD_URLS` — see gui.ll's `AGENTS.md`, "Known Issues", for why that wrapper exists.

## 12. Code conventions

pedal.guru is **C++** (the submodules are C). Conventions inferred from the existing code — follow
them, and do not "modernize" them silently.

- **File and directory names are PascalCase**, acronyms in UPPERCASE (`GUIDrawer.cpp`,
  `HIDHandler.hpp`, `OpenStreetMapAPI.cpp`, `GPSFixData.hpp`). Headers are `.hpp`, sources `.cpp`,
  except the C platform pieces (`HttpClient.c/.h`). Known deviation:
  `Device/DIY/wired_reel_speedometer.cpp` (§5).
- **Everything lives in `namespace PedalGuru`.**
- **Types** (classes, structs, enums) → PascalCase. **Public methods** → PascalCase. **Variables,
  parameters and public data members** → camelCase. **Private members** → trailing underscore
  (`settings_`, `connected_`, `pageIndex_`). **Macros and enum values** → UPPER_SNAKE_CASE
  (`COLOR_LL`, `ENTER_PRESSED_2_SECONDS`, `TILE_WIDTH`).
- **Abstract bases declare the contract as pure virtuals** and carry a
  `virtual ~X() = default; // make it polymorphic` line. New devices and sensors follow `Device.hpp` /
  `Sensor.hpp`.
- **Ownership is `std::unique_ptr` in `std::list`** for devices and pages; callbacks are
  `std::shared_ptr<Callback>` wrapping a `std::function<void()>`.
- **C headers from the submodules are included inside `extern "C" { }`** (`HAL.h`, `FileSystem.h`,
  `Canvas.h`, `LCDSetup.h`, `LCDRenderer.h`, `HttpClient.h`, `Types.h`, `pthread.h`, pico-sdk and
  FreeRTOS headers).
- **Headers use `#pragma once`**, not include guards, in pedal.guru's own C++ code. Deviations exist
  (`GUI/Page/PageMap.hpp` has none at all).
- **Every file opens with the AGPL-3.0 header block.** New files get it too.
- **Comments explain *why***, and are used where a decision looks arbitrary (the page cycle order, the
  OSM rate-limit delay, the 8px padding to centre a 256px tile on a 240px screen). Both submodules
  carry an explicit "keep comments minimal" policy — prefer clear names and structure over prose, and
  do not restate what the code already says.
- **Indentation is 4 spaces, ruler at 120 columns** (`.vscode/settings.json`).
- `TODO` comments are used to mark known-incomplete work. They are load-bearing; do not delete one
  without doing the work or asking.

## 13. Migration status: what still has to move out of pedal.guru

pedal.guru is meant to be pure application logic. It is not there yet. Known material that is either
platform-specific or infrastructure, and is a candidate to become (or move into) a submodule:

- **`src/Platform/*/HttpClient.*`** → the future `net.ll`. The Simulator implementation is a full
  socket + OpenSSL HTTP client; that does not belong in an application layer.
- **`src/Platform/*/Thread.*`** and **`src/Platform/*/Time.*`** → the future `thread.ll` (or whatever
  the dev names it): threads, mutexes, timers, delays.
- **GPS UART access** (`src/Sensor/GPS.cpp`, currently commented out) → the future `serial.ll`.

Tracked as `TODO-E1`. Absence of something from this list does not mean it stays here. Ask the dev.

Not implemented at all yet, from §1's product goal: the whole **"guru" coaching side** — hydration
and nutrition reminders, cadence guidance by stretch or elapsed time. There is no module for it
(`TODO-D4`).

## 14. Current status (verified this session)

- **Simulator** — configures, builds and links clean (`cmake -DPLATFORM_NAME=Simulator`, then
  `cmake --build`). This is the only platform that builds today. The repository already contains a
  `build/` configured for Simulator and a `build/pedal.guru` binary.
- **RP2040** — configures (pico-sdk found, picotool found) but **fails to compile**. The first errors
  come from `src/Platform/RP2040/Time.hpp`, which is not valid C++ (see §16).
- **ESP32** — **fails at configure**: `EXTRA_COMPONENT_DIRS` points at
  `${CMAKE_SOURCE_DIR}/src/lib/Platform/ESP32`, which does not exist (see §16).
- Git branch: `feature/RP2040-migration`.
- The working tree has **uncommitted changes to `.gitignore` and `.gitmodules`**: they comment out
  `ignore = all` on both submodules and comment both paths out of `.gitignore`. That is the move back
  to real submodules described in §2 — expected, not a leftover to revert. Do not commit or revert them
  on your own initiative.

## 15. Not verified

- All three build outcomes in §14 were **executed** in isolated build directories this session
  (Simulator: configure + build + link; RP2040: configure + build; ESP32: `idf.py reconfigure`). The
  Simulator binary was **not** run, so nothing here confirms runtime behaviour, and no claim in this
  file about what appears on screen is validated.
- Nothing in this file is validated **on hardware**. Statements about the RP2040 and ESP32 platform
  code come from reading it.
- Everything in §3 about the planned expansion board (microSD slot, two reed switches, GPS, WiFi+BT
  module for the RP2040, single shared design, waterproof case, magnetic ring), and everything in §1
  about the coaching features, is **stated by the dev** and has no counterpart in the code yet. The
  parts that *are* in code are the SD card and card-detect wiring, which live in gui.ll's
  `HALConfig.h`.
- `DIAGRAM.md` is **stale**: it links to a `PedalGuru` repository under old paths
  (`src/task_manager.cpp`, `src/gui/pages/`, `src/PedalGuru.hpp`) that no longer exist, and describes
  a `TaskManager` API (`Run`, `DisplayPage`, `StartDevice`) that does not match `TaskManager.hpp`.
  Treat it as a historical sketch — `TODO-C5`.
- `.vscode/c_cpp_properties.json` is also stale (references `src/Target/**` and
  `src/Dependency/pico-sdk`, neither of which exists, and defines `TARGET_RP2040`, which no longer
  matches the `RP2040` compile definition the build sets). clangd, not the MS C/C++ extension, is what
  the project actually uses — `TODO-C6`.


## 16. Traps and sharp edges

Known, unfixed, and easy to trip over. They are recorded so that changing them is a **deliberate
decision by the dev**, not a drive-by fix. **Every item here has a `TODO-*` entry in §17** — that is the
list to work from. Do not fix any of these as a side effect of unrelated work.

**Build-breaking:**

- `TODO-A1` — `CMakeLists.txt` ESP32 branch sets `EXTRA_COMPONENT_DIRS` to `src/lib/Platform/ESP32`.
  pedal.guru has no `src/lib`; the folder is `src/Platform/ESP32` (which does contain the component
  `CMakeLists.txt`). The path looks copied from gui.ll's layout. This is the single reason the ESP32
  configure fails.
- `TODO-A2` — `src/Platform/RP2040/Time.hpp` declares `void Delay(milliseconds: unsigned int);` —
  Pascal/Rust-style parameter syntax, not C++. It also puts `extern "C" { #include ... }` *inside*
  `namespace PedalGuru`, declares `Delay` as an instance method while callers use `Time::Delay(...)`
  statically, and has no `#pragma once` and no licence header. `Time.cpp` repeats the same invalid
  signature and also `#include`s its header inside the namespace.
- `TODO-A3` — `src/Platform/RP2040/Thread.cpp` initializes a pico-sdk `mutex_t` with
  `PTHREAD_MUTEX_INITIALIZER` (a POSIX macro from a different type). It then lazily calls `mutex_init`
  inside `Lock()`, which is itself a race between two cores.
- `TODO-A4` — `src/Platform/ESP32/Thread.cpp` has `Mutex::lock_ = xSemaphoreCreateMutex();` with no type
  on the definition, and `#include`s its header inside the namespace.
- `TODO-A5` — `src/Platform/ESP32/Time.hpp` is an empty namespace, no `Time` class at all. `Time.cpp`,
  `ESP32/HttpClient.{c,h}` and `RP2040/HttpClient.{c,h}` are **0-byte files** that are nonetheless
  listed in `pedal.guru.cmake`'s `SOURCES`.

**Logic / correctness:**

- `TODO-B1` — `HIDHandler::RegisterEventHandler` returns `list::end()` for every case instead of an
  iterator to the element it just pushed. `UnregisterEventHandler` then erases `end()`, which is
  undefined behaviour — almost certainly why `GUINavigator`'s `UnregisterEvents()` is commented out with
  an "invalid pointer exception" note.
- `TODO-B2` — `GUINavigator::GoToNextPage` advances the iterator and dereferences it without re-checking
  for `end()`, so the last page steps one past the end. `GoToPreviousPage` sets
  `pageIndex_ = pages_.end()` and dereferences it directly. Both dereference `end()`.
- `TODO-B3` — `TaskManager::GetDevicesData` has
  `device = (device == devices_.end()) ? devices_.begin() : device++;` — the ternary yields the
  *pre*-increment value, so the iterator never actually advances past the first device, and the `end()`
  check happens before the increment rather than after.
- `TODO-B4` — `GPSFixData::set` stores `data[11]`, `data[14]` and `data[15]` (a local `char[16][16]`)
  into the `char *` members `geoidalSeparation`, `differentialGPSStationId` and `checksum`. Those
  pointers dangle as soon as `set` returns.
- `TODO-B5` — `PageMapSync::DrawPageContents` sizes a VLA as `char progress[(totalTiles_ * 2) + 3]` from
  a runtime value and writes into it with `sprintf`. Beyond the VLA itself, the size formula does not
  follow from the format it writes.
- `TODO-B6` — `PageMap`'s `previousLatitude` / `previousLongitude` are never initialized.
- `TODO-B7` — `Color.hpp`'s `COLOR_LL` casts to **signed `short`**. gui.ll's Decision 14 is explicit
  that RGB565 values must be `UINT16`: as a signed `short`, any colour above `0x7FFF` (including
  `COLOR_TRANSPARENT` = `0xF81F`) is negative, and a comparison against a `UINT16` is always false after
  integer promotion — which silently breaks gui.ll's colour-key transparency.
- `TODO-B8` — `Texture::Release()` calls `free()` on the texture data but `Texture` has no destructor,
  so leaking is one forgotten call away. `PageMap` / `PageMapSync` release in `PostDrawPageContents`.
- `TODO-B9` — `GPS::GetData` loops up to 50 times over `UartGetLine`, which currently returns a
  constant, so with a non-matching constant it would spin 50 times per pass.
- `TODO-B10` — `OpenStreetMapAPI::ListTilesForArea` declares `int x = 0, y = 0;` and then shadows both
  in the loop headers. Harmless, but the outer pair is dead.

Not a defect, recorded so it is not "fixed" by accident: `COLOR_MAGENTA` and `COLOR_TRANSPARENT` are
deliberately the same value (`255, 0, 255`), matching gui.ll's colour key. The consequence — magenta
cannot be used as a real drawable colour — is the accepted cost of colour-key transparency in RGB565.

**Stale / cosmetic:**

- `TODO-C2` — `pedal.guru.cmake` lists `src/GUI/Interface` in `INCLUDE_DIRS`; that directory does not
  exist.
- `TODO-C3` — `src/Model/SensorData.hpp` is an empty struct, referenced by `DIAGRAM.md` but by no code.
- `TODO-C4` — `TaskManager::ReadSettings` hardcodes the settings with a `TODO`; nothing reads or writes
  settings from the card yet.

## 17. TODO list

The working backlog. When the dev asks what there is to do, **this is the list to return**.

Nothing here is scheduled, and nothing here is authorized: each item still needs the dev's go-ahead
before being worked on (ground rule 2). Items are grouped by kind and carry a stable ID so they can be
referenced in conversation. Keep this list up to date: when an item is done, remove it and update the
section it came from; when a new one is found, add it here rather than fixing it silently.

### A. Build-breaking — only the Simulator builds today

The RP2040 and ESP32 platform folders under `src/Platform` do not compile. Verified in §14. These are
the items that stand between the project and a three-platform build.

| ID | Item |
|---|---|
| `TODO-A1` | ESP32: `EXTRA_COMPONENT_DIRS` points at the non-existent `src/lib/Platform/ESP32` instead of `src/Platform/ESP32`. Blocks the ESP32 configure entirely. |
| `TODO-A2` | RP2040: `Platform/RP2040/Time.{hpp,cpp}` is not valid C++ (`milliseconds: unsigned int`, `#include` inside the namespace, instance method used statically, no `#pragma once`, no licence header). First error of the RP2040 build. |
| `TODO-A3` | RP2040: `Platform/RP2040/Thread.cpp` initializes a pico-sdk `mutex_t` with `PTHREAD_MUTEX_INITIALIZER`, and lazily `mutex_init`s inside `Lock()` — a race between the two cores. |
| `TODO-A4` | ESP32: `Platform/ESP32/Thread.cpp` — `lock_` definition has no type, `#include` inside the namespace. |
| `TODO-A5` | ESP32/RP2040: `ESP32/Time.hpp` is an empty namespace; `ESP32/Time.cpp`, `ESP32/HttpClient.{c,h}` and `RP2040/HttpClient.{c,h}` are 0-byte files that `pedal.guru.cmake` nevertheless lists in `SOURCES`. Decide whether they get implemented here or go straight into the future `net.ll` / `thread.ll` (`TODO-E1`). |

### B. Bugs and correctness

| ID | Item |
|---|---|
| `TODO-B1` | `HIDHandler::RegisterEventHandler` returns `list::end()` instead of an iterator to the pushed element; `UnregisterEventHandler` then erases `end()`. Fixing this is what unblocks `GUINavigator::UnregisterEvents()`, currently commented out. |
| `TODO-B2` | `GUINavigator::GoToNextPage` / `GoToPreviousPage` both dereference `end()` at the ends of the page cycle. |
| `TODO-B3` | `TaskManager::GetDevicesData`'s iterator never advances past the first device (`device++` inside a ternary). |
| `TODO-B4` | `GPSFixData::set` leaves three `char *` members pointing at a local buffer that dies with the call. |
| `TODO-B5` | `PageMapSync::DrawPageContents` uses a runtime-sized VLA plus `sprintf`, with a size formula unrelated to what it writes. |
| `TODO-B6` | `PageMap::previousLatitude` / `previousLongitude` are never initialized. |
| `TODO-B7` | `COLOR_LL` casts to signed `short`, which breaks gui.ll's colour-key transparency (gui.ll Decision 14 requires `UINT16`). |
| `TODO-B8` | `Texture` has no destructor; `Release()` must be called by hand. Candidate for RAII. |
| `TODO-B9` | `GPS::GetData` can spin 50 times per pass against a stubbed `UartGetLine`. |
| `TODO-B10` | `OpenStreetMapAPI::ListTilesForArea` has a dead, shadowed `int x = 0, y = 0;`. |

### C. Obsolete / cleanup

| ID | Item |
|---|---|
| `TODO-C1` | Remove the leftover `src/Dependency/fs.ll.cmake` and `src/Dependency/gui.ll.cmake` copies, residue of the contract-based approach that submodules replaced (§2). The build currently gets its source and include lists from them, so this is a migration, not a deletion. |
| `TODO-C2` | `pedal.guru.cmake` lists the non-existent `src/GUI/Interface` in `INCLUDE_DIRS`. |
| `TODO-C3` | `src/Model/SensorData.hpp` is an empty struct used by nothing. Fill it in or drop it. |
| `TODO-C4` | `TaskManager::ReadSettings` hardcodes the settings; no persistence to or from the card yet. |
| `TODO-C5` | `DIAGRAM.md` is stale: links to a `PedalGuru` repo with paths that no longer exist, and a `TaskManager` API that no longer matches. Refresh or drop. |
| `TODO-C6` | `.vscode/c_cpp_properties.json` is stale: references `src/Target/**` and `src/Dependency/pico-sdk` (neither exists) and defines `TARGET_RP2040` instead of the `RP2040` the build actually sets. The project uses clangd, not the MS C/C++ extension — so this may be droppable outright. |
| `TODO-C7` | `Device/DIY/wired_reel_speedometer.cpp`: out of the build, `snake_case` name, class declared inside the `.cpp` with no header, does not implement all of `Device`'s pure virtuals. Keep as a placeholder, complete and rename to `WiredReelSpeedometer.{cpp,hpp}`, or remove until the device is really supported. |

### D. Pending implementation

| ID | Item |
|---|---|
| `TODO-D1` | Wire the HID events to real hardware: the `HIDHandler` firing functions (`EnterDown`, `ExitPressed`, …) are private and nothing calls them. They are meant to become GPIO interrupt callbacks for the two reed switches (§3, §6). Until then there is no user input at all. |
| `TODO-D2` | Wire up the GPS UART. Every `uart_*` call in `GPS.cpp` is commented out and `UartGetLine` returns a hard-coded NMEA sentence (§8). |
| `TODO-D3` | Implement the stub pages: `PageAltimetry`, `PageDistance`, `PageHillsGraph`, `PageRoute`, `PageSummary`. Only `PageMap` and `PageMapSync` do anything today. |
| `TODO-D4` | Implement the whole **"guru"** side — the coaching features from §1: hydration and nutrition reminders, cadence guidance by road stretch or elapsed training time. No module exists for it yet. |
| `TODO-D5` | Support multi-sensor devices, e.g. the same device measuring cadence on the pedal or speed on the wheel depending on configuration (§5). The `Device`/`Sensor` split already allows it; nothing exercises it. |
| `TODO-D6` | Navigation *within* a page (menus, buttons), not just between pages. `GUINavigator` only cycles pages today (§6). |

### E. Architecture / migration

| ID | Item |
|---|---|
| `TODO-E1` | Extract the remaining infrastructure from `src/Platform` into the dot-ll-collection: `HttpClient` → `net.ll`; `Thread` + `Time` → `thread.ll` (name not final); GPS UART → `serial.ll` (name not final). See §2 and §13. Overlaps `TODO-A5`. |
| `TODO-E2` | Decide SD card thread safety. fs.ll's `AGENTS.md` §12 leaves it open and says the decision is to be driven from here. Current state, verified: all card access happens on the UI thread and `DataManager` does **not** mediate it, so the card is single-threaded by accident of call placement rather than by design (§6). This needs a decision before anything on the sensor thread starts writing to the card — e.g. ride logging, which `TODO-D4` and `TODO-C4` both imply. |
| `TODO-E3` | Design the open-hardware expansion board (microSD slot, two reed switches, GPS, optional WiFi+BT for the RP2040) as a single shared board, using only the header pins common to both MCUs (§3). Nothing of this exists in the repository yet. |
