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

### 5. Comments only when essential

**The code has to speak for itself.** Prefer clear names and structure over prose. This is a standing
rule from the dev, in force across pedal.guru, fs.ll and gui.ll — and it is routinely violated by agents,
so read it as a hard limit, not a preference.

Write a comment only when it carries information the code cannot:

- a non-obvious **why** (a rationale, a subtle invariant, a bug a naive change would reintroduce);
- a necessary technical note (units, fixed-point format, overflow reasoning, a hardware or spec quirk);
- a short header stating a module's purpose.

Do **not** write:

- anything that restates what the code already says. If the line sits in the `else()` of a platform
  branch, "no supported platform matched" adds nothing — that is what `else()` means;
- step-by-step narration above or inside a function, or per-line annotation;
- requirement or decision tags (`// Req 3.1`, `// Decision 14`). Traceability belongs in this file, not
  scattered through the source;
- multi-paragraph explanations. Keep it to a line or two. If a block needs a paragraph to be understood,
  extract a well-named helper instead, or put the explanation here in `AGENTS.md` and leave at most a
  pointer in the code.

Rationale that is long but genuinely valuable belongs in this document, where it is read once, not in a
header that every reader has to scroll past.

Applies to new code and to edits of existing code. Pre-existing third-party code under `src/Dependency/`
is left as-is unless touched.

### 6. All hardware access goes through hal.ll

**Nothing in pedal.guru touches hardware directly.** No `gpio_*`, `spi_*`, `uart_*`, `sleep_ms`,
`vTaskDelay` or any other SDK call — everything goes through **hal.ll**, which owns GPIO, SPI, PWM, UART,
timing, the RTC, threads and mutexes, and the board pinout in `HALConfig.h`. If something is missing
there, it gets added there rather than worked around here.

This rule holds across the whole dot-ll-collection, so the same paragraph appears in every `AGENTS.md`.

### 7. pedal.guru is the only place C++ belongs

The whole dot-ll-collection is **100% C** — fs.ll, gui.ll, hal.ll and net.ll consume each other, so a C++
construct in a shared header would break them. C++ stops here, in the application. When pedal.guru wraps
a library API in a class, the class lives here and the library stays C.

### 8. Do not write tests

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
| **hal.ll** | `src/Dependency/hal.ll` | GPIO, SPI, PWM, UART, timing, RTC, threads, and the board pinout | `src/Dependency/hal.ll/AGENTS.md` |
| **fs.ll** | `src/Dependency/fs.ll` | file system on the SD card wired to the hardware (wraps FatFs) | `src/Dependency/fs.ll/AGENTS.md` |
| **gui.ll** | `src/Dependency/gui.ll` | drawing on the LCD panel or on the simulator window | `src/Dependency/gui.ll/AGENTS.md` |
| **net.ll** | `src/Dependency/net.ll` | WiFi scan and HTTP/HTTPS download to storage. Consumed by pedal.guru since wave 5 | `src/Dependency/net.ll/AGENTS.md` |

**Read those `AGENTS.md` files.** They carry the conventions, the build-contract mechanics, the hardware
pinout decisions and the known traps that pedal.guru inherits. This file does not repeat them.

hal.ll sits below the other three: fs.ll, gui.ll and net.ll all consume it, and so does pedal.guru
directly (for sensors). It is the one place where SDK calls are allowed — see ground rule 6.

### Why git submodules here, and not the `.cmake` contract

**Decided by the dev.** pedal.guru consumes fs.ll and gui.ll as **git submodules**, on purpose, so that
**GitHub reports the dependency between the repositories** in its reference/usage statistics. A
directory populated at configure time by a `git clone` inside a `.cmake` file is invisible to GitHub;
a submodule is not. That visibility is the deciding factor here.

This is a deliberate divergence from gui.ll's Decision 16, which drops sibling libraries from
`.gitmodules` so that several consumers can share one checkout. Both positions are valid — they
optimize for different things (shared checkout vs. visible dependency graph), and pedal.guru, being the
top-level application rather than a library, chooses visibility.

Because the libraries are checkouts in the tree, their contracts are included **straight from the
submodules** — `src/Dependency/gui.ll/gui.ll.cmake` — instead of from copies kept here. The copies that
the earlier contract-based approach required are gone.

That makes pinning `HAL_LL_PATH`, `FS_LL_PATH` and `GUI_LL_PATH` mandatory rather than optional. Each
contract defaults its own path to a folder *next to itself*, so `gui.ll.cmake` read from inside the
submodule would default `GUI_LL_PATH` to `src/Dependency/gui.ll/gui.ll`, find no sentinel file there, and
`git clone` a second copy at configure time. Setting all three variables before the include is what keeps
the build on the submodules — `HAL_LL_PATH` matters twice over, because both gui.ll's and fs.ll's copies of
`hal.ll.cmake` would otherwise each resolve to a folder inside their own checkout.

Planned, still living inside pedal.guru and expected to move out (stated by the dev, names not
final): `net.ll` for networking, and **hal.ll for threads, timers and UART, which is already done** —
there is no separate `thread.ll`. A `serial.ll` may still appear, but for protocol rather than
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
the two boards may be used** — with one deliberate exception, the radio, explained below.

**Three sources, and they answer different questions.** Confusing them is how a wrong pinout gets
written down:

- **This repository's `README.md`, the "🔌 Wiring" table** — what is *actually wired* today, in the form
  of the connector/shield that swaps between the two devices. gui.ll's README carries the same table;
  prefer this one, since the peripherals below belong to the product, not to the drawing library.
- **gui.ll's `AGENTS.md`, Design Decision 7** — which pins are *free and shared* between the two
  boards, derived by overlaying the two pinouts. **Two traps in that document:** the summary line under
  its tables says "3 fully usable", which contradicts the tables themselves — the tables are right, and
  there are 5. And its "H1 Pin 17 / 19" entries are *header positions*, not GPIO numbers: those
  positions carry `SWCLK`/`SWDIO`, which on the RP2040 are dedicated pads outside the 30 GPIOs. GPIO 17
  and GPIO 19 are ordinary free GPIOs with nothing to do with that caveat.
- **`Documentation/Image/RP2040_LCD_1_28.png`** — the board's own onboard functions, which rule pins out
  entirely, plus the header positions, which tell you whether a block of pins is physically contiguous.

Do not invent pin assignments. Those three plus each platform's `HALConfig.h` in hal.ll are the source
of truth, and some values there are hardware-validated.

#### The RP2040 pin map

Consolidated here because it spans peripherals that no single submodule owns: gui.ll knows the LCD and
the card but not the GPS, the switches or the radio.

**Taken by the Waveshare board itself** — internal traces, not negotiable:

| GPIO | function |
|---|---|
| 6, 7 | IMU I2C (SDA, SCL) |
| 8, 9, 10, 11, 12 | LCD DC, CS, CLK, DIN/MOSI, RST |
| 23, 24 | QMI8658C IMU interrupts 1 and 2 |
| 25 | LCD backlight |
| 29 | battery voltage ADC |

**Assigned by pedal.guru on the shared expansion board** — every one of these lands on a header
position that exists on the ESP32-S3 board too:

| GPIO | header | function | ESP32-S3 GPIO |
|---|---|---|---|
| 0, 1, 2, 3 | — | SD card MISO, CS, SCK, MOSI | 46, 45, 42, 41 |
| 5 | H1-12 | SD card detect | 39 |
| 16 | H2-2 | GPS UART TX | 13 |
| 13 | H1-11 | GPS UART RX | 18 |
| 14 | H1-13 | navigation switch A | 17 |
| 15 | H1-15 | navigation switch B | 16 |

`GP16`/`GP13` is the **only** usable TX/RX pair among the shared pins: of the five, only those two carry
a UART data function. `GP14` and `GP15` are UART0 CTS/RTS and `GP27` is UART1 RTS, none of which carries
data; and UART1's data pins, `GP20`/`GP21`, belong to the radio. Verified against the mux table in the
pico-sdk's `io_bank0.h`. On the ESP32-S3 the GPIO matrix routes a UART to any pin, so only the RP2040
constrains this.

**Assigned to the radio — RP2040 only**, see below:

| GPIO | header | signal |
|---|---|---|
| 19 | H2-8 | `WL_REG_ON` |
| 20 | H2-10 | `WL_DATA` (bidirectional, also host wake) |
| 21 | H2-12 | `WL_CLOCK` |
| 22 | H2-14 | `WL_CS` |

**Still free: `GP27`** (H2-7, ESP32-S3 GP2) is the last shared pin available, plus the two SWD-caveat
positions (H1-17, H1-19) as a last resort. On the RP2040 alone, 4, 17, 18, 26 and 28 remain.

The **SD card pins are hardware-validated** — the card was read on the RP2040-LCD-1.28 with this exact
wiring. The commented second number beside each one in `HALConfig.h` is the pin used on an older plain
Pico driving an external round LCD, also tested there; it is a record of the two boards, not a menu.
None of the GPS, switch or radio assignments is validated yet — `TODO-D1`, `TODO-D2`, `TODO-E3`.

#### The radio is allowed non-shared pins

**Decided by the dev.** The WiFi+BT module is RP2040-only by nature — the ESP32-S3 has its radio
built in — so it does not have to fit the shared budget. It uses pins that are free only on the
RP2040, and a board destined for the ESP32-S3 leaves that footprint unpopulated. One board design
still serves both.

That frees the 5 shared pins for the GPS (TX + RX) and the two reed/hall switches, which is 4 of the 5.

The radio needs **four** pins, because three of the CYW43's functions share one wire:

| signal | function | speed |
|---|---|---|
| `WL_REG_ON` | plain GPIO output; enables the module's regulator | static, set once at bring-up |
| `WL_DATA` | the single **bidirectional** data line, PIO-driven; also the module's host-wake interrupt | fast |
| `WL_CLOCK` | gSPI clock, a PIO side-set pin | fast |
| `WL_CS` | plain GPIO output, chip select | slow |

**No pin has a special-function requirement.** The bus is not the hardware SPI peripheral — it is a PIO
program (`cyw43_bus_pio_spi.pio`), because the CYW43's gSPI is half-duplex and one wire carries both
the outgoing command and the reply, which the SPI peripheral cannot do. PIO can drive any GPIO, and the
driver configures each of the three PIO pins individually (`sm_config_set_out_pins(..., 1)`,
`sm_config_set_in_pins`, `sm_config_set_sideset_pins`), so **there is no adjacency requirement**. The
`static_assert` in that file constrains only the RP2350B, which has two GPIO banks.

Cost to be aware of: one of the RP2040's 8 PIO state machines plus 2 DMA channels. Nothing else in the
project uses PIO, so there is no contention today.

**Decided by the dev:** GPIO 19, 20, 21, 22 — header H2 positions 8, 10, 12 and 14, four contiguous
positions in one column, which suits a module footprint. `WL_REG_ON` on 19, `WL_DATA` on 20, `WL_CLOCK`
on 21, `WL_CS` on 22.

Only DATA and CLOCK have critical timing, hence keeping them neighbours; it also holds the GPIO range the
PIO reserves to 2 pins instead of 4. REG_ON is static and CS is slow, so they take the outer positions.
The module also needs 3V3 and GND, but **no** extra RP2040 pin for power-save or VBUS sense: those live
on the module's own three GPIOs, which is where the Pico W's onboard LED sits.

Signal names here come from the SDK macros and the Pico W schematic nets (`WL_ON`, `WL_D`, `WL_CLK`,
`WL_CS`); confirm the labels against the RM2 datasheet before wiring.

#### Where the radio pins are declared, and why not in `HALConfig.h`

The radio's four pins live in **`src/Platform/RP2040/Boards/WaveshareRP2040LCD1In28Extended.h`**, a
pico-sdk *board header*, selected with `PICO_BOARD` and found through `PICO_BOARD_HEADER_DIRS` (§10).
Every other pin on the device stays in hal.ll's `HALConfig.h`.

The dividing line is **who reads the number**. The radio's pins are read by third-party code — the
cyw43 driver inside the pico-sdk — and that code only looks at its own `CYW43_DEFAULT_PIN_WL_*` macros.
It is the one case in this project where a pin number is consumed from outside, so it has to be declared
where the outsider looks. Everything else is read by our own code, which reads `HALConfig.h`.

That header does two jobs, not one: it declares `PICO_CYW43_SUPPORTED` so the radio code can compile at
all, **and** it tells the driver where the module is wired. It also `#include`s
`boards/waveshare_rp2040_lcd_1.28.h` last, so the base board's own definitions (flash size, default
UART, platform) are inherited while every `#ifndef`-guarded value above wins over it. The pico-sdk
follows that include when scraping cmake directives, so composing headers this way is supported rather
than a trick. **No pico-sdk file is modified.**

Verified through the preprocessor after the build: `REG_ON 19u`, `DATA 20u`, `HOST_WAKE 20u`,
`CLOCK 21u`, `CS 22u`, with `PICO_FLASH_SIZE_BYTES` and `PICO_DEFAULT_UART` arriving from the base
board header.

#### Distributing the firmware is unaffected by PIO

Worth stating because it is a natural worry: the PIO program needs **nothing** shipped alongside the
`.uf2`. `pioasm` assembles `cyw43_bus_pio_spi.pio` at build time into a C header holding the program as
a five-entry `uint16_t` array, that array is linked into the firmware like any other constant, and
`pio_add_program()` copies it into the PIO block's instruction memory at every boot. PIO is not separate
persistent storage. One file to flash, exactly as before.

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
    hal.ll/                     submodule, carries its own hal.ll.cmake contract
    fs.ll/                      submodule, carries its own fs.ll.cmake contract
    gui.ll/                     submodule, carries its own gui.ll.cmake contract
    net.ll/                     submodule, carries its own net.ll.cmake contract; not consumed yet
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
both guarded by the platform `Mutex` (§9). `GetInstance()` takes the same lock, because both threads
reach it (`GPS::LogGpsData` and `PageMap`) and either could be the first.

### How Mutex is initialized, and why

A mutex has to be set up before first use, and the three platforms differ on how. `pthread` offers a
compile-time constant; the pico-sdk needs a `mutex_init()` call; FreeRTOS needs
`xSemaphoreCreateMutex()`, which also allocates. Three options were considered: lazy initialization
inside `Lock()`, one explicit call before the threads start, or **the constructor**.

**The constructor won**, and the reasoning is worth keeping, because the earlier code tried the other
two and both were wrong here:

- The queue is **unbounded**, and deliberately so: the GPS emits roughly one fix per second and the UI
thread consumes at the same rate, so the dev expects it to hold no more than a few items. `GPSFixData` is
152 bytes on ARM. If growth is ever observed in practice, revisit then — it is not a problem being
ignored, it is a bound that the data rate already provides.

`Mutex::lock_` is **per instance**, not `static`. It used to be `static`, which quietly made every
  `Mutex` in the program share a single lock — harmless while `DataManager` owned the only one,
  misleading the moment a second appeared. Once the lock belongs to the object, the object's constructor
  is the natural place to prepare it, and there is nothing left for a caller to remember.
- **Lazy initialization was rejected.** The old RP2040 `Lock()` did
  `if (!mutex_is_initialized(&lock_)) mutex_init(&lock_);`, and that check-then-init is itself an
  unprotected race between the two cores: the object meant to prevent a race was created by one.
- **An explicit `Initialize()` call was tried and then dropped.** It worked, and it mirrored the way the
  SD card is mounted in `app_entry`, but it left `Lock()` on a fresh `Mutex` as undefined behaviour and
  put the burden on every future owner.

All three platforms therefore have the same shape: a constructor that prepares `lock_`, plus `Lock()`
and `Release()`. The Simulator uses `pthread_mutex_init()` rather than the constant it could have used,
so that no platform is the odd one out — that asymmetry is exactly what produced the original bug, where
`PTHREAD_MUTEX_INITIALIZER` had been copied onto a pico-sdk type.

Consequence to be aware of on the hardware platforms: `DataManager::mutex_` is a static object, so its
constructor runs during static initialization, before `main`/`app_main`. On ESP32 that means
`xSemaphoreCreateMutex()` allocates before `app_main`. Both compile; neither has been **run** on
hardware (§15). There is no destructor — these mutexes live for the whole program.

**Important, and it corrects an assumption recorded in fs.ll's `AGENTS.md` §12:** `DataManager` does
**not** centralize SD card access — it only holds in-RAM sensor data. Every card access in pedal.guru
today happens on the **UI thread**: `Texture::DrawPng` / `DrawPngToArea` (`GUI/Render/Texture.cpp`),
and `OpenStreetMapAPI::DownloadTile` → `HttpDownloadFile` from net.ll (both reached from page callbacks).
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
- **Download** — `DownloadTile` checks the cache with `PathOrFileExists` first, then calls
  `HttpDownloadFile` from net.ll, then sleeps 500 ms. That delay is deliberate: the
  [OSM tile usage policy](https://operations.osmfoundation.org/policies/tiles/) caps requests. Keep it.

`PageMapSync` walks a tile list and downloads it one tile per draw pass, showing progress.
`PageMap` reacts to a coordinate change by rebuilding the 2×2 grid and blitting the four tile
intersections into a 240×240 texture, then draws the position marker (green when the GPS has a fix,
orange otherwise).

## 8. GPS

`Sensor/GPS` parses NMEA `GGA` sentences (`$GNGGA`, `$GPGGA`) into `Model/GPSFixData`, converts
NMEA `DDMM.MMMM` to decimal degrees, applies the N/S and E/W sign, and pushes the result into
`DataManager`. `GPSFixData::set` splits the sentence with `TextHelper::Tokenize`.

**The UART is wired up in code but not in hardware.** `GPS.cpp` now calls `UARTInit`, `UARTIsEnabled`,
`UARTIsReadable` and `UARTGetChar` through hal.ll — the Linux `std::ifstream` layer and the old
pico-sdk `uart_*` calls are both gone. On the Simulator, `UARTIsEnabled` always returns `false`, so
`GetData` returns immediately without touching any data — the GPS is effectively silent until hardware
is connected. The `L96GPS` compile definition (set unconditionally at the end of `pedal.guru.cmake`)
guards the `$PMTK` configuration commands for the Quectel L96, which are now expressed as
`UARTPuts(GPS_UART, ...)` calls; the strings themselves are unchanged and hardware-validated.
Serial I/O now goes through hal.ll as the ground rules require — `TODO-D2` (wire up the hardware
and confirm the pin assignments) is the remaining open item.

## 9. Platform abstraction owned by pedal.guru

`src/Platform/<PLATFORM_NAME>/` now contains only the thin C++ wrappers that pedal.guru needs over
hal.ll's C API. `pedal.guru.cmake` interpolates `${PLATFORM_NAME}` into both the source list and the
include path, so the application includes `"Thread.hpp"` / `"Time.hpp"` with no `#ifdef` and gets the
right file — which is the same file on every platform, since all wrappers are now identical. Same
mechanism as fs.ll and gui.ll use for their own platform folders.

Each platform folder exposes:

| file | contract |
|---|---|
| `Thread.{cpp,hpp}` | `PedalGuru::Thread::NewThread(void(*)())` and `PedalGuru::Mutex` (constructor, `Lock`, `Release`) |
| `Time.{cpp,hpp}` | `PedalGuru::Time::Delay(unsigned int milliseconds)` |
| `CMakeLists.txt` | ESP32 only: ESP-IDF component registration |

All three platforms now share **identical** `.hpp` and `.cpp` files — the platform-specific logic lives
entirely in hal.ll. The headers include `HAL.h` in an `extern "C"` block, `Mutex` holds a `HALMutex`,
and the implementations call `MutexInit/Lock/Release` and `ThreadStart`. `Time.Delay` calls `::Delay`.
No platform SDK headers appear here anymore.

`HttpClient` left `src/Platform` in wave 5, moving to net.ll where it belongs. The single call site,
`OpenStreetMapAPI::DownloadTile`, now calls `HttpDownloadFile` from net.ll's `HttpClient.h`.

`HttpDownloadFile` is a stub on the two hardware platforms: it prints a message and returns `false`,
keeping the firmware linkable and a failed map sync diagnosable. On the RP2040 a real implementation
is blocked on hardware — no native wireless, needs the expansion board's radio module (§3). On the
ESP32-S3 it is pending work (`TODO-E1`). On the Simulator it is the same full POSIX + OpenSSL
implementation that always worked.

net.ll's `WiFi.c` **is** compiled into pedal.guru on all three platforms, and net.ll's radio libraries
are linked through the normal `PLATFORM_LIBRARIES` / `PLATFORM_REQUIRES` lists. The application needs
the radio — that is how map tiles get downloaded — so nothing about it is filtered out of the build.
On the RP2040 that requires `PICO_BOARD` to name a board declaring `PICO_CYW43_SUPPORTED`; see §10.

Do not add anything new to `src/Platform`: new platform code belongs in hal.ll (ground rule 6).

## 10. Build system

Two files, mirroring the pattern the submodules use.

**`pedal.guru.cmake`** declares `PLATFORM_NAME` (cached, `Simulator` by default, one of
`Simulator`/`RP2040`/`ESP32`), then **appends** the application's sources and include dirs to
`SOURCES` / `INCLUDE_DIRS`, and finally pins `HAL_LL_PATH` / `FS_LL_PATH` / `GUI_LL_PATH` to the three
submodules and includes `${GUI_LL_PATH}/gui.ll.cmake`.

### Two rules this file must obey

Both are the result of ESP-IDF's build model, and breaking either one is silent or confusing.

1. **No directory-scoped or target-scoped commands — variables and messages only.** ESP-IDF evaluates
   the component `CMakeLists.txt` (and therefore this file) in **script mode** (`cmake -P`, through
   `component_get_requirements.cmake`) to collect `REQUIRES` before the real configure. Commands such as
   `add_compile_definitions` do not exist in that mode and abort the ESP32 configure. `fs.ll.cmake` and
   `gui.ll.cmake` obey this too; that discipline is what makes a `.cmake` contract safe to include from
   an ESP-IDF component.
2. **Never clobber a `PLATFORM_NAME` the caller already set.** The cache default is written as
   `if(NOT PLATFORM_NAME)` + `set(... CACHE ...)`. In script mode there is no cache, so an unguarded
   `set(... CACHE ...)` is *not* skipped: it would overwrite the `ESP32` the component just set with
   `Simulator`, and `gui.ll.cmake` would then go looking for SDL2 in the middle of a firmware build.

### There is no platform macro

The platform is selected by the build — the source list and the include path (§9) — and **not** by the
preprocessor. No `SIMULATOR` / `RP2040` / `ESP32` macro is defined, and code must not test for one.
Validation lives in CMake instead, in two places, because each entry point needs it:

Both parts of the root check are needed: without the cache default an unset `PLATFORM_NAME` matches no
branch, and without the `else()` an invalid one configures "successfully" and writes a build system with
no target at all.

A `#error Platform must be informed!` guard used to live in `PedalGuru.cpp`. It was removed with the
macros, and it was ineffective anyway: with an unknown platform no source file was ever compiled, so the
`#error` was never reached.

**The validation lives only where the value can actually be wrong**, which is the root `CMakeLists.txt`:
`PLATFORM_NAME` comes from the user there, so the platform branch closes with an `else()` that raises
`FATAL_ERROR`. It is not repeated in `pedal.guru.cmake`, which every platform includes — that would run
the same check three times for a problem belonging to one entry point.

`src/Platform/ESP32/CMakeLists.txt` deliberately has **no** platform validation. It hardcodes
`set(PLATFORM_NAME "ESP32")`, so there is no user input to reject; and a wrong `PROJ_ROOT` is already
handled by the sentinel fallback just above, with the `include()` of `pedal.guru.cmake` failing loudly
right after if that fallback is also wrong. `pedal.guru.cmake` only defaults `PLATFORM_NAME` when the
caller left it unset.

### Why only gui.ll.cmake is included, and fs.ll.cmake is not

pedal.guru uses fs.ll directly (`FileSystem.h` in `PedalGuru.cpp`, `Texture.cpp` and the Simulator
`HttpClient.c`), so the naive wiring is to include both contracts. That does not work, and the reason is
worth knowing before anyone "restores" it:

`gui.ll.cmake` already includes its own versioned copies of `hal.ll.cmake` and `fs.ll.cmake` at its very
end, so including `fs.ll.cmake` here as well would only add a second path to the same file. pedal.guru
just points those nested includes at the right checkouts by setting the `*_PATH` variables first. Since
they are cached, the nested includes reuse them, so nothing is downloaded and every library shares the
one submodule.

**Include order between the sibling contracts no longer matters, and that is the whole point of wave 3.**
Until then it did, in a way that could not be satisfied: both libraries shipped a `HAL.h` and a
`HALConfig.h` under their platform folders, only gui.ll's carried the `LCD_*` pins, so gui.ll's platform
include directory had to come **before** fs.ll's — while `gui.ll.cmake` simultaneously had to come
**after** `fs.ll.cmake`, because it ended with
`list(REMOVE_ITEM SOURCES "${FS_LL_PLATFORM_DIR}/HAL.c")` and a later include would put that file back
and break the link with duplicate symbols. Two contradictory requirements propping up one duplication.
With the HAL in hal.ll there is a single `HAL.h` and a single `HALConfig.h` in the tree, the
`REMOVE_ITEM` line is gone, and the contracts compose in any order.

So the whole dependency stack enters through a single line, `include(${GUI_LL_PATH}/gui.ll.cmake)`,
preceded only by the three `*_PATH` variables (§2).

There is no `add_library` anywhere in the whole stack: each library appends to the two shared list
variables and the top-level target consumes them.

**`CMakeLists.txt`** branches on `PLATFORM_NAME`:

- `Simulator` — plain `project()`, include `pedal.guru.cmake`, `add_executable`, then
  `find_package(OpenSSL)` and link `${SDL2_LIBRARIES} m OpenSSL::SSL OpenSSL::Crypto`
  `${PLATFORM_LIBRARIES}`. OpenSSL is
  needed by the Simulator `HttpClient`; SDL2 is located by `gui.ll.cmake`.
- `RP2040` — defaults `PICO_BOARD` to `pico_w` (see below) and `PICO_SDK_PATH` to `~/pico-sdk`,
  `pico_sdk_init()`, include `pedal.guru.cmake`, `add_executable`, link
  `${PLATFORM_LIBRARIES}` (hal.ll publishes the pico-sdk target list; the hardcoded one it replaced
  named `hardware_adc`, which nothing uses, and omitted `hardware_uart`, which hal.ll needs),
  `add_compile_definitions(${PLATFORM_DEFINITIONS})` from net.ll, stdio over USB (UART off),
  `pico_add_extra_outputs`.
- `ESP32` — does **not** include `pedal.guru.cmake` itself. It points `EXTRA_COMPONENT_DIRS` at a
  platform folder and hands the build to ESP-IDF; that folder's `CMakeLists.txt` includes
  `pedal.guru.cmake` and calls `idf_component_register`. `EXTRA_COMPONENT_DIRS` currently points at
  `src/lib/Platform/ESP32`, **a path that does not exist in this repository** — see §16.

### The RP2040 build uses its own board header

```cmake
set(PICO_BOARD_HEADER_DIRS "${CMAKE_SOURCE_DIR}/src/Platform/RP2040/Boards")
set(PICO_BOARD WaveshareRP2040LCD1In28Extended)
```

The application needs the radio — that is how map tiles are downloaded — so net.ll's WiFi code is part
of the build, and on the RP2040 that only compiles when the pico-sdk exposes `pico/cyw43_arch.h`.

That header is gated on `PICO_CYW43_SUPPORTED`, which is declared **by the board header**. The plain
`pico` header, which this build used implicitly for a long time, does not mention CYW43 at all, so the
whole `pico_cyw43_arch*` target family is never created. Two things are worth remembering:

- **The failure is misleading.** `target_link_libraries` with a name that is not a target passes it to
  the linker as a plain library name, so configure succeeds and the build dies later on a missing
  header. The error points nowhere near the cause.
- **The stock `waveshare_rp2040_lcd_1.28` header does not solve it.** The pico-sdk ships one for the
  actual base board, and it correctly does *not* declare CYW43 support, because the bare board has no
  radio. Ours includes it and adds the radio on top — see §3 for the contents and the reasoning.

`PICO_BOARD_HEADER_DIRS` is searched before the SDK's own board directory, and the SDK ends up including
the file by absolute path, so the folder can follow this project's PascalCase convention rather than
being named `boards`.

`-DDEBUGMSGS` (no value needed) defines both `DEBUGMSGS` and `_DEBUG`. `DEBUGMSGS` is the one that
works: it is gui.ll's mechanism, which turns `SHOWDEBUG(...)` into `printf`. `_DEBUG` guards `std::cout`
traces in `PedalGuru.cpp` and `GPS.cpp` and is legacy — that path does not compile today (`TODO-C10`).

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
- **Comments are minimal and explain *why*** — see ground rule 5, which is binding. Existing good
  examples: the page cycle order, the OSM rate-limit delay, the 8px padding to centre a 256px tile on a
  240px screen.
- **Indentation is 4 spaces, ruler at 120 columns** (`.vscode/settings.json`).
- `TODO` comments are used to mark known-incomplete work. They are load-bearing; do not delete one
  without doing the work or asking.

## 13. Migration status: what still has to move out of pedal.guru

pedal.guru is meant to be pure application logic. It is not there yet. Known material that is either
platform-specific or infrastructure, and is a candidate to become (or move into) a submodule:

- **`src/Platform/*/HttpClient.*`** → **net.ll**: done in wave 5. `HttpDownloadFile` is the new name.
  The Simulator implementation is the same full POSIX + OpenSSL HTTP client; on the two hardware
  platforms it is a stub returning `false`.
- **`src/Platform/*/Thread.*`** and **`src/Platform/*/Time.*`** → **hal.ll**: done in wave 5. The three
  platform folders now hold identical thin C++ wrappers. There is **no `thread.ll`**: threading was
  folded into hal.ll because it is MCU abstraction of the same nature as GPIO and timers.
- **GPS UART access** (`src/Sensor/GPS.cpp`) → **hal.ll**: done in wave 5. The old Linux
  `std::ifstream` layer and the pico-sdk `uart_*` calls are replaced by `UARTInit/Deinit/IsEnabled/
  IsReadable/GetChar/UARTPuts`. The pins are `PLACEHOLDER` in `HALConfig.h` until the hardware is
  confirmed — `TODO-D2`. A `serial.ll` would only make sense for *protocol* on top (NMEA, the `$PMTK`
  commands), not for the peripheral. Not decided.

`TODO-E1` is closed.

### Wave 4 and net.ll: the decisions now live in net.ll's own AGENTS.md

Wave 4 is done, and everything it settled — the API shape, the Simulator's `netsh` route, poll mode,
`CYW43_LWIP=0`, the mocked fields, the traps — is documented in
`src/Dependency/net.ll/AGENTS.md`, which is where it belongs now that the library exists. **Read that
file before touching net.ll.** Only what pedal.guru itself has to know is kept here:

- **net.ll offers a WiFi scan and a download.** `WiFiInitialize` / `WiFiDeinitialize` /
  `WiFiScan(networks, maxNetworks, &found)`, filling an array the caller owns, and
  `HttpDownloadFile(url, filePath)` streaming the body straight to the card.
- **The download function lost its underscore.** `HttpClient_DownloadFile` became `HttpDownloadFile`; the
  old name was legacy. Parameters and behaviour are unchanged. Done in wave 5.
- **`HttpDownloadFile` is a stub on RP2040 and ESP32**, so the move to net.ll changed nothing about what
  works on hardware. Implementing it needs hardware to test on, and on the RP2040 also lwIP.
- **Connect is not implemented, on purpose**, so pedal.guru cannot join a network through net.ll yet. The
  intended flow — show the scanned networks, let the **user** pick, move on if joining fails — is
  recorded, not built.
- **Credentials are pedal.guru's problem, and only pedal.guru's.** net.ll asks for none, and no submodule
  may know `SettingsData` or pedal.guru exist. The dev has plans here; nothing is written down yet.
- **The OSM rate-limit delay stays here.** The `Time::Delay(500)` in `OpenStreetMapAPI::DownloadTile` is a
  rule of the tile API, not of HTTP, so it does not follow the client into net.ll.
- **Every network operation is synchronous and net.ll starts no thread.** That matches what pedal.guru
  already does — `HttpDownloadFile` blocks, and `PageMapSync` downloads one tile per draw pass on the UI
  thread (§6). One nuance: ESP-IDF's WiFi driver keeps internal tasks of its own once the radio is up.
  net.ll creates none; the SDK's cannot be switched off.
- **net.ll does not reach hal.ll**, because every platform hands over a whole stack rather than a bus. It
  depends on fs.ll, which is what brings hal.ll into the build.
- **net.ll's contract publishes a third list, `PLATFORM_DEFINITIONS`**, which hal.ll and fs.ll do not.
- **`net.ll`'s contract publishes a third list, `PLATFORM_DEFINITIONS`.** Done in wave 5: applied in
  `CMakeLists.txt`'s RP2040 branch at directory scope.

Two things about the boards, because they are easy to conflate: pedal.guru's RP2040 target is the
**Waveshare RP2040-LCD-1.28**, which has **no radio at all** — so wireless there waits on the expansion
board (`TODO-E3`) carrying Raspberry Pi's RM2. net.ll's RP2040 build therefore defaults to
`PICO_BOARD=pico_w`, a development vehicle, not this project's board. The ESP32-S3 has its radio natively.

Not implemented at all yet, from §1's product goal: the whole **"guru" coaching side** — hydration
and nutrition reminders, cadence guidance by stretch or elapsed time. There is no module for it
(`TODO-D4`).

## 14. Current status

**All three platforms compiled and linked clean, after wave 5.** Warning counts match exactly:

| platform | command | artifact | warnings |
|---|---|---|---|
| Simulator | `cmake -B build -DPLATFORM_NAME=Simulator && cmake --build build` | `build/pedal.guru`, ~818 KB | 6 (pre-existing) |
| RP2040 | `cmake -B build -DPLATFORM_NAME=RP2040 && cmake --build build` | `build/pedal.guru.uf2`, ~459 KB | 6 (pre-existing) |
| ESP32 | `idf.py -DPLATFORM_NAME=ESP32 build` | `build/pedal.guru.bin`, ~438 KB | 4 (pre-existing) |

All three now compile net.ll's WiFi code and link its radio libraries. The warning counts are unchanged
from wave 3, and every warning is one of the pre-existing `TODO-B10` / `TODO-B12` items — wave 5 added
none. The ESP32 binary grew from the ~422 KB wave 3 baseline, which is where the radio code landed.

The **Simulator has been run by the dev and works correctly**. Neither firmware has been flashed or
executed (§15).

### Warnings, and why the three platforms disagree

Zero errors everywhere. Warnings, measured:

| platform | total | in our code | in dependencies |
|---|---|---|---|
| Simulator | 6 | 6 | 0 |
| RP2040 | 6 | 6 | 0 |
| ESP32 | 4 | 2 | 2 |

The six in our code are `TODO-B10` (dead `x`/`y` in `ListTilesForArea`, 2 warnings) and `TODO-B12`
(4 × `-Wsign-compare` in `TextHelper.cpp` and `GPS.cpp`). Simulator and RP2040 report the identical set,
same files and lines.

The ESP32 numbers differ for two reasons, and neither means it is more lax or more strict overall:

- **It hides the sign-compare four.** ESP-IDF compiles with `-Wall -Werror=all -Wextra` but then adds
  `-Wno-sign-compare` and `-Wno-unused-parameter`. So on that axis our Simulator/RP2040 builds are
  *stricter* than ESP-IDF.
- **It shows two warnings from gui.ll.** ESP-IDF applies its flags to every source in the component,
  dependencies included. Our `-Wall -Wextra` is scoped to `PEDAL_GURU_SOURCES` on purpose, so vendored
  code stays quiet. Those two are `TODO-E4`: a redundant guard in gui.ll's `CanvasDrawPoint`, noted and
  left alone.

Consequence worth remembering: a clean build on one platform does not mean a clean build on another,
and that is a property of the toolchains, not of the code.
- Git branch: `feature/RP2040-migration`.
- The working tree has **uncommitted changes to `.gitignore` and `.gitmodules`**: they comment out
  `ignore = all` on the fs.ll and gui.ll submodules and comment both paths out of `.gitignore`, and they
  add `hal.ll` and `net.ll` as submodules. That is the move back to real submodules described in §2 —
  expected, not a leftover to revert. Do not commit or revert them on your own initiative.
- Wave 3's edits live **inside the gui.ll submodule** and are uncommitted there too, so `git status` here
  shows gui.ll as modified. They have to be committed and pushed in gui.ll's own repository before the
  submodule pointer here can be bumped.

## 15. Not verified

- All three builds in §14 were **executed** in isolated build directories, through to a linked artifact.
  The **Simulator was run by the dev and behaved correctly before wave 3**; after wave 3 it has only been
  *built*, not visually checked. gui.ll's own Simulator sample was started post-wave-3 and ran without
  crashing or printing an error, which says the SDL window comes up and nothing aborts — it says nothing
  about what is on screen. **A visual pass on both is still owed.**
- The RP2040 and ESP32 firmwares have **never been flashed or run**. They compile and link; nothing more
  than that is established. In particular the `Mutex` constructor running during static initialization,
  before `main`/`app_main`, is reasoned about and compiles on both, but is unverified at runtime — on
  ESP32 it allocates via `xSemaphoreCreateMutex()` at that point (§6).
- Nothing in this file is validated **on hardware**. Statements about the RP2040 and ESP32 platform
  code come from reading and compiling it.
- Everything in §3 about the planned expansion board (microSD slot, two reed switches, GPS, WiFi+BT
  module for the RP2040, single shared design, waterproof case, magnetic ring), and everything in §1
  about the coaching features, is **stated by the dev** and has no counterpart in the code yet. The
  parts that *are* in code are the SD card and card-detect wiring, which live in hal.ll's
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
decision by the dev**, not a drive-by fix. **Every item here has a `TODO-*` entry in §18** — that is the
list to work from. Do not fix any of these as a side effect of unrelated work.

**Build-breaking:**


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
  in the loop headers. Harmless, but the outer pair is dead. Now reported by `-Wunused-variable`.
- `TODO-B12` — four `-Wsign-compare` warnings in `TextHelper.cpp` and `GPS.cpp`: a signed loop counter
  compared against `size_t` / `std::string::size_type`.

Not a defect, recorded so it is not "fixed" by accident: `COLOR_MAGENTA` and `COLOR_TRANSPARENT` are
deliberately the same value (`255, 0, 255`), matching gui.ll's colour key. The consequence — magenta
cannot be used as a real drawable colour — is the accepted cost of colour-key transparency in RGB565.

**Stale / cosmetic:**

- `TODO-C3` — `src/Model/SensorData.hpp` is an empty struct, referenced by `DIAGRAM.md` but by no code.
- `TODO-C4` — `TaskManager::ReadSettings` hardcodes the settings with a `TODO`; nothing reads or writes
  settings from the card yet.

## 17. In progress: extracting the HAL into hal.ll

A five-wave migration agreed with the dev. **Read this before touching any `.cmake` file or any
platform code**, because between waves the tree is deliberately inconsistent.

| wave | what | state |
|---|---|---|
| 1 | create hal.ll | **done, pushed** |
| 2 | migrate fs.ll onto hal.ll | **done, pushed** |
| 3 | migrate gui.ll onto hal.ll | **done, pushed** |
| 4 | create net.ll (**copy** the `HttpClient` into it; pedal.guru untouched) | **done, not pushed yet** |
| 5 | migrate pedal.guru onto net.ll, and empty `src/Platform` | **done, not pushed yet** |

**Wave 5 is done.** Everything in `src/Platform` is now a thin wrapper or the ESP32 CMake component:
`HttpClient` moved to net.ll (`HttpDownloadFile`), `Thread`/`Time` wrap hal.ll's C API, and `GPS.cpp`
calls hal.ll's UART functions. The three platform folders are as thin as they can be without disappearing
altogether — they hold the C++ wrappers pedal.guru needs because it is C++, which is exactly what
pedal.guru is and the submodules are not. Details and measured numbers are in §14.

**The wave 4/5 boundary was deliberate**, and it is the one place the earlier plan was ambiguous. Wave 4
ended with net.ll standing on its own — its contract, its `Sample.c`, its three platforms, its
`AGENTS.md` — and with pedal.guru untouched, still compiling its own `src/Platform/*/HttpClient.*`. That
was checked rather than assumed: `git diff` on `src/Platform` came back empty, the old
`HttpClient_DownloadFile` name is still in place here, and all three artifacts rebuilt **byte-identical**
to wave 3's (823064 / 464384 / 421904 bytes, same 6/6/4 warnings).

Only wave 5 deleted that folder, rewired `OpenStreetMapAPI.cpp:130` to `HttpDownloadFile`, applied net.ll's
`PLATFORM_DEFINITIONS` in the RP2040 branch, and took on the rest of `TODO-E1` (`Thread` and `Time` onto
hal.ll, the GPS UART). Keeping the application working through wave 4 was the point of the split.

### What wave 5 did

- **`OpenStreetMapAPI.cpp:130`** rewired to `HttpDownloadFile`. The old `HttpClient_DownloadFile` name
  was legacy and the underscore is gone.
- **`src/Platform/*/HttpClient.{c,h}`** deleted. net.ll's `HttpClient.h` now resolves through the
  `src/lib` include path that `net.ll.cmake` appended.
- **`pedal.guru.cmake`**: `NET_LL_PATH` pinned; `net.ll.cmake` included after gui.ll's, with no filtering
  — net.ll's sources, include dirs and radio libraries all enter the build normally.
- **`CMakeLists.txt` RP2040 branch**: `PICO_BOARD` defaults to `pico_w`. This is required, not cosmetic:
  `PICO_CYW43_SUPPORTED` is declared by the *board header*, and without it the pico-sdk never creates the
  `pico_cyw43_arch*` targets, so net.ll's `WiFi.c` cannot find `pico/cyw43_arch.h`. The plain `pico`
  header has no CYW43 mention at all. Note the failure mode: `target_link_libraries` with a name that is
  not a target is passed through to the linker as a plain library name, so the configure step succeeds
  and only compilation fails — the error points at a missing header, not at the real cause.
- **`CMakeLists.txt` RP2040 branch**: `add_compile_definitions(${PLATFORM_DEFINITIONS})` added at
  directory scope, because `CYW43_LWIP=0` has to reach pico-sdk's own cyw43 sources.
- **`src/Platform/*/Thread.{cpp,hpp}`**: all six files are now identical. `.hpp` has `#pragma once`,
  `extern "C" { #include "HAL.h" }`, and `Mutex` using `HALMutex`. `.cpp` calls `MutexInit/Lock/Release`
  and `ThreadStart`. No platform SDK headers remain in the headers — the old `pthread.h`,
  `pico/mutex.h`, `freertos/semphr.h`, and the lambda+`xTaskCreate` in the ESP32 `.cpp` are gone.
- **`src/Platform/*/Time.{cpp,hpp}`**: same treatment. `.hpp` is now uniform across all three (fixing
  `TODO-C9` — the Simulator header was missing `#pragma once`). `.cpp` calls `::Delay`.
- **`src/Sensor/GPS.cpp`**: Linux `std::ifstream` layer and pico-sdk `uart_*` calls removed. Now calls
  `UARTInit(GPS_UART, GPS_UART_BAUDRATE, GPS_UART_TX_PIN, GPS_UART_RX_PIN)` in `Enable`, `UARTDeinit`
  in `Disable`, and `UARTIsEnabled`/`UARTIsReadable`/`UARTGetChar` in the read path. `GetData` guards
  with `UARTIsEnabled` — on the Simulator that always returns `false`, so the GPS is silent until
  hardware is connected. The `#ifdef L96GPS` block is intact; only the function name changed from
  `uart_puts(uart0,` to `UARTPuts(GPS_UART,`.

What wave 4 established, and what it did not: net.ll configures, compiles and links on all three platforms
with **zero warnings**, and its sample **ran on the Simulator**, listing the real networks in range through
`netsh.exe` — including a hidden one, and showing the scan's non-repeatability across calls. An external
consumer with nothing pinned also cloned fs.ll and hal.ll by itself and built clean. But **neither firmware
was flashed**, so net.ll's RP2040 and ESP32 `WiFi.c` is new code that has never executed, and
`HttpDownloadFile` has only ever run on the Simulator. net.ll's `AGENTS.md` §10 is the authoritative list.

Two bugs surfaced while implementing it, both worth knowing because they are the kind that compile
silently: net.ll's own auth-mode enum values collided with ESP-IDF's `wifi_auth_mode_t` (C enum values
share one scope), and the RP2040's scan `auth_mode` turned out **not** to be a `CYW43_AUTH_*` constant
despite the driver's own comment — those are 32-bit connect values, while the scan field is a `uint8_t`
bitmask built from beacon information elements. Details in net.ll's `AGENTS.md` §6.

**pedal.guru builds again on all three platforms** — that was wave 3's exit criterion, and §14 has the
measured numbers. The breakage wave 3 cleared was in gui.ll's versioned copy of `fs.ll.cmake`: it was the
pre-wave-2 version, still listing `HAL.c` and `RTC.c`, files fs.ll no longer has, so any configure died
with `Cannot find source file: fs.ll/src/lib/Platform/Simulator/RTC.c`. It could not be patched in
isolation, because refreshing that copy alone would have put hal.ll's `HAL.c` *and* gui.ll's own `HAL.c`
in `SOURCES` and broken the link with duplicate symbols.

### Why the HAL was extracted

fs.ll and gui.ll each shipped their own `HAL.h`, `HAL.c` and `HALConfig.h`, with the same filenames
**and the same include guards**. Only one was ever textually included; the build made gui.ll's win by
include-path precedence and then deleted fs.ll's `HAL.c` from the source list so `Delay` and
`STDIOInitAll` would not collide at link time. Two invisible, order-dependent hacks holding up a
duplication. Measured: those were the only two colliding filenames in the whole tree, so moving them to
one place removes the ordering rule entirely — the contracts compose in any order afterwards.

### What wave 3 did

The nine planned items, all applied to gui.ll:

1. **Deleted** `src/lib/Platform/<P>/HAL.{c,h}`, `HALConfig.h` and `src/lib/Types.h`. All of it comes from
   hal.ll now, `Types.h` included (it carries `UINT8/16/32` and `DateTime`). Checked before deleting:
   hal.ll's `HAL.h` and `HALConfig.h` are strict supersets, and every pin value is identical.
2. **Consumes hal.ll**: `hal.ll.cmake` copied into `src/Dependency/` and included from `gui.ll.cmake`,
   with `src/Dependency/hal.ll` added to `.gitignore` (resolved through `HAL_LL_PATH`, not a submodule).
3. **Refreshed gui.ll's stale copy** of `src/Dependency/fs.ll.cmake` from fs.ll's current one.
4. **Dropped the `list(REMOVE_ITEM SOURCES "${FS_LL_PLATFORM_DIR}/HAL.c")` line.**
5. **SPI calls gained their bus argument** in `src/lib/Driver/GC9A01/Driver.c` and
   `src/lib/LCD/1in28/LCDRenderer.c`.
6. **`DateTime` left `Canvas.h`.** `CanvasDrawTime` keeps its signature.
7. **Early script-mode `return()` in `gui.ll.cmake`**, which includes `hal.ll.cmake` before returning so
   `PLATFORM_REQUIRES` still reaches ESP-IDF. The old partial guard on the clone is gone, and so is the
   `if(NOT CMAKE_SCRIPT_MODE_FILE)` around `configure_file` — unreachable in script mode now, and leaving
   it would imply otherwise.
8. **Uses the contract's link lists**: `${PLATFORM_LIBRARIES}` in `target_link_libraries`,
   `${PLATFORM_REQUIRES}` in `idf_component_register`'s `REQUIRES`.
9. **The Simulator `Delay` stayed plain in hal.ll.** hal.ll's `AGENTS.md` §9 preserves the original
   SDL-aware implementation and a sketch for bringing it back.

Three things came up that the plan did not anticipate:

- **`src/Sample.c` still included `RTC.h`**, which fs.ll deleted back in wave 2. `RTCInitialize` is in
  hal.ll's `HAL.h`, already included on the line above, so the include was simply removed.
- **`GUI_LL_PLATFORM_DIR` and gui.ll's platform-folder check are gone**, and the now-empty `Simulator`
  and `RP2040` folders under `src/lib/Platform` were deleted with them. With the HAL out, the only file
  left there is the ESP32 component's `CMakeLists.txt`; git cannot version an empty directory, so the
  existence check would have failed on a fresh clone anyway. An unsupported `PLATFORM_NAME` is rejected
  by hal.ll's contract instead. **Approved by the dev.**
- **`src/lib/Helper/Debug.h` was deleted too**, for the same reason as `HAL.h`: it duplicated hal.ll's
  with the same filename *and* the same `DEBUG_H` guard, gui.ll's winning by include-path precedence. The
  preprocessor-visible content was character-identical (the file differed only by hal.ll's licence header
  and by being CRLF), so `SHOWDEBUG` now comes from hal.ll and nothing changed behaviourally — verified
  with a `-DDEBUGMSGS` build, and every artifact came out byte-identical in size. `src/lib/Helper` keeps
  `Trigonometry.{c,h}`, so it stays on the include path. **Approved by the dev.**

Verified before declaring the wave done: gui.ll standalone on all three platforms, gui.ll standalone with
**nothing** pinned (which cloned hal.ll and fs.ll itself and still built clean), and pedal.guru on all
three platforms. Numbers in §14.

## 18. TODO list

The working backlog. When the dev asks what there is to do, **this is the list to return**.

Nothing here is scheduled, and nothing here is authorized: each item still needs the dev's go-ahead
before being worked on (ground rule 2). Items are grouped by kind and carry a stable ID so they can be
referenced in conversation. Keep this list up to date: when an item is done, remove it and update the
section it came from; when a new one is found, add it here rather than fixing it silently.

### A. Build-breaking — only the Simulator builds today

The RP2040 and ESP32 platform folders under `src/Platform` do not compile. Verified in §14. These are
the items that stand between the project and a three-platform build.

Done so far: `TODO-A1` (ESP32 component path), `TODO-A2` (RP2040 `Time`), `TODO-A3` + `TODO-A4` (the
`Mutex` rework — see §6), `TODO-A5` (the empty `Time`/`HttpClient` files), `TODO-A6` (the fs.ll/gui.ll
include ordering), `TODO-A7` (the `add_compile_definitions` calls, replaced by CMake-level validation —
§10), `TODO-B11` (the unguarded read in `DataManager::Pop`) and `TODO-C2` (the non-existent
`src/GUI/Interface` include dir, which ESP-IDF rejects outright rather than ignoring).

Each round of fixes uncovered the next blocker, because the build never got far enough to reach it.
Expect that to continue: the list below is what is known now, not necessarily all that remains.

**Group A is empty: all three platforms build.** `TODO-A8` was the last item, and closing it also
uncovered and fixed `TODO-A9` — `app_main` was defined in a C++ translation unit without `extern "C"`,
so ESP-IDF could not find it at link time. That one only became visible once the ESP32 build got as far
as linking, for the first time.

### B. Bugs and correctness

| ID | Item |
|---|---|
| `TODO-B1` | `HIDHandler::RegisterEventHandler` returns `list::end()` instead of an iterator to the pushed element; `UnregisterEventHandler` then erases `end()`. Fixing this is what unblocks `GUINavigator::UnregisterEvents()`, currently commented out. |
| `TODO-B2` | `GUINavigator::GoToNextPage` / `GoToPreviousPage` both dereference `end()` at the ends of the page cycle. |
| `TODO-B3` | `TaskManager::GetDevicesData`'s iterator never advances past the first device (`device++` inside a ternary). |
| `TODO-B12` | Four `-Wsign-compare` warnings: `TextHelper.cpp:27,35,60` and `GPS.cpp:69` compare a signed loop counter against `size_t`/`std::string::size_type`. Surfaced by the new `-Wall -Wextra`. |
| `TODO-B5` | `PageMapSync::DrawPageContents` uses a runtime-sized VLA plus `sprintf`, with a size formula unrelated to what it writes. |
| `TODO-B6` | `PageMap::previousLatitude` / `previousLongitude` are never initialized. |
| `TODO-B7` | `COLOR_LL` casts to signed `short`, which breaks gui.ll's colour-key transparency (gui.ll Decision 14 requires `UINT16`). |
| `TODO-B8` | `Texture` has no destructor; `Release()` must be called by hand. Candidate for RAII. |
| `TODO-B9` | `GPS::GetData` can spin 50 times per pass against a stubbed `UartGetLine`. |
| `TODO-B10` | `OpenStreetMapAPI::ListTilesForArea` has a dead, shadowed `int x = 0, y = 0;`. |

### C. Obsolete / cleanup

| ID | Item |
|---|---|
| `TODO-C3` | `src/Model/SensorData.hpp` is an empty struct used by nothing. Fill it in or drop it. |
| `TODO-C4` | `TaskManager::ReadSettings` hardcodes the settings; no persistence to or from the card yet. |
| `TODO-C8` | Licence headers are missing in places. All of pedal.guru's `src` is covered now, but the submodules are not, and they need a header **adapted to their own context** (fs.ll and gui.ll are libraries with their own identity, not pedal.guru files). Decide the wording per repository before mass-applying anything. |
| `TODO-C9` | ~~`src/Platform/Simulator/Time.hpp` has no `#pragma once`, unlike its siblings.~~ Fixed in wave 5. |
| `TODO-C10` | **Legacy debug plumbing, needs review — not urgent.** `DEBUGMSGS` is gui.ll's mechanism (`Helper/Debug.h` turns `SHOWDEBUG` into `printf`), and it works. What is legacy is the pedal.guru side: `CMakeLists.txt` also defines `_DEBUG`, and `PedalGuru.cpp` / `GPS.cpp` guard `std::cout` traces on it. That path does not even compile — `PedalGuru.cpp` uses `std::cout` without including `<iostream>` (pre-existing, verified identical to `HEAD`). Decide later whether pedal.guru gets its own tracing or just adopts `SHOWDEBUG`; fix it when the tracing is actually needed. |
| `TODO-C5` | `DIAGRAM.md` is stale: links to a `PedalGuru` repo with paths that no longer exist, and a `TaskManager` API that no longer matches. Refresh or drop. |
| `TODO-C6` | `.vscode/c_cpp_properties.json` is stale: references `src/Target/**` and `src/Dependency/pico-sdk` (neither exists) and defines `TARGET_RP2040` instead of the `RP2040` the build actually sets. The project uses clangd, not the MS C/C++ extension — so this may be droppable outright. |
| `TODO-C7` | `Device/DIY/wired_reel_speedometer.cpp`: out of the build, `snake_case` name, class declared inside the `.cpp` with no header, does not implement all of `Device`'s pure virtuals. Keep as a placeholder, complete and rename to `WiredReelSpeedometer.{cpp,hpp}`, or remove until the device is really supported. |

### D. Pending implementation

| ID | Item |
|---|---|
| `TODO-D1` | Wire the HID events to real hardware: the `HIDHandler` firing functions (`EnterDown`, `ExitPressed`, …) are private and nothing calls them. They are meant to become GPIO interrupt callbacks for the two navigation switches (§3, §6) — reed versus hall sensor is still the dev's open choice. Their pins also need reassigning, see `TODO-D2`. Until then there is no user input at all. |
| `TODO-D2` | Wire up the GPS hardware and validate the read loop with a module attached. `GPS.cpp` calls hal.ll's UART API since wave 5, and the pins are assigned (§3), but nothing has been tested against a real GPS. Three things are deliberately left for that moment, because they need observed behaviour rather than reasoning: **(a)** `UARTGetChar` blocks on both hardware platforms, and `UartGetLine` relies on `UARTIsReadable` to avoid hanging when the module goes quiet — the cost is that a line can come back truncated if the buffer drains mid-sentence, with no way for the caller to tell. Returning a `bool` for "reached end of line", so a partial read is discarded instead of parsed, is the agreed shape when this is picked up. **(b)** The loop breaks on `\n` but keeps the `\r`, and NMEA ends in CRLF, so the trailing `\r` lands in the last token — today that is `checksum` in `GPSFixData`, which is stored and never verified, so it is harmless until someone verifies it. **(c)** `\0` is treated as a valid line terminator, which is the behaviour that worked on the RP2040; it is not obviously right for NMEA and should be re-examined against real data. On the Simulator `UARTIsEnabled` always returns `false`, so the GPS is silent there until a mock is designed. |
| `TODO-D3` | Implement the stub pages: `PageAltimetry`, `PageDistance`, `PageHillsGraph`, `PageRoute`, `PageSummary`. Only `PageMap` and `PageMapSync` do anything today. |
| `TODO-D4` | Implement the whole **"guru"** side — the coaching features from §1: hydration and nutrition reminders, cadence guidance by road stretch or elapsed training time. No module exists for it yet. |
| `TODO-D5` | Support multi-sensor devices, e.g. the same device measuring cadence on the pedal or speed on the wheel depending on configuration (§5). The `Device`/`Sensor` split already allows it; nothing exercises it. |
| `TODO-D6` | Navigation *within* a page (menus, buttons), not just between pages. `GUINavigator` only cycles pages today (§6). |

### E. Architecture / migration

| ID | Item |
|---|---|
| `TODO-E1` | ~~Empty `src/Platform` out into the collection.~~ Done in wave 5 — `HttpClient` → net.ll, `Thread`+`Time` → hal.ll wrappers, GPS UART → hal.ll. What remains is `TODO-D2` (connect hardware and confirm GPS pin assignments) and the WiFi connect path in net.ll. |
| `TODO-E2` | Decide SD card thread safety. fs.ll's `AGENTS.md` §12 leaves it open and says the decision is to be driven from here. Current state, verified: all card access happens on the UI thread and `DataManager` does **not** mediate it, so the card is single-threaded by accident of call placement rather than by design (§6). This needs a decision before anything on the sensor thread starts writing to the card — e.g. ride logging, which `TODO-D4` and `TODO-C4` both imply. |
| `TODO-E3` | Design the open-hardware expansion board (microSD slot, two reed/hall switches, GPS, WiFi+BT module for the RP2040) as a single shared board. **The radio is the exception to the shared-pin rule**, decided by the dev: it is an RP2040-only module, so it may use pins that exist only there, and a board destined for the ESP32-S3 simply leaves that footprint unpopulated. See §3 for the pin budget and the mapping decided so far. |
| `TODO-E4` | **In gui.ll, not here. Just noted, no action planned.** `src/lib/GUI/Canvas.c:129` — the `< 0` guard in `CanvasDrawPoint` is always false (`pixelSize` is an enum with unsigned underlying type, so the whole expression is unsigned), and it has no effect: the wrapped value truncates to ~65534 at the `UINT16` parameter and `CanvasSetPixel`'s own bounds check discards it. So the guard is redundant, not harmful. Whoever touches it should note it is a `break`, not a `continue` — a guard that actually fired would abandon the remaining inner-loop pixels, so making it "work" as written would be a regression. |
