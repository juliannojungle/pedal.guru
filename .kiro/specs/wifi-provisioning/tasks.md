# Implementation Plan: wifi-provisioning

## Overview

The work spans four git submodule repositories and is grouped by repository below, so each group is one
repository's commit. The order never puts a consumer before its dependency: fs.ll and hal.ll first (one
function each), then net.ll (radio operations, then the HTTP server), then pedal.guru, which consumes all
three.

Platform order inside net.ll follows D6: ESP32 first, RP2040 immediately after as a stub. The Simulator
implementation of the HTTP server is written before ESP32 because the ESP32 one is the same BSD socket
logic over lwIP and ports from it nearly unchanged.

The RP2040 HTTP server and access point are stubs in this feature. Enabling lwIP, authoring `lwipopts.h`
and adding a DHCP server on that platform are explicitly out of scope and no task below asks for them.

Verification is a clean build on the three targets plus observed behaviour, per design.md's
`## Verification` section. There are no test tasks: `AGENTS.md` ground rule 8.

## Tasks

- [x] 1. fs.ll: truncate a file at the current pointer
  - [x] 1.1 Add `TruncateFile` to fs.ll
    - Declare `bool TruncateFile(FIL *file);` in
      `src/Dependency/fs.ll/src/lib/FileSystem.h`, beside `WriteFile`, and implement it in
      `src/Dependency/fs.ll/src/lib/FileSystem.c` wrapping FatFs's `f_truncate`, returning whether the
      `FRESULT` was `FR_OK`
    - Leave `WriteFile` unchanged and unrenamed, and do not add a seek
    - No build change: both files are already in fs.ll's source list
    - _Requirements: 6.7_

- [x] 2. hal.ll: restart the device
  - [x] 2.1 Declare `DeviceRestart` in the three platform HAL headers
    - Add `void DeviceRestart(void);` under a `system` group after the stdio group in
      `src/Dependency/hal.ll/src/lib/Platform/Simulator/HAL.h`,
      `src/Dependency/hal.ll/src/lib/Platform/RP2040/HAL.h` and
      `src/Dependency/hal.ll/src/lib/Platform/ESP32/HAL.h`
    - _Requirements: 7.1, 7.2_

  - [x] 2.2 Implement `DeviceRestart` on the three platforms
    - `src/Dependency/hal.ll/src/lib/Platform/ESP32/HAL.c`: `esp_restart()`, already covered by
      `esp_system` in `PLATFORM_REQUIRES`
    - `src/Dependency/hal.ll/src/lib/Platform/RP2040/HAL.c`: `watchdog_reboot(0, 0, 0)`
    - `src/Dependency/hal.ll/src/lib/Platform/Simulator/HAL.c`: `exit(EXIT_SUCCESS)`
    - _Requirements: 7.1, 7.2_

  - [x] 2.3 Add `hardware_watchdog` to hal.ll's RP2040 library list
    - Append `hardware_watchdog` to the `PLATFORM_LIBRARIES` list in the `RP2040` branch of
      `src/Dependency/hal.ll/hal.ll.cmake`
    - _Requirements: 9.8_

- [x] 3. net.ll: access point and station connect on the radio API
  - [x] 3.1 Declare the access point and station connect surface
    - In `src/Dependency/net.ll/src/lib/WiFi.h`, add the `#ifndef`-guarded
      `WIFI_ACCESS_POINT_ADDRESS` macro defaulting to `"192.168.33.1"`, then
      `bool WiFiAccessPointStart(const char *ssid, const char *password);`,
      `bool WiFiAccessPointStop(void);`, `bool WiFiAccessPointIsRunning(void);` and
      `bool WiFiStationConnect(const char *ssid, const char *password);`
    - A null or empty `password` means an open access point
    - _Requirements: 2.2, 2.8, 8.1_

  - [x] 3.2 Implement the ESP32 access point and station connect
    - In `src/Dependency/net.ll/src/lib/Platform/ESP32/WiFi.c`, start softAP mode with the SSID the
      caller passed and open authentication when the password is null or empty, and configure the
      interface address to `WIFI_ACCESS_POINT_ADDRESS` with `esp_netif_set_ip_info`, stopping and
      restarting the DHCP server around that call so the gateway handed to the phone is that address
      rather than ESP-IDF's 192.168.4.1 default
    - Require a successful `WiFiInitialize` first, the way `WiFiScan` already does
    - Track the running state for `WiFiAccessPointIsRunning`, and implement `WiFiAccessPointStop`
    - Implement `WiFiStationConnect` in station mode, returning only once the connection is established
      or has failed, and returning false immediately while the access point is running
    - _Requirements: 2.2, 2.3, 2.7, 2.8, 8.1_

  - [x] 3.3 Implement the RP2040 access point and station connect stubs
    - In `src/Dependency/net.ll/src/lib/Platform/RP2040/WiFi.c`, `WiFiAccessPointStart` and
      `WiFiStationConnect` return false and `WiFiAccessPointIsRunning` returns false, each printing a
      diagnostic in the shape `HttpDownloadFile` already uses there, so the firmware stays linkable and
      the failure is diagnosable
    - `WiFiAccessPointStop` returns true, having nothing to stop
    - _Requirements: 9.7, 9.8, 9.9_

  - [x] 3.4 Implement the Simulator access point as a successful no-op and connect as a failing stub
    - In `src/Dependency/net.ll/src/lib/Platform/Simulator/WiFi.c`, `WiFiAccessPointStart` returns true
      without starting any radio, `WiFiAccessPointIsRunning` reports that no-op state,
      `WiFiAccessPointStop` returns true, and `WiFiStationConnect` returns false
    - _Requirements: 2.6, 9.9_

- [x] 4. net.ll: the HTTP server
  - [x] 4.1 Declare the HTTP server API and its types
    - Create `src/Dependency/net.ll/src/lib/HttpServer.h` with `HttpMethod`, `HttpRequest`,
      `HttpResponse` and `HttpEndpointCallback` exactly as design.md's Data Models section defines them,
      the `HTTP_SERVER_MAX_REQUEST_SIZE` of 2048 and `HTTP_SERVER_MAX_ROUTES` of 8, and the five
      functions `HttpServerStart`, `HttpServerRegisterEndpoint`, `HttpServerPoll` and `HttpServerStop`
    - State the buffer ownership contract in the header: the three `HttpRequest` pointers are valid only
      for the duration of the callback, and the `HttpResponse` body the callback supplies must stay valid
      until `HttpServerPoll` returns
    - _Requirements: 4.5, 4.6, 4.8, 9.4_

  - [x] 4.2 Implement the Simulator HTTP server over POSIX sockets
    - Create `src/Dependency/net.ll/src/lib/Platform/Simulator/HttpServer.c`
    - `HttpServerStart` creates the listening socket on the given port with a backlog of 1;
      `HttpServerRegisterEndpoint` stores method, path, callback and `void *context` in a fixed array of
      `HTTP_SERVER_MAX_ROUTES`
    - `HttpServerPoll` waits up to `timeoutMilliseconds` with `select`, and on a connection reads the
      request into the single static `HTTP_SERVER_MAX_REQUEST_SIZE` buffer, NUL-terminates the path, the
      query and the body in place, defaults the response to status 200 and `text/html`, calls the
      matching callback on the caller's thread, writes the response and closes the connection before
      returning whether a request was served
    - Copy nothing in either direction, create no thread
    - Respond 404 for an unmatched path, 405 for a matched path with a method that route does not serve,
      and 413 for a request exceeding the buffer bound, closing without buffering its remainder, and
      keep serving subsequent requests in every case
    - `HttpServerStop` closes the listening socket and clears the routes
    - _Requirements: 4.4, 4.5, 4.6, 4.8, 4.9, 9.4_

  - [x] 4.3 Implement the ESP32 HTTP server over lwIP BSD sockets
    - Create `src/Dependency/net.ll/src/lib/Platform/ESP32/HttpServer.c` carrying the same logic and the
      same contract as the Simulator implementation, over lwIP's BSD socket API
    - Do not use `esp_http_server`: that component spawns its own task, and net.ll creates no thread
    - _Requirements: 4.4, 4.5, 4.6, 4.8, 4.9, 9.4_

  - [x] 4.4 Implement the RP2040 HTTP server stub
    - Create `src/Dependency/net.ll/src/lib/Platform/RP2040/HttpServer.c` where `HttpServerStart`
      returns false with a diagnostic, `HttpServerRegisterEndpoint` and `HttpServerPoll` return false and
      `HttpServerStop` does nothing, so the firmware links and an unprovisioned device reports that it
      cannot be configured
    - _Requirements: 9.7, 9.8, 9.9_

  - [x] 4.5 Add the server to net.ll's build contract
    - In `src/Dependency/net.ll/net.ll.cmake`, append `"${NET_LL_PLATFORM_DIR}/HttpServer.c"` to
      `SOURCES`, and add `lwip` to `PLATFORM_REQUIRES` in the ESP32 branch for the socket headers
    - Leave the RP2040 branch untouched: `CYW43_LWIP=0` and `pico_cyw43_arch_poll` stay
    - Keep the file variables-and-messages-only
    - _Requirements: 9.8, 9.13_

- [x] 5. Checkpoint - the four dependency repositories build
  - Build all three targets so the fs.ll, hal.ll and net.ll additions are known to configure, compile and
    link before pedal.guru starts consuming them. Ask the dev if questions arise.
  - _Requirements: 9.8_

- [x] 6. pedal.guru: the types and the two small primitives the flow needs
  - [x] 6.1 Add the settings and credential data types
    - Create `src/Model/SettingsEntry.hpp` with `SettingsEntry` and `SettingsFileData`, and
      `src/Model/CredentialData.hpp` with `CredentialData`, both in `namespace PedalGuru` and shaped as
      design.md's Data Models section defines them
    - `src/Model` is already on the include path, so no build change
    - _Requirements: 1.10, 6.8, 8.2_

  - [x] 6.2 Add the provisioning state type
    - Create `src/Model/ProvisioningState.hpp` with `enum class ProvisioningState { SERVING, CONFIGURED,
      STORE_FAILED, UNAVAILABLE };` in `namespace PedalGuru`
    - _Requirements: 2.4, 3.5, 4.7, 6.6_

  - [x] 6.3 Add `Time::TicksMs()` to the three platform folders
    - Add `static unsigned int TicksMs();` to `Time.hpp` and its definition to `Time.cpp` in
      `src/Platform/Simulator`, `src/Platform/RP2040` and `src/Platform/ESP32`, forwarding to hal.ll's
      `TicksMs()`, with the three copies identical the way `Time::Delay` already is
    - _Requirements: 7.3, 9.1_

  - [x] 6.4 Let a page end the render loop
    - Add `void RequestClose();` and a `bool closeRequested_` to `src/GUI/GUIDrawer.hpp` and
      `src/GUI/GUIDrawer.cpp`, and change the loop condition in `GUIDrawer::Execute` to
      `while (!window.ShouldClose() && !closeRequested_)`
    - Leave the PostDraw callback and `window.Close()` running exactly as they do when the user closes
      the window
    - _Requirements: 7.1, 7.4_

- [x] 7. pedal.guru: `DataManager` owns the Credential_Store
  - [x] 7.1 Add the settings file reader to `DataManager`
    - In `src/DataManager.hpp` and `src/DataManager.cpp`, add the private
      `bool ReadSettingsFile(SettingsFileData& fileData)` and the public
      `bool ReadCredentials(CredentialData& credentials)`
    - `ReadCredentials` takes `mutex_` on entry and releases it on exit, like `Push` and `Pop`;
      `ReadSettingsFile` takes no lock and is only called from inside it. No public method calls another
      public method, because `Mutex` is not recursive on any platform
    - `ReadCredentials` guards with `PathOrFileExists` and returns without opening anything when the file
      is absent, because fs.ll's `OpenFile` creates a missing file and the startup read must create,
      truncate and write nothing
    - When the file exists and `OpenFile` still fails, re-mount through `MountSdCard()` and
      `SelectActiveDrive()` before returning, since fs.ll's `OpenFile` unmounts the volume on failure and
      this read is the only one that runs before the sensor thread exists
    - `ReadSettingsFile` accumulates the whole file through repeated `ReadFile` calls with a 512-byte
      chunk up to a size cap, closes the file, splits on `'\n'` discarding a trailing `'\r'`, and splits
      each line at its first `'='`; a line with no `'='` becomes an entry with an empty key and the whole
      line as its value, and entry order is the file order
    - `ReadCredentials` finds `WIFI_SSID` and `WIFI_PWD` wherever they appear, in any order and separated
      by anything, and reports provisioned only when the SSID is non-empty and at most 32 bytes and a
      `WIFI_PWD` line is present
    - _Requirements: 1.2, 1.4, 1.5, 1.10, 6.10, 8.2, 9.3, 9.7_

  - [x] 7.2 Add the settings file writer to `DataManager`
    - Add the private `bool WriteSettingsFile(const SettingsFileData& fileData)` and the public
      `bool WriteCredentials(const std::string& ssid, const std::string& password)`, with
      `WriteCredentials` taking `mutex_` on entry and releasing it on exit
    - `WriteCredentials` calls `ReadSettingsFile` to obtain the existing entries, replaces the
      `WIFI_SSID` and `WIFI_PWD` entries or appends them when absent, and hands every entry in read order
      to `WriteSettingsFile`, so unrecognised `key=value` lines are carried through byte-identically
    - `WriteSettingsFile` serialises all entries into one `std::string`, then `OpenFile`, one `WriteFile`
      of that whole content, then fs.ll's `TruncateFile`, then `CloseFile` — in that order, because
      `OpenFile` positions at offset 0 without truncating and the pointer after the single write is
      exactly where the cut belongs
    - Two opens rather than one, because fs.ll exposes no seek: the read closes the file before the write
      reopens it at offset 0
    - Report failure at either the open or the write
    - _Requirements: 6.1, 6.5, 6.6, 6.7, 6.8, 6.9, 9.3, 9.7_

  - [x] 7.3 Add the restart request to `DataManager`
    - Add the `bool restartRequested_` member and the public `void SetRestartRequested()` and
      `bool GetRestartRequested()`, each taking `mutex_` on entry and releasing it on exit
    - _Requirements: 7.1_

- [x] 8. pedal.guru: the self-hosted configuration page
  - [x] 8.1 Decode form-encoded bodies
    - Create `src/API/SelfHost/FormBody.{cpp,hpp}` in `namespace PedalGuru`, parsing
      `application/x-www-form-urlencoded`: split on `&`, split each pair at its first `=`, percent-decode
      with `+` mapped to a space
    - Work on bytes, validate no UTF-8, and leave an invalid `%` escape as the bytes it is rather than
      throwing, so the decoded value is exactly what the browser sent
    - _Requirements: 6.1_

  - [x] 8.2 Build the configuration page content and both renderers
    - Create `src/API/SelfHost/ConfigurationPage.{cpp,hpp}` in `namespace PedalGuru`
    - Hold the page as a file-scope raw string literal `PAGE_TEMPLATE` of type `const char[]` with the
      single `%NETWORKS%` substitution point, the CSS and the JavaScript inline, and no HTML, CSS, JS,
      font or image referenced from any other host. The form posts to `/settings/save` with
      `accept-charset="UTF-8"` and carries the network select, a free-text SSID field, one password
      field, the submit control and the rescan button
    - Write one shared helper that de-duplicates the network list by SSID bytes keeping the entry with
      the highest RSSI, omits entries with an empty SSID, and keeps an unidentifiable authentication mode
      labelled as unidentified, so the two renderers can never disagree about which networks exist
    - `RenderNetworkOptions` returns the `<option>` block replacing `%NETWORKS%`, yielding a single
      disabled "no network found" option for an empty list, escaping `&`, `<`, `>`, `"` and `'` to their
      entity forms in both the option text and the `value` attribute, and altering nothing else
    - Add the JSON renderer producing an array of objects with the three keys `ssid`, `auth` and `rssi`,
      and a separate JSON string escaper: `"` to `\"`, `\` to `\\`, the named control escapes `\b` `\f`
      `\n` `\r` `\t`, every other byte below 0x20 as `\u00XX`, `/` left alone, and bytes at 0x80 and
      above passed through unless the SSID is not valid UTF-8, in which case each such byte is
      `\u00XX`-escaped so the response stays parseable
    - The JavaScript rebuilds the select from that JSON and refreshes neither the page nor the list other
      than on the rescan button, so typed input is never lost
    - _Requirements: 4.1, 4.2, 4.3, 5.2, 5.3, 5.4, 5.5, 5.6, 5.9_

  - [x] 8.3 Route the three endpoints and cross the C boundary
    - Create `src/API/SelfHost/Server.{cpp,hpp}` in `namespace PedalGuru`, with `Start`, `Stop`, `Poll`,
      `CredentialsStored`, `CredentialStoreFailed`, the three private handlers `OnRoot`, `OnScan`,
      `OnSave`, and the `page_`, `networks_`, `responseBody_`, `credentialsStored_` and
      `credentialStoreFailed_` members design.md declares
    - `Start` starts the server on port 80 and registers `GET /`, `GET /network/scan` and
      `POST /settings/save` through `HttpServerRegisterEndpoint`, passing `this` as the context
    - Add three file-scope `extern "C"` trampolines in `Server.cpp` that cast the `void* context` back to
      `Server*` and call the matching handler, so no C++ type crosses the boundary
    - Every generated body is built into `responseBody_` and pointed at with its `size()`, which outlives
      the `HttpServerPoll` call; `GET /` with an empty substitution can point straight at the page
      constant
    - Include net.ll's `HttpServer.h` and `WiFi.h` inside `extern "C"` blocks
    - _Requirements: 4.1, 4.5, 4.8, 5.1, 9.1, 9.11_

  - [x] 8.4 Serve the root, the rescan and the save
    - `OnRoot` serves the page with `%NETWORKS%` substituted from the `networks_` held since bring-up and
      requests no scan, with content type `text/html`
    - `OnScan` asks net.ll for a fresh scan with the access point running, replaces `networks_` and
      returns the JSON list with content type `application/json` on success; on failure it responds with
      a non-200 status and a short plain-text message, leaving the access point and the server running
    - `OnSave` decodes the body through `FormBody`, then validates in order: an SSID that is empty,
      whitespace-only or over 32 bytes decoded gives 400; for a network whose reported authentication
      mode is other than open, a password shorter than 8 or longer than 63 characters gives 400; an open
      network accepts an empty password and stores `WIFI_PWD=` empty. An SSID typed by hand that is not
      in `networks_` has no reported mode and is treated as non-open
    - Only after validation does `OnSave` call `DataManager::WriteCredentials`; on success it sets
      `credentialsStored_` and responds with the confirmation the page displays, on failure it sets
      `credentialStoreFailed_` and responds 500. A rejection leaves the Credential_Store untouched
    - `Stop` stops the server through `HttpServerStop`
    - _Requirements: 4.1, 5.1, 5.7, 5.8, 6.1, 6.2, 6.3, 6.4, 6.5, 6.6_

- [x] 9. pedal.guru: the provisioning page and the mode branch
  - [x] 9.1 Implement `PageProvisioning`
    - Create `src/GUI/Page/PageProvisioning.{cpp,hpp}` as a `BasePage` subclass with the `server_`,
      `networks_`, `screenTexture_`, `state_` and `confirmationStart_` members and the private
      `DrawScreen`, overriding `PreDrawPageContents`, `DrawPageContents` and `PostDrawPageContents` and
      registering them through `Setup()`
    - `PreDrawPageContents` runs the bring-up in order: `WiFiInitialize`, then `WiFiScan` into
      `networks_`, then `WiFiAccessPointStart` with the SSID `pedal.guru` and no password, then
      `server_.Start(networks_)`. A failed or empty scan leaves `networks_` empty and the flow continues;
      a failed access point start or a failed server start sets `UNAVAILABLE`
    - `DrawPageContents` calls `DrawScreen`, returns immediately when `UNAVAILABLE`, otherwise polls the
      server for `SERVER_POLL_MILLISECONDS` of 100, moves to `STORE_FAILED` or to `CONFIGURED` recording
      `Time::TicksMs()` as `confirmationStart_`, and once `CONFIGURED` and the unsigned difference
      against `Time::TicksMs()` has reached the 5000 ms confirmation hold, calls
      `DataManager::SetRestartRequested()` and `drawer_.RequestClose()`. Neither `STORE_FAILED` nor
      `UNAVAILABLE` reaches that branch
    - The hold is a deadline measured with `TicksMs`, never a `Time::Delay`, so the server keeps being
      polled across it
    - `DrawScreen` draws into `screenTexture_` with `Texture::DrawText`, the SSID `pedal.guru` and
      `http://192.168.33.1/` on separate lines and simultaneously visible, and blits it with
      `window_.DrawTexture` the way `PageMapSync` does; `CONFIGURED`, `STORE_FAILED` and `UNAVAILABLE`
      swap the text and stay on screen for the rest of the run
    - `PostDrawPageContents` tears down in order: `HttpServerStop`, `WiFiAccessPointStop`,
      `WiFiDeinitialize`, `screenTexture_.Release()`. It does not unmount the card
    - _Requirements: 2.1, 2.4, 2.5, 3.1, 3.2, 3.3, 3.4, 3.5, 4.7, 7.3, 7.4, 9.2, 9.3_

  - [x] 9.2 Branch the page list on the startup mode
    - In `src/TaskManager.cpp` and `src/TaskManager.hpp`, read the mode at the top of `Execute()` right
      after `ReadSettings()` into a `provisioned_` member through `DataManager::ReadCredentials`, and at
      the existing `CreatePages(drawer)` call site either call `CreatePages` as today or push a single
      `PageProvisioning` into `pages_`
    - Change nothing else: `ReadSettings`, `CreateDevices`, `ConnectToDevices`, the sensor thread,
      `HIDHandler`, `GUINavigator` and `drawer.Execute()` all run in both modes, and `Execute()` keeps
      its `void` signature
    - _Requirements: 1.1, 1.6, 1.7, 1.8, 1.9, 8.4_

  - [x] 9.3 Restart from `app_entry` after the unmount
    - In `src/PedalGuru.cpp`, after the existing `UnMountSdCard()` call, call hal.ll's `DeviceRestart()`
      when `DataManager::GetRestartRequested()` reports a request. That `if` is the whole change to
      `app_entry`, and the unmount call stays exactly where it is
    - _Requirements: 7.1, 7.2, 7.4, 7.5, 9.5, 9.6_

  - [x] 9.4 Add the new sources and include directory to the build
    - In `pedal.guru.cmake`, append `src/GUI/Page/PageProvisioning.cpp`, `src/API/SelfHost/Server.cpp`,
      `src/API/SelfHost/ConfigurationPage.cpp` and `src/API/SelfHost/FormBody.cpp` to
      `PEDAL_GURU_SOURCES`, and `src/API/SelfHost` to `INCLUDE_DIRS`
    - Add no platform folder, no contract include and no directory- or target-scoped command, so the file
      stays evaluable in ESP-IDF's script mode. `src/Platform/ESP32/CMakeLists.txt` needs no change
    - _Requirements: 9.8, 9.13_

- [x] 10. Verify the three builds
  - Configure, compile and link the Simulator, the RP2040 and the ESP32 targets with no error and no new
    warning, per design.md's `## Verification` section. This is the only check that covers the RP2040,
    whose server and access point are stubs. Ask the dev if questions arise.
  - _Requirements: 9.8_

- [x] 11. Verify the flow by running it
  - [x] 11.1 Run the Simulator flow end to end
    - Follow design.md's `## Verification` section, Simulator paragraph: run from the repository root with
      `sample/sdcard.img` holding no `settings` file, and observe the Provisioning_Screen with the SSID
      and the URL, the page served at `http://127.0.0.1/`, the scanned list with a hidden network absent
      and a duplicated SSID appearing once, a rescan that does not clear the password field, a save that
      writes `WIFI_SSID` and `WIFI_PWD` into the image, the confirmation on the page and on the screen,
      the application closing about 5 seconds later, and the next run going straight to the ride pages
    - Then confirm an unrecognised `key=value` line survives a save byte-for-byte, and check the
      rejections the error table lists: an empty SSID, a 33-byte SSID, a 7-character password, and a
      request to `/generate_204`, with the server still serving after each
    - Restore the image afterwards with `git checkout -- sample/sdcard.img`
    - _Requirements: 1.3, 3.1, 4.4, 5.4, 5.5, 5.6, 5.9, 6.3, 6.4, 6.8, 7.2, 7.3_

  - [x] 11.2 Run the ESP32 flow on hardware
    - Follow design.md's `## Verification` section, ESP32 paragraph: join `pedal.guru` from a phone,
      confirm the gateway handed out is 192.168.33.1, open `http://192.168.33.1/`, provision, and confirm
      the device restarts into the ride pages with the credentials on the card
    - _Requirements: 2.2, 2.3, 2.8, 3.1, 7.1_

## Notes

- Each top-level group belongs to one repository and is one submodule commit: tasks 1 (fs.ll), 2
  (hal.ll), 3 and 4 (net.ll), 6 through 9 (pedal.guru).
- There are no test tasks. Verification is the clean build of task 10 and the observed runs of task 11,
  as design.md's `## Verification` section describes.
- Traceability is the `_Requirements:_` lines here and design.md's traceability table. No requirement tag
  goes into the source.
- The RP2040 access point and HTTP server are stubs in this feature. lwIP, `lwipopts.h` and a DHCP server
  on that platform are out of scope.

## Task Dependency Graph

```json
{
  "waves": [
    { "id": 0, "tasks": ["1.1", "2.1", "3.1", "4.1", "6.1", "6.2", "6.3", "6.4"] },
    { "id": 1, "tasks": ["2.2", "2.3", "3.2", "3.3", "3.4", "4.2", "4.3", "4.4", "4.5", "7.1", "8.1", "8.2"] },
    { "id": 2, "tasks": ["7.2", "7.3", "8.3"] },
    { "id": 3, "tasks": ["8.4", "9.1", "9.4"] },
    { "id": 4, "tasks": ["9.2", "9.3"] },
    { "id": 5, "tasks": ["11.1", "11.2"] }
  ]
}
```
