# Requirements Document

## Introduction

pedal.guru runs on a device mounted on a bicycle and is designed to work offline while the rider is on
the road, where no network is available. Network connectivity is only expected at home, on the rider's
own WiFi, before a ride or after returning — that is when map tiles and other remote content can be
fetched.

A device taken out of its box carries no configuration at all, and it must be configured before it can
do anything useful. Instead of building configuration screens on a 240x240 round display driven by two
magnetic switches, the device serves a plain HTML+CSS+JS page from itself: when no credentials are found
in the `settings` file at the root of the SD card, pedal.guru scans for the networks in range, starts its
own open WiFi access point named `pedal.guru`, shows that name and the configuration URL
`http://192.168.33.1/` on the display, and serves the configuration page to a phone that joins that
access point. The page
lets the rider pick a WiFi network and type its password; on save, the device writes the SSID and the
password to the SD card, holds a confirmation on the display, and restarts into normal operation.

This feature is the groundwork for the download features that follow: without stored credentials the
device can never sync map tiles. It stops at storing the credentials and at exposing a station connect
operation in net.ll. Actually joining the configured network is `PageMapSync`'s job and is future work.

### Scope of this document

In scope:

- deciding at startup whether the device is already provisioned, by looking for the Credential_Store on
  the SD card;
- a provisioning mode that scans for networks, brings up an access point and serves the configuration
  page;
- three HTTP endpoints on port 80: `GET /` returning the configuration page, `POST /settings/save`
  receiving the selected network and its password and writing them to the SD card, and
  `GET /network/scan` attempting a fresh scan on the rider's request;
- returning to normal operation by restarting the device once the credentials are stored;
- exposing a station connect operation in Net_LL, and reading the stored SSID and password in
  Pedal_Guru.

Out of scope, and deliberately so:

- **joining the configured network.** The station connect is exposed but never called by this feature.
  Driving it, reporting "network not available" to the rider, and any Connect_Timeout belong to the
  `PageMapSync` work that follows (Decisions, D14);
- connection retry and re-provisioning a network that no longer works (D9);
- behaviour when the SD card is absent or cannot be mounted — the card is assumed present (D12);
- an access point on the Simulator (D-AP);
- downloading anything over the connected network (`TODO-D3` / map sync already exists as a page and is
  not touched here);
- configuring anything other than the WiFi SSID and password through the page (page selection, map sync
  area and the rest of `SettingsData` stay where they are — see `TODO-C4`);
- Bluetooth pairing, the Web Serial API, and any variant of the configuration page hosted on the public
  web. All were considered by the dev and rejected: they demand more technical skill from the rider and
  an online connection.
- automated tests of any kind. Verification for this project is a clean build plus observed behaviour
  on the Simulator or on the device (`AGENTS.md` ground rule 8).

### What exists today, and what has to be built

Verified in the code, so that the requirements below do not read as if the plumbing were already there:

- **net.ll offers a WiFi scan and nothing else on the radio side.** `src/Dependency/net.ll/src/lib/WiFi.h`
  declares only `WiFiInitialize`, `WiFiDeinitialize` and `WiFiScan`. There is **no station connect, no
  access point mode, and no HTTP server**. net.ll's `AGENTS.md` §11 records connect as deliberately
  unimplemented.
- **`HttpDownloadFile` is the only HTTP surface** (`src/Dependency/net.ll/src/lib/HttpClient.h`), it is
  download-only, and it is a **stub returning `false` on both RP2040 and ESP32**. Only the Simulator has
  a real implementation.
- **The Simulator has no radio of its own.** net.ll's Simulator scan shells out to `netsh.exe` under
  WSL. There will be no access point on that platform at all; its HTTP server is a POSIX socket server.
- **Settings are hardcoded.** `TaskManager::ReadSettings` assigns the six page flags and
  `mapSyncingBaseUrl` in code with a `TODO`; `SettingsData` (`src/Model/SettingsData.hpp`) has no
  network fields, and nothing in pedal.guru reads or writes settings from the card yet.
- **fs.ll gives byte-level file access only**: `MountSdCard`, `SelectActiveDrive`, `OpenFile`,
  `CloseFile`, `ReadFile`, `WriteFile`, `UnMountSdCard`, `PathOrFileExists`, `CreatePathDirectories`.
  That list is what exists today and not the final surface — this feature adds one function to it,
  `TruncateFile` (Decisions, D27). There is no seek. There is also no configuration file format anywhere
  in the project, so this feature introduces the first one.
- **The card is mounted in `app_entry` before any thread exists** and unmounted after, and every card
  access today happens on the UI thread. fs.ll thread safety is still an open decision (`TODO-E2`).
- **There is no user input yet.** The `HIDHandler` firing functions (`EnterDown`, `ExitPressed`, …) are
  private and nothing calls them (`TODO-D1`), so a provisioning screen cannot expect a button press to
  advance.
- **The page list is built at one call site.** `TaskManager::Execute` constructs `GUIDrawer drawer;`
  (`src/TaskManager.cpp` line 49) and calls `CreatePages(drawer);` on line 50, then constructs
  `HIDHandler handler;`, `GUINavigator guiNavigator(handler, pages_);` and calls `drawer.Execute();` on
  line 53. `CreatePages` fills `pages_` in page cycle order and always appends `PageMapSync` last.
  `GUINavigator`'s constructor returns early when `pages_` is empty, otherwise calls `Setup()` on the
  first page; `GUIDrawer::Execute` runs the render loop over the three callbacks a `BasePage` registers
  through `Setup()`.

So the greater part of the work for this feature lands in **net.ll** (access point mode, an HTTP server,
station connect), and pedal.guru consumes it. The requirements below are written against that boundary
and name which side owns each behaviour.

## Glossary

- **Pedal_Guru**: the application in this repository, the layer that decides which mode to run in, what
  to display, and what content the HTTP endpoints answer.
- **Net_LL**: the `net.ll` submodule at `src/Dependency/net.ll`, which owns every network operation.
  No hardware access, no sockets and no radio calls belong in Pedal_Guru.
- **FS_LL**: the `fs.ll` submodule, which owns all SD card access.
- **HAL_LL**: the `hal.ll` submodule, which owns every hardware access other than the radio.
- **Credential_Store**: the file named `settings` at the root of the SD card, holding `key=value` pairs
  one per line, among them the WiFi SSID and its password in plaintext under the keys `WIFI_SSID` and
  `WIFI_PWD`.
- **Maximum_Request_Size**: 2048 bytes, the largest HTTP request the Provisioning_Server accepts,
  counting the request line, the headers and the body together.
- **Maximum_Connections**: 1, the number of connections the Provisioning_Server serves at a time.
- **Provisioning_Mode**: the startup mode Pedal_Guru enters when no usable Credential_Store is found. In
  this mode the Network_List has been scanned, the radio runs as an access point where the platform
  supports one, the Provisioning_Server is serving, and the display shows the Provisioning_Screen.
- **Normal_Mode**: the startup mode Pedal_Guru enters when a usable Credential_Store is found. In this
  mode the ride pages run as they do today.
- **Provisioning_Access_Point**: the open WiFi access point Pedal_Guru brings up in Provisioning_Mode,
  with the fixed SSID `pedal.guru`, no password and the fixed IP address `192.168.33.1` on every
  platform, for a phone to join.
- **Provisioning_Server**: the HTTP server Net_LL runs in Provisioning_Mode, which routes each request
  to an Endpoint_Callback registered by Pedal_Guru.
- **Endpoint_Callback**: a C function pointer Pedal_Guru registers with the Provisioning_Server for one
  path, which produces that path's response. Net_LL never knows what is being served.
- **Configuration_Page**: the plain HTML+CSS+JS page compiled into the firmware as a constant and served
  by an Endpoint_Callback at the URL `http://192.168.33.1/`, with no external assets and no internet
  dependency.
- **Network_List**: the set of WiFi networks in range, as reported by Net_LL's scan, held in memory by
  Pedal_Guru and offered on the Configuration_Page for the rider to choose from.
- **Provisioning_Screen**: what the display shows while in Provisioning_Mode, drawn by the page class
  `PageProvisioning`, which takes no part in the page cycle.
- **SSID**: a WiFi network name, up to 32 bytes. **BSSID**: the MAC address of one access point radio.
  Net_LL's scan reports one entry per BSSID, so the same SSID can appear more than once.
- **Rescan_Trigger**: the button on the Configuration_Page through which the rider asks for a freshly
  scanned Network_List.
- **Confirmation_Hold**: 5 seconds, the minimum duration the Provisioning_Screen keeps the "device has
  been configured" indication on the display before the device restarts.

## Requirements

### Requirement 1: Decide the startup mode from the SD card

**User Story:** As a rider, I want the device to configure itself only when it has never been
configured, so that a device I have already set up boots straight into the ride screens.

#### Acceptance Criteria

1. WHEN Pedal_Guru starts, THE Pedal_Guru SHALL mount the SD card and select the active drive through
   FS_LL, and THEN check through FS_LL's `PathOrFileExists` whether the Credential_Store exists at the
   root of the card, before creating the devices, the sensor thread or the pages.
2. IF the Credential_Store exists and its content yields a non-empty SSID of at most 32 bytes on a
   `WIFI_SSID=<name>` line together with a value on a `WIFI_PWD=<password>` line, THEN THE Pedal_Guru
   SHALL enter Normal_Mode.
3. IF no file named `settings` exists at the root of the SD card, THEN THE Pedal_Guru SHALL enter
   Provisioning_Mode.
4. IF the Credential_Store exists but FS_LL reports a read failure, or its content yields an empty SSID
   or holds no `WIFI_PWD` line, THEN THE Pedal_Guru SHALL enter Provisioning_Mode and SHALL leave the
   existing Credential_Store content unchanged.
5. WHILE the startup mode is being decided, THE Pedal_Guru SHALL perform read-only access to the SD
   card and SHALL NOT create, truncate or write any file.
6. THE Pedal_Guru SHALL decide the startup mode exactly once per program run, before the sensor thread
   exists, and SHALL keep that mode in effect until the device restart of Requirement 7.
7. WHILE Provisioning_Mode is active, THE `TaskManager` SHALL push exactly one page, an instance of
   `PageProvisioning`, into `pages_` at the call site that calls `CreatePages` in Normal_Mode
   (`src/TaskManager.cpp` line 50), and SHALL NOT call `CreatePages`.
8. WHILE Normal_Mode is active, THE `TaskManager` SHALL call `CreatePages` at that same call site,
   producing the same `pages_` content and the same page cycle order it produces today.
9. THE `TaskManager` SHALL construct `HIDHandler`, construct `GUINavigator` over `pages_` and call
   `GUIDrawer::Execute` identically in Provisioning_Mode and in Normal_Mode, so that the mode decision
   changes the content of `pages_` and nothing else.
10. WHEN Pedal_Guru reads the Credential_Store, THE Pedal_Guru SHALL take the `WIFI_SSID` and `WIFI_PWD`
    values from wherever those two lines appear in the file, SHALL accept them separated by any number
    of other lines and in either order, and SHALL retain every `key=value` line whose key it does not
    recognise, so that the file can later carry the rest of `SettingsData` (`TODO-C4`).

### Requirement 2: Provisioning mode scans, then brings up an access point

**User Story:** As a rider with a brand-new device, I want the device to publish its own WiFi network,
so that I can reach its configuration page from my phone with no other equipment.

#### Acceptance Criteria

1. WHEN Pedal_Guru enters Provisioning_Mode, THE Pedal_Guru SHALL ask Net_LL to initialize the radio
   and, only after that initialization reports success, SHALL ask Net_LL for a scan and hold the
   resulting Network_List in memory, and only after that scan has returned SHALL ask Net_LL to start the
   Provisioning_Access_Point.
2. THE Net_LL SHALL expose an access point operation that starts the radio in access point mode with the
   SSID `pedal.guru` and no password, and that returns to the caller whether the access point is
   running.
3. WHILE the Provisioning_Access_Point is running, THE Net_LL SHALL accept HTTP requests from a device
   joined to that access point, and SHALL keep the Provisioning_Access_Point running while it serves
   them.
4. IF starting the Provisioning_Access_Point fails, THEN THE Pedal_Guru SHALL display on the
   Provisioning_Screen an indication that the device cannot be configured, SHALL keep that indication
   displayed for the remainder of the program run, and SHALL NOT restart the device.
5. IF the scan of criterion 1 fails, THEN THE Pedal_Guru SHALL hold an empty
   Network_List, SHALL continue to start the Provisioning_Access_Point, and SHALL continue into
   Provisioning_Mode.
6. WHERE the platform provides no access point implementation, THE Pedal_Guru SHALL report the access
   point step as successful without a radio being started, and SHALL continue to the
   Provisioning_Server, so that the Configuration_Page and the whole provisioning flow are exercised on
   the Simulator.
7. IF a station connect is requested while the Provisioning_Access_Point is running, THEN THE Net_LL
   SHALL report failure and SHALL leave the Provisioning_Access_Point running, because the device has a
   single radio that cannot be in access point mode and station mode at the same time.
8. WHEN Net_LL starts the Provisioning_Access_Point, THE Net_LL SHALL configure that access point to
   serve on the IP address `192.168.33.1` on every platform, rather than accept the platform's default
   access point address.

### Requirement 3: The display tells the rider how to reach the configuration page

**User Story:** As a rider, I want the device screen to tell me which network to join and which address
to open, so that I do not need a manual to configure it.

#### Acceptance Criteria

1. WHILE Provisioning_Mode is active, THE Provisioning_Screen SHALL display the SSID `pedal.guru` and
   the Configuration_Page URL `http://192.168.33.1/`, carrying no port number because the
   Provisioning_Server listens on TCP port 80, both simultaneously visible on the 240x240 round display.
2. THE `PageProvisioning` SHALL be a `BasePage` subclass implementing `PreDrawPageContents`,
   `DrawPageContents`, `PostDrawPageContents` and registering them through `Setup()`, and SHALL be
   created only by the Provisioning_Mode branch of Requirement 1, criterion 7, so that it never appears
   in the page cycle `CreatePages` builds.
3. WHILE Provisioning_Mode is active, THE Provisioning_Screen SHALL be redrawn on every pass of the UI
   render loop with no HID event of any type having occurred, since the navigation switches fire nothing
   yet (`TODO-D1`).
4. WHILE Provisioning_Mode is active, THE Pedal_Guru SHALL draw `PageProvisioning` as the only content
   of the display and SHALL draw no ride page, since `pages_` holds that one page and `GUINavigator` has
   nothing to cycle to.
5. WHEN the Endpoint_Callback that receives the submitted credentials reports that they have been
   written to the Credential_Store, THE Provisioning_Screen SHALL display that the device has been
   configured and SHALL keep that indication displayed until the device restarts.

### Requirement 4: Serve the configuration page

**User Story:** As a rider, I want to open a page hosted by the device itself, so that configuring it
needs no internet connection and no app install.

#### Acceptance Criteria

1. WHEN the Provisioning_Server receives a `GET /` request, THE Pedal_Guru Endpoint_Callback registered
   for that path SHALL produce the whole Configuration_Page content from the constant compiled into the
   firmware, with content type `text/html`.
2. THE Configuration_Page SHALL reference no HTML, CSS, JavaScript, font, image or other asset retrieved
   from any host other than the Provisioning_Server, so that it renders on a phone with no internet
   access.
3. THE Configuration_Page SHALL present the Network_List as a selection control, a single password
   field, the Rescan_Trigger button, and a control that submits the selected SSID together with that
   password to the Provisioning_Server.
4. IF the Provisioning_Server receives a request for a path no Endpoint_Callback is registered for, or a
   request whose HTTP method that path's Endpoint_Callback does not serve, THEN THE Provisioning_Server
   SHALL respond with a rejection indicating the request is not served, SHALL leave the Credential_Store
   unchanged, and SHALL remain serving subsequent requests — captive-portal probes issued by phones
   joining the Provisioning_Access_Point arrive on paths the device does not serve.
5. THE Net_LL SHALL expose an HTTP server operation that registers one Endpoint_Callback per served
   path, reports whether the server started, and can be stopped, and SHALL obtain every response body
   from the Endpoint_Callback the caller supplied, holding no knowledge of the Configuration_Page, of
   the Network_List or of the Credential_Store.
6. IF a request exceeds the Maximum_Request_Size of 2048 bytes counted over its request line, headers
   and body, or arrives while the Maximum_Connections of 1 connection is already in use, THEN THE
   Provisioning_Server SHALL reject that request without buffering the remainder of it and without
   serving it, and SHALL keep serving the connection already accepted.
7. IF Net_LL reports that the Provisioning_Server did not start, THEN THE Pedal_Guru SHALL display that
   the device cannot be configured and SHALL keep that failure visible on the display.
8. THE Provisioning_Server SHALL listen on TCP port 80 and SHALL serve exactly three routes, `GET /`
   returning the Configuration_Page, `GET /network/scan` returning a freshly scanned Network_List, and
   `POST /settings/save` receiving the submitted SSID and password.
9. THE Provisioning_Server SHALL serve every request without requiring any credential, token or code
   from the client (Decisions, D16).

### Requirement 5: Offer the networks in range for selection

**User Story:** As a rider, I want to pick my home network from a list, so that I do not have to type
its name and risk a typo.

#### Acceptance Criteria

1. WHEN the Provisioning_Server receives a `GET /` request, THE Pedal_Guru Endpoint_Callback SHALL serve the
   Network_List already held in memory from the scan of Requirement 2, criterion 1, and SHALL request no
   scan.
2. THE Pedal_Guru SHALL include, for each entry of the Network_List, the SSID, the authentication mode
   and the signal strength reported by Net_LL, SHALL escape every SSID character that carries syntactic
   meaning in the Configuration_Page's markup and in the data the page receives, and SHALL submit back
   the SSID the rider selected byte-identical to the one Net_LL reported.
3. WHERE Net_LL reports an authentication mode it cannot identify, THE Pedal_Guru SHALL keep that entry
   in the Network_List and SHALL present its authentication mode as unidentified.
4. IF the Network_List is empty, THEN THE Configuration_Page SHALL indicate that no network was found
   and SHALL allow the rider to type an SSID directly.
5. WHERE the Network_List holds more than one entry with the same SSID, THE Pedal_Guru SHALL present
   that SSID once, taking the authentication mode and signal strength from the entry reporting the
   strongest signal, since Net_LL reports one entry per BSSID.
6. WHERE an entry of the Network_List reports an empty SSID, THE Pedal_Guru SHALL omit that entry from
   the Network_List presented to the rider, and the rider SHALL still be able to type that network's
   name directly.
7. WHEN the rider presses the Rescan_Trigger, THE Configuration_Page SHALL issue a `GET /network/scan`
   request, and THE Pedal_Guru Endpoint_Callback for that path SHALL request a fresh scan from Net_LL
   with the Provisioning_Access_Point running, and on success SHALL replace the in-memory Network_List
   and return it to the page.
8. IF the scan requested through `GET /network/scan` fails, or the platform
   cannot scan while the Provisioning_Access_Point is running, THEN THE Endpoint_Callback SHALL report
   the failure to the Configuration_Page, THE Net_LL SHALL leave the Provisioning_Access_Point running
   and the Provisioning_Server accepting requests, and THE Configuration_Page SHALL keep the
   Network_List it is already presenting together with every value the rider has already typed.
9. THE Configuration_Page SHALL refresh neither itself nor the Network_List other than when the rider
   presses the Rescan_Trigger, so that typed input is never lost and a listed network never disappears
   on its own.

### Requirement 6: Receive and store the credentials

**User Story:** As a rider, I want the device to remember my network after I press Save, so that I
configure it once and never again.

#### Acceptance Criteria

1. WHEN the Provisioning_Server receives a `POST /settings/save` request, THE Pedal_Guru
   Endpoint_Callback for that path SHALL decode the submitted SSID and password from their form
   encoding into the exact byte sequence the rider typed, including non-ASCII bytes, and SHALL write
   them to the Credential_Store through FS_LL as the two lines `WIFI_SSID=<name>` and
   `WIFI_PWD=<password>` in a single `WriteFile` operation.
2. WHEN the Credential_Store has been written, THE Endpoint_Callback SHALL respond with a confirmation
   the Configuration_Page displays to the rider.
3. IF the submission carries an SSID that is empty, that consists only of space or tab characters, or
   that exceeds 32 bytes once decoded, THEN THE Endpoint_Callback SHALL respond with HTTP status 400 and
   an indication that the SSID was rejected, and SHALL leave the Credential_Store byte-for-byte
   unchanged.
4. IF the selected network reports an authentication mode other than open and the submitted password is
   shorter than 8 characters or longer than 63 characters, THEN THE Endpoint_Callback SHALL respond with
   HTTP status 400 and an indication that the password was rejected, and SHALL leave the
   Credential_Store byte-for-byte unchanged.
5. WHERE the selected network reports an open authentication mode, THE Endpoint_Callback SHALL accept an
   empty password and SHALL write `WIFI_PWD=` with an empty value.
6. IF writing the Credential_Store fails, at either opening it or writing its bytes, THEN THE
   Endpoint_Callback SHALL respond with HTTP status 500 and an indication that the credentials were not
   stored, and THE Provisioning_Screen SHALL report the failure while Provisioning_Mode stays active.
7. WHERE a Credential_Store already exists, THE Endpoint_Callback SHALL replace its whole content rather
   than append to it, leaving no byte of the previous content in the file, by writing the full new
   content from offset 0 and THEN truncating the file at the end of what was written through FS_LL's
   `TruncateFile` (Decisions, D27), since `OpenFile` opens at offset 0 without truncating and would
   otherwise leave the previous tail behind.
8. WHERE a Credential_Store already exists and holds `key=value` lines whose keys are neither `WIFI_SSID`
   nor `WIFI_PWD`, THE Endpoint_Callback SHALL include those lines, byte-identical, in the content it writes
   under criterion 7, so that a future `SettingsData` line survives a credential save (`TODO-C4`).
9. THE Credential_Store that Pedal_Guru writes SHALL be readable by the check in Requirement 1 and SHALL
   yield the same SSID bytes and the same password bytes that were submitted.
10. IF a Credential_Store on the card holds no `WIFI_SSID` line or no `WIFI_PWD` line, as a write
    interrupted by power loss would leave it, THEN THE Pedal_Guru SHALL treat it as not provisioned and SHALL enter
    Provisioning_Mode as Requirement 1 states.

The 32-byte SSID bound in criteria 1 and 3 is the length the 802.11 standard defines; the 8 to 63
character range in criterion 4 is the WPA2-Personal passphrase range.

### Requirement 7: Return to normal operation

**User Story:** As a rider, I want the device to start behaving like a cycle computer as soon as it is
configured, so that I can use it right after setting it up.

#### Acceptance Criteria

1. WHEN the Credential_Store has been written successfully, THE Pedal_Guru SHALL leave
   Provisioning_Mode by restarting the device through HAL_LL, so that Normal_Mode is reached by a fresh
   program run with the radio and the card in their initial state.
2. WHERE the platform is the Simulator, THE Pedal_Guru SHALL close the application instead of
   restarting the device.
3. WHEN the Provisioning_Screen has displayed that the device has been configured, THE Pedal_Guru SHALL
   keep that message on the display for at least the Confirmation_Hold of 5 seconds before the restart,
   giving the rider's phone time to render the confirmation, and SHALL require no navigation-switch
   input to move on, since no user input is wired up yet (`TODO-D1`).
4. WHEN Pedal_Guru is about to restart, THE Pedal_Guru SHALL complete this order first: the confirmation
   response of Requirement 6 is fully handed to Net_LL for delivery, then the Provisioning_Server is
   stopped, then the Provisioning_Access_Point is stopped and the radio deinitialized, then the SD card
   is unmounted through FS_LL.
5. WHEN Pedal_Guru restarts, THE Pedal_Guru SHALL make the unmount call at the point in `app_entry` where
   it is made today, and SHALL leave the sensor thread neither stopped nor waited for (Decisions, D26).

### Requirement 8: Make the stored credentials usable by a station connect

**User Story:** As the developer, I want the connect operation and the stored credentials both
available, so that the map sync page can join the configured network later without this feature
guessing how it should behave.

#### Acceptance Criteria

1. THE Net_LL SHALL expose a station connect operation that takes an SSID and a password, powers the
   radio on in station mode, and reports either connection established or connection not established.
2. THE Pedal_Guru SHALL expose a read of the Credential_Store through FS_LL that yields the stored SSID
   and password, or reports that no usable Credential_Store is present.
3. THE Pedal_Guru SHALL call the station connect operation nowhere in this feature, since driving the
   connection belongs to `PageMapSync` (Decisions, D14).
4. WHILE Normal_Mode is active with no network connection, THE Pedal_Guru SHALL keep every ride page
   usable, because the device is designed to be offline on the road.

### Requirement 9: Threading and platform constraints the feature inherits

**User Story:** As the developer, I want this feature to obey the architecture already in place, so
that it does not break the two-thread model, the platform seam or the build on any of the three
targets.

#### Acceptance Criteria

1. THE Pedal_Guru SHALL reach the radio only through Net_LL, the SD card only through FS_LL and every
   other hardware resource only through HAL_LL, adding no direct SDK call, socket call or radio call to
   this repository.
2. THE Pedal_Guru SHALL run every operation this feature adds on one of the two threads that already
   exist and SHALL create no additional thread, so that the thread count stays at exactly two, matching
   the RP2040's two cores.
3. THE Pedal_Guru SHALL run the Provisioning_Server, every Endpoint_Callback and every Credential_Store
   read and write on the same one of those two threads, so that no Credential_Store access is concurrent
   with another.
4. THE Net_LL SHALL make every operation added by this feature synchronous: each operation SHALL return
   its success or failure result to the calling thread before returning control, SHALL invoke every
   Endpoint_Callback on the caller's thread, and SHALL create no thread of its own.
5. THE Pedal_Guru SHALL mount the SD card through FS_LL on the thread `app_entry` runs on, before
   either thread is created.
6. THE Pedal_Guru SHALL unmount the SD card through FS_LL on the same thread that mounted it, the
   thread `app_entry` runs on, and SHALL make neither that unmount call nor the mount call of criterion
   5 from the sensor thread, as FatFs requires of `f_mount` (Decisions, D26).
7. THE Pedal_Guru SHALL perform every Credential_Store read and write while the SD card is mounted by
   that single mount.
8. THE Pedal_Guru SHALL configure, compile and link on the Simulator, RP2040 and ESP32 targets, with no
   compiler or linker error on any of the three, independently of which platform receives a working
   implementation first (Decisions, D6).
9. WHERE a platform has no implementation of an operation this feature needs, THE Net_LL SHALL provide a
   stub that returns a failure result to the caller, leaves the Credential_Store unchanged and does not
   abort the program, so that the firmware stays linkable and the failure is diagnosable — the same
   shape `HttpDownloadFile` already has on the two hardware platforms. THE Net_LL access point operation
   on the Simulator SHALL be such a stub, reporting the successful no-op of Requirement 2, criterion 6
   rather than a failure, and THE Net_LL station connect on the Simulator SHALL be such a stub as well.
10. THE Pedal_Guru SHALL include every Net_LL, FS_LL and HAL_LL header inside an `extern "C"` block, and
    SHALL add no C++ construct to those submodules, which are C.
11. THE Pedal_Guru SHALL expose each Endpoint_Callback as a C function with C linkage so that Net_LL can
    hold it as a function pointer, and SHALL reach the C++ objects the callback needs without passing a
    C++ type across that boundary.
12. THE Pedal_Guru SHALL select platform-specific behaviour through the CMake source list and include
    path only, and SHALL NOT test any platform preprocessor macro, since no `SIMULATOR`, `RP2040` or
    `ESP32` macro exists.
13. THE Pedal_Guru SHALL express every build change this feature needs in `pedal.guru.cmake` as variable
    assignments and messages only, using no directory-scoped or target-scoped CMake command, because
    ESP-IDF evaluates that file in script mode.

## Decisions

Settled by the dev, recorded so the design can cite them.

**D-AP — the Simulator has no access point.** A real access point on a desktop is complex and pointless.
On the Simulator only the HTTP server exists, as a POSIX socket server, alongside the existing mocked /
`netsh` scan. The access point step there is a successful no-op, so the Configuration_Page and the whole
provisioning flow are still exercised.

**D-SCAN — the scan happens before the access point is started.** ESP-IDF documents a combined AP+STA
mode with scanning supported, but the scan needs the station interface started and it interrupts the
joined phone for the duration of the sweep; on the RP2040's CYW43 there are forum reports that the scan
callback is not delivered while another mode is active. Neither was verified in this project's code, so
the flow does not depend on it. The scan therefore runs while entering Provisioning_Mode, before the
access point, and the resulting Network_List is held in memory and served from there. The Rescan_Trigger
is an explicit button that attempts a fresh scan with the access point running and, on failure, changes
nothing the rider can see except reporting the failure.

**D1 — Credential_Store location.** A file named `settings` at the root of the SD card.

**D2 — Credential_Store format.** `key=value`, one pair per line.

**D3 — plaintext.** Credentials are stored in plaintext for this first version. The exposure is
acknowledged: a removable card carrying a home WiFi password in the clear. Obfuscation without a
hardware secret is not security, and neither MCU here has a key store this project uses.

**D4 — SSID and password only.** Not username and password. Only what is needed to join a home WiFi:
one SSID of up to 32 bytes and one password field, a WPA2-Personal passphrase of 8 to 63 characters, or
empty for an open network. Every username and WPA2-Enterprise consideration is dropped.

**D5 — access point identity and URL.** Fixed SSID `pedal.guru`, open access point with no password, and
the Configuration_Page URL is the fixed IP address of D18. No hostname, therefore no mDNS and no
captive-portal DNS responder.

**D6 — platform order.** ESP32 first, RP2040 immediately after. The Simulator gets the HTTP server and
the flow.

**D7 — the return to Normal_Mode is a device restart.** On the Simulator it closes the application.

**D8 — the Provisioning_Screen is `PageProvisioning`**, a new `BasePage` subclass that takes no part in
the page cycle. The wiring point is the `CreatePages(drawer)` call site in `TaskManager::Execute`
(`src/TaskManager.cpp` line 50): if the WiFi credentials are not configured, `push_back` only
`PageProvisioning`; if they are, call `CreatePages` as it does today. `HIDHandler`, `GUINavigator` and
`drawer.Execute()` are created and run identically in both modes.

**D9 — no connection retry and no re-provisioning** in this feature. Deferred.

**D10 — Net_LL runs the HTTP server, Pedal_Guru owns the content.** Net_LL implements the server over
sockets and must not know what is hosted: it exposes registration of a path with an Endpoint_Callback
that Pedal_Guru supplies, and that callback produces the response. Consequently the Credential_Store
write happens in Pedal_Guru's callback, never in Net_LL. This creates a C/C++ boundary: the callback is
a C function pointer crossing into C++ code.

**D11 — the Configuration_Page is compiled into the firmware as a constant**, not read from the SD card,
which would force the card to be prepared in advance.

**D12 — the SD card is assumed always present.** The card-unavailable behaviour is deferred.

**D13 — Rescan_Trigger is an explicit button**, settled by D-SCAN.

**D14 — the station connect is not driven by this feature.** Connecting to the configured network,
Connect_Timeout and the "network not available" messaging become `PageMapSync`'s responsibility and are
future work. This feature only exposes the connect operation and the credential read.

**D15 — no Transition_Timeout.** Since D7 is a restart, a timeout in software after the restart call has
nothing left to run on: the restart either happens or the device is wedged. Confirmation_Hold is 5
seconds.

**D16 — the Provisioning_Server is unauthenticated, and that is accepted for this version.** The
Provisioning_Access_Point is open and no endpoint asks for a credential, a token or a code, so while the
device is unprovisioned anyone within radio range can join `pedal.guru` and POST to `/settings/save`. The
exposure window is only the unprovisioned state, and the worst outcome is a wrong network being stored,
which the rider fixes by provisioning again. Two options were considered and are recorded for a later
version without being adopted now: a password on the access point, or a short code shown on the LCD that
the Configuration_Page has to submit.

**D17 — there is no Scan_Timeout; the scan is used exactly as Net_LL exposes it.** An earlier version of
this document put a 5 second bound on the scan. It is dropped, and Net_LL's `WiFiScan` is called as it
exists today, with no timeout parameter and no new Net_LL function. Pedal_Guru lives with the platform's
blocking scan until it returns.

The reason is the thread budget. Bounding a blocking call from the outside needs a thread to do the
waiting, and there is no third thread available on the RP2040 (Requirement 9, criterion 2), so a timeout
cannot be imposed without breaking the two-thread constraint. Bounding it from the inside would mean
changing `WiFiScan`'s signature or adding a second function beside it, which is Net_LL work for a bound
the hardware already respects in practice: an active scan of the 13 channels of the 2.4 GHz band at
ESP-IDF's default per-channel dwell of roughly 120 ms comes to about 1.5 to 2 seconds, and the CYW43 is in
the same order. The focus stays on the MCU work.

The scan-failure paths are unchanged: a failed scan leaves an empty Network_List, and the rider can still
type an SSID by hand (Requirement 5, criterion 4).

**D18 — endpoints, port and address.** The Provisioning_Server listens on TCP port 80, so the URL on the
Provisioning_Screen carries no port and reads `http://192.168.33.1/`. Three routes: `GET /` for the
Configuration_Page, `GET /network/scan` for a fresh scan returning the Network_List, and
`POST /settings/save` for the submitted SSID and password.

The access point address is fixed at `192.168.33.1` on every platform, which means Net_LL must **configure**
the access point's address rather than accept the platform default: ESP-IDF's softAP defaults to
192.168.4.1 and the CYW43's default differs, so without that configuration the printed URL would be wrong
on at least one target. One fixed address on both platforms is what makes one printed URL correct
everywhere, and it is compiled in, so the URL is known before the access point starts.

The address satisfies three constraints at once. Its subnet, `192.168.33.0/24`, is outside the ones a
home router typically hands out — `192.168.0.0/24` and `192.168.1.0/24` — so joining the access point
cannot collide with an address the rider's phone already holds in cache or history. It is not ESP-IDF's
softAP default of `192.168.4.1`, so a developer's other ESP32 project cannot be confused with this one.
And the host part is `.1`, the conventional gateway position, which is what a phone and a DHCP client
expect of an access point.

Because the gateway sits on `.1` of its own subnet, the DHCP pool takes its default shape on both
platforms and needs no special handling.

**D19 — request limits.** Maximum_Request_Size is 2048 bytes over the request line, the headers and the
body together, and Maximum_Connections is 1. The largest request this flow produces is the save POST —
an SSID of up to 32 bytes plus a password of up to 63, form-encoded, plus the headers a phone sends —
which lands in the 500 byte to 1 KB range, so 2048 leaves room without being generous. The cap is also
what stops a client from making the device allocate without bound, which is why an oversized request is
rejected rather than buffered.

**D20 — Credential_Store keys.** Two lines: `WIFI_SSID=<name>` and `WIFI_PWD=<password>`. The `WIFI_`
prefix on both is symmetric and groups the WiFi keys once the file absorbs the rest of `SettingsData`
later (`TODO-C4`). The file is shaped for that absorption, so the reader tolerates keys it does not
recognise and requires neither WiFi line to be first or adjacent, and the writer carries every
unrecognised key back into the content it writes.

**D21 — file and class placement.** `src/GUI/Page/PageProvisioning.cpp` and its `.hpp` hold the page
drawn on the LCD, alongside the existing `BasePage` subclasses. `src/API/SelfHost/` holds the pedal.guru
side of the self-hosted configuration: `ConfigurationPage.cpp/.hpp` generating the HTML, `Server.cpp/.hpp`
wiring the endpoints to Net_LL, and whatever else that folder turns out to need. That folder sits beside
the existing `src/API/OpenStreetMapAPI.{cpp,hpp}` and follows the PascalCase file convention.

**D22 — nothing in `TaskManager`'s workflow changes.** The **only** change to `TaskManager` is that when
no `WIFI_*` settings are available, the single page pushed into `pages_` is `PageProvisioning`. Everything
else stays exactly as it is, in both modes: `ReadSettings`, `CreateDevices`, `ConnectToDevices`, the sensor
thread, `HIDHandler`, `GUINavigator` and `drawer.Execute()`. That uniformity is deliberate groundwork for
improvements the dev has in mind, so Provisioning_Mode is not a reduced program — it is the same program
with a different page.

`TaskManager::Execute()` therefore keeps its current `void` signature. The restart request cannot travel
back through a return value, so `PageProvisioning` records it in `DataManager` (D23) and `app_entry` reads
it after `Execute()` has returned and after `UnMountSdCard()`, then calls the HAL_LL restart.

**D23 — `DataManager` owns the Credential_Store; there is no separate store class.** `DataManager` is
already the singleton that centralizes state between the two threads behind a `Mutex`, and that is where
the `settings` file belongs, so both threads reach it safely. The reader, the writer and the restart
request are methods on `DataManager`, each taking that same mutex; the plain data types the file parses
into live in `src/Model/` beside `SettingsData.hpp`. No new folder and no new class.

What the mutex does not cover: `f_mount` is never thread-safe in FatFs, so the mount and the unmount stay
in `app_entry`, outside any thread, exactly where they are today. `DataManager` guards file access, not
mounting. This also answers the question fs.ll's own `AGENTS.md` §12 asks of this repository — whether
every card access funnels through the parent application's `DataManager` — with a yes for this file, which
is what makes the serialization point exist above that library.

**D24 — `GET /network/scan` returns JSON.** The endpoint responds `application/json` with the network
list, and the Configuration_Page's JavaScript parses it and rebuilds the selection control from it. `GET /`
keeps rendering the same list into the page's markup server-side, so both renderers exist and neither
replaced the other. The cost is accepted: a JSON string escaper beside the markup escaper.

**D25 — the poll interval and the restart name.** The interval at which the Provisioning_Server is
serviced from the render loop is **100 milliseconds**: nothing on the Provisioning_Screen animates, so
100 ms of block per render pass costs nothing visible and leaves ample margin. The HAL_LL operation is
named **`DeviceRestart`**, because on both MCUs it is a genuine device reset — the chip comes back through
its reset vector with RAM re-initialised, not merely the application. On the Simulator the same operation
ends the process and nothing brings it back, which D7 already accepted; the name describes the hardware
behaviour and the Simulator is the deliberate exception.

**D26 — the sensor thread is never stopped and never waited for.** `running_ = false` living in
`~TaskManager` is deliberate: the intent is that the sensors keep collecting the current activity's data
and recording it to the card even while the application is doing something other than showing that screen.
Stopping the sensor thread would imply there is no activity in progress. Activity recording does not exist
in the project yet, so the question is not live, and this feature adds no wait and no join.

The unmount is safe today because the sensor thread reaches only `DataManager::Push` and touches no file.
That changes once `DataManager` owns the `settings` file for a writer on that thread, and once activity
recording exists — at which point the ordering between the unmount and the sensor loop becomes a real
question to revisit.

**D27 — FS_LL gains `TruncateFile`, and the Credential_Store write is write-then-cut.** fs.ll gains
`bool TruncateFile(FIL *file)` in `src/lib/FileSystem.h` and `FileSystem.c`, wrapping FatFs's
`f_truncate`, which cuts the file at the current file pointer. `WriteFile` is neither changed nor renamed.

The write is two opens, not one, and the reason is that fs.ll exposes no seek: a single open cannot read
to the end of the file and then write again from offset 0. So the read happens first — open, read the
whole existing content, close — which is where the unrecognised keys come from; then the full new content
is serialised in memory, the file is opened again, one `WriteFile` writes that content from offset 0,
`TruncateFile` cuts the file at the pointer, and `CloseFile` closes it. `OpenFile` leaves the pointer at
offset 0 and truncates nothing, and after the write the pointer sits at the end of what was just written,
which is exactly where the cut belongs — so the truncation comes after the write, never before it.

The other settings survive because the truncation happens *after* the full new content, including every
unrecognised line byte-identically, has already been written. The region being cut lies past the end of
that content and is leftover from the previous, longer file, so it can never be a live setting.

Two alternatives were considered and rejected. **Seek**: adding `f_lseek` is easy and FatFs has it, but a
seek only positions the pointer — it neither shortens a file nor shifts bytes — so it does not address
this problem. **Line-level editing**: no filesystem exposes a line primitive, a file is a byte array, and
replacing a line with one of a different length requires shifting everything after it, which is precisely
why the whole content is rebuilt in memory and rewritten. The in-memory rebuild *is* the replace.

Nothing remains open in these requirements, and no design-level choice is open in `design.md` either.

Deferred, recorded so they are not lost: connection retry and re-provisioning (D9), SD-card-absent
handling (D12), the station connect being driven by `PageMapSync` including Connect_Timeout and the
"network not available" messaging (D14), and authenticating the provisioning flow, either with a password
on the access point or with a short code shown on the LCD (D16).
