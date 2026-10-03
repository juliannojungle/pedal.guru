# Requirements Document

## Introduction

This feature unifies application settings ownership inside `DataManager` and exposes the enabled optional pages through the Wi-Fi provisioning configuration page. Today the six page-enable flags and the map tile base URL live in a `SettingsData` struct seeded with hardcoded defaults in `TaskManager::ReadSettings()`, and they are passed by reference through `BasePage` and `GUINavigator`. The feature moves that state into `DataManager`, persists it in the existing key=value settings file, and lets the rider choose which optional pages are active during provisioning. The map tile base URL also becomes a persisted key with a default when absent. A stale recovery comment in `DataManager.cpp` is verified and removed if confirmed stale.

## Glossary

- **DataManager**: Application singleton that shares data between the sensor and UI threads and owns persisted settings access.
- **SettingsData**: In-memory structure holding six page-enable booleans and the map syncing base URL.
- **Settings_File**: The `settings` file on the SD card, stored as `key=value` lines, already holding `WIFI_SSID` and `WIFI_PWD`.
- **Settings_Accessor**: The `DataManager::GetInstance()->Settings()` method returning a reference to the cached SettingsData.
- **Optional_Page**: One of PageAltimetry, PageDistance, PageHillsGraph, PageRoute, PageSummary, PageMap.
- **TaskManager**: Component that reads settings, creates devices and builds the page cycle.
- **ConfigurationPage**: The HTML provisioning page served over Wi-Fi.
- **Server**: The provisioning HTTP server whose `OnSave` handler persists submitted values.
- **MAP_BASE_URL_DEFAULT**: The literal `https://tile.openstreetmap.org`.

## Requirements

### Requirement 1

**User Story:** As a developer, I want DataManager to own SettingsData, so that settings have a single source of truth with lazy loading.

#### Acceptance Criteria

1. THE DataManager SHALL expose a Settings_Accessor method returning a reference to a cached SettingsData instance.
2. WHEN the Settings_Accessor is called for the first time, THE DataManager SHALL read the Settings_File, populate the cached SettingsData, and return the cached instance.
3. WHEN the Settings_Accessor is called after the cached SettingsData exists, THE DataManager SHALL return the cached instance without reading the Settings_File.
4. WHEN a caller modifies a field through the Settings_Accessor reference, THE DataManager SHALL retain the modified value in the cached SettingsData for subsequent calls.
5. IF a page-enable key is absent from the Settings_File, THEN THE DataManager SHALL set the corresponding SettingsData boolean to true.
6. IF the map syncing base URL key is absent from the Settings_File, THEN THE DataManager SHALL set the SettingsData map syncing base URL to MAP_BASE_URL_DEFAULT.

### Requirement 2

**User Story:** As a developer, I want settings persisted in the existing key=value file, so that the same storage mechanism serves credentials and settings.

#### Acceptance Criteria

1. THE DataManager SHALL persist each of the six page-enable booleans and the map syncing base URL as `key=value` lines in the Settings_File using the existing SettingsFileData, SettingsEntry, ReadSettingsFile and WriteSettingsFile mechanism.
2. WHEN the DataManager persists a page-enable boolean, THE DataManager SHALL write the value as a boolean representation under its dedicated key.
3. WHEN the DataManager writes a settings value for a key already present in the Settings_File, THE DataManager SHALL update the existing entry rather than append a duplicate entry.
4. THE DataManager SHALL preserve the existing `WIFI_SSID` and `WIFI_PWD` entries when writing page-enable or map syncing base URL entries.

### Requirement 3

**User Story:** As a developer, I want all SettingsData use sites to read from DataManager, so that the per-reference passing through pages is removed.

#### Acceptance Criteria

1. THE TaskManager SHALL obtain page-enable values through the Settings_Accessor when building the page cycle.
2. THE TaskManager SHALL remove the hardcoded defaults previously assigned in ReadSettings() in favor of DataManager-owned defaults.
3. THE BasePage SHALL remove its SettingsData member and SettingsData constructor parameter.
4. THE GUINavigator SHALL remove its SettingsData member and SHALL construct pages without passing SettingsData.
5. WHEN PageMapSync downloads a tile, THE PageMapSync SHALL obtain the map syncing base URL through the Settings_Accessor.

### Requirement 4

**User Story:** As a rider, I want to choose which optional pages are active during provisioning, so that my device only cycles through the screens I want.

#### Acceptance Criteria

1. THE ConfigurationPage SHALL present one checkbox for each of the six Optional_Pages.
2. THE ConfigurationPage SHALL NOT present a checkbox for PageMapSync.
3. THE ConfigurationPage SHALL include the state of each Optional_Page checkbox in the save request submitted to the Server alongside the network name and password fields.
4. IF the save request contains no selected Optional_Page, THEN THE ConfigurationPage SHALL block submission and prompt the rider to select at least one page.

### Requirement 5

**User Story:** As a rider, I want my page selections persisted on save, so that they survive a device restart.

#### Acceptance Criteria

1. WHEN the Server handles a save request with at least one selected Optional_Page and valid credentials, THE Server SHALL persist each Optional_Page selection as a boolean in the Settings_File.
2. IF a save request contains no selected Optional_Page, THEN THE Server SHALL reject the request with a client error status and SHALL NOT persist page selections.
3. THE Server SHALL handle credential persistence and page-selection persistence in separate responsibilities within OnSave.

### Requirement 6

**User Story:** As a developer, I want the stale recovery comment verified, so that documentation reflects the actual code behavior.

#### Acceptance Criteria

1. THE developer SHALL verify whether the comment above the ReadSettingsFile call in ReadCredentials accurately describes current fs.ll unmount-and-recover behavior.
2. WHERE the comment is confirmed stale, THE DataManager SHALL remove the comment and the recovery note SHALL be removed from AGENTS.md.
3. WHERE the comment is confirmed accurate, THE DataManager SHALL retain the comment unchanged.
