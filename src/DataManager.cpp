/*
    Pedal.guru is an open-source software
    for cycle computers based on DIY hardware (MCUs like RP2040 and ESP32-S3).
    Copyright (C) 2022, Julianno F. C. Silva (@juliannojungle)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/agpl-3.0.html>.
*/

#include "DataManager.hpp"
#include <cctype>
#include <string>

extern "C" {
    #include "FileSystem.h"
}

namespace PedalGuru {

const char *SETTINGS_FILE_NAME = "settings";
const char *WIFI_SSID_KEY = "WIFI_SSID";
const char *WIFI_PWD_KEY = "WIFI_PWD";
const char *PAGE_ALTIMETRY_KEY = "PAGE_ALTIMETRY";
const char *PAGE_DISTANCE_KEY = "PAGE_DISTANCE";
const char *PAGE_HILLS_GRAPH_KEY = "PAGE_HILLS_GRAPH";
const char *PAGE_MAP_KEY = "PAGE_MAP";
const char *PAGE_ROUTE_KEY = "PAGE_ROUTE";
const char *PAGE_SUMMARY_KEY = "PAGE_SUMMARY";
const char *MAP_BASE_URL_KEY = "MAP_BASE_URL";
const char *MAP_BASE_URL_DEFAULT = "https://tile.openstreetmap.org";
const unsigned int SETTINGS_BUFFER_SIZE = 512;
const unsigned int SETTINGS_MAX_SIZE = 4096;
const std::size_t SSID_MAX_LENGTH = 32;

static void ParseSettingsLine(const std::string &line, SettingsFileData &fileData) {
    SettingsEntry entry;
    auto separator = line.find('=');

    if (separator == std::string::npos) {
        entry.value = line;
    } else {
        entry.key = line.substr(0, separator);
        entry.value = line.substr(separator + 1);
    }

    fileData.entries.push_back(entry);
}

static void ParseSettingsContent(const std::string &content, SettingsFileData &fileData) {
    std::string line;

    for (auto character : content) {
        if (character != '\n') {
            line += character;
            continue;
        }

        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        ParseSettingsLine(line, fileData);
        line.clear();
    }

    if (!line.empty()) {
        ParseSettingsLine(line, fileData);
    }
}

void DataManager::Push(PedalGuru::GPSFixData &gpsFixData) {
    mutex_->Lock();
    this->gpsFixData_.push_back(gpsFixData);
    mutex_->Release();
}

void DataManager::Pop(PedalGuru::GPSFixData &gpsFixData) {
    mutex_->Lock();

    if (!this->gpsFixData_.empty()) {
        gpsFixData = *(this->gpsFixData_.cbegin());
        this->gpsFixData_.pop_front();
    }

    mutex_->Release();
}

bool DataManager::ReadSettingsFile(PedalGuru::SettingsFileData &fileData) {
    FIL file;
    mutex_->Lock();

    if (!OpenFile(&file, SETTINGS_FILE_NAME)) {
        mutex_->Release();
        return false;
    }

    std::string content;
    char buffer[SETTINGS_BUFFER_SIZE];
    unsigned int bytesRead;

    do {
        bytesRead = ReadFile(&file, buffer, sizeof(buffer) - 1);
        buffer[bytesRead] = '\0';
        content.append(buffer, bytesRead);
    } while (bytesRead == SETTINGS_BUFFER_SIZE && content.length() < SETTINGS_MAX_SIZE);

    CloseFile(&file);
    mutex_->Release();
    ParseSettingsContent(content, fileData);

    return true;
}

bool DataManager::ReadCredentials(PedalGuru::CredentialData &credentials) {
    credentials.ssid.clear();
    credentials.password.clear();
    credentials.ssidPresent = false;
    credentials.passwordPresent = false;

    if (!PathOrFileExists(SETTINGS_FILE_NAME)) {
        return false;
    }

    PedalGuru::SettingsFileData fileData;

    /** fs.ll unmounts the volume when OpenFile fails, and later card access depends on it. */
    if (!ReadSettingsFile(fileData)) {
        MountSdCard();
        SelectActiveDrive();
        return false;
    }

    for (const auto &entry : fileData.entries) {
        if (entry.key == WIFI_SSID_KEY) {
            credentials.ssid = entry.value;
            credentials.ssidPresent = true;
        } else if (entry.key == WIFI_PWD_KEY) {
            credentials.password = entry.value;
            credentials.passwordPresent = true;
        }
    }

    bool provisioned = credentials.ssidPresent
        && credentials.passwordPresent
        && !credentials.ssid.empty()
        && credentials.ssid.length() <= SSID_MAX_LENGTH;

    return provisioned;
}

bool DataManager::WriteSettingsFile(const PedalGuru::SettingsFileData &fileData) {
    std::string content;

    for (const auto &entry : fileData.entries) {
        if (entry.key.empty()) {
            content += entry.value;
        } else {
            content += entry.key;
            content += '=';
            content += entry.value;
        }

        content += '\n';
    }

    FIL file;
    mutex_->Lock();

    if (!OpenFile(&file, SETTINGS_FILE_NAME)) {
        mutex_->Release();
        return false;
    }

    unsigned int bytesWritten = WriteFile(&file, const_cast<char *>(content.data()), content.length());

    if (bytesWritten != content.length()) {
        CloseFile(&file);
        mutex_->Release();
        return false;
    }

    /** OpenFile does not truncate, so the cut belongs at the pointer left by the single write. */
    bool truncated = TruncateFile(&file);
    CloseFile(&file);
    mutex_->Release();

    return truncated;
}

bool DataManager::ParseBool(const std::string &value, bool fallback) {
    static const std::string trueText = "true";

    if (value.length() != trueText.length()) {
        return fallback;
    }

    for (std::size_t index = 0; index < value.length(); ++index) {
        if (std::tolower(static_cast<unsigned char>(value[index])) != trueText[index]) {
            return fallback;
        }
    }

    return true;
}

const char *DataManager::BoolText(bool value) {
    return value ? "true" : "false";
}

void DataManager::UpsertEntry(PedalGuru::SettingsFileData &fileData, const char *key, const std::string &value) {
    for (auto &entry : fileData.entries) {
        if (entry.key == key) {
            entry.value = value;
            return;
        }
    }

    SettingsEntry entry;
    entry.key = key;
    entry.value = value;
    fileData.entries.push_back(entry);
}

bool DataManager::WriteCredentials(const std::string &ssid, const std::string &password) {
    PedalGuru::SettingsFileData fileData;
    ReadSettingsFile(fileData);

    bool ssidWritten = false;
    bool passwordWritten = false;

    for (auto &entry : fileData.entries) {
        if (entry.key == WIFI_SSID_KEY) {
            entry.value = ssid;
            ssidWritten = true;
        } else if (entry.key == WIFI_PWD_KEY) {
            entry.value = password;
            passwordWritten = true;
        }
    }

    if (!ssidWritten) {
        SettingsEntry entry;
        entry.key = WIFI_SSID_KEY;
        entry.value = ssid;
        fileData.entries.push_back(entry);
    }

    if (!passwordWritten) {
        SettingsEntry entry;
        entry.key = WIFI_PWD_KEY;
        entry.value = password;
        fileData.entries.push_back(entry);
    }

    bool written = WriteSettingsFile(fileData);
    return written;
}

bool DataManager::WritePageSelection(const PedalGuru::SettingsData &selection) {
    PedalGuru::SettingsFileData fileData;
    ReadSettingsFile(fileData);

    UpsertEntry(fileData, PAGE_ALTIMETRY_KEY, BoolText(selection.pageAltimetryEnabled));
    UpsertEntry(fileData, PAGE_DISTANCE_KEY, BoolText(selection.pageDistanceEnabled));
    UpsertEntry(fileData, PAGE_HILLS_GRAPH_KEY, BoolText(selection.pageHillsGraphEnabled));
    UpsertEntry(fileData, PAGE_MAP_KEY, BoolText(selection.pageMapEnabled));
    UpsertEntry(fileData, PAGE_ROUTE_KEY, BoolText(selection.pageRouteEnabled));
    UpsertEntry(fileData, PAGE_SUMMARY_KEY, BoolText(selection.pageSummaryEnabled));
    UpsertEntry(fileData, MAP_BASE_URL_KEY, selection.mapSyncingBaseUrl);

    return WriteSettingsFile(fileData);
}

void DataManager::ApplyEntry(const PedalGuru::SettingsEntry &entry) {
    if (entry.key == PAGE_ALTIMETRY_KEY) {
        settings_.pageAltimetryEnabled = ParseBool(entry.value, settings_.pageAltimetryEnabled);
    } else if (entry.key == PAGE_DISTANCE_KEY) {
        settings_.pageDistanceEnabled = ParseBool(entry.value, settings_.pageDistanceEnabled);
    } else if (entry.key == PAGE_HILLS_GRAPH_KEY) {
        settings_.pageHillsGraphEnabled = ParseBool(entry.value, settings_.pageHillsGraphEnabled);
    } else if (entry.key == PAGE_MAP_KEY) {
        settings_.pageMapEnabled = ParseBool(entry.value, settings_.pageMapEnabled);
    } else if (entry.key == PAGE_ROUTE_KEY) {
        settings_.pageRouteEnabled = ParseBool(entry.value, settings_.pageRouteEnabled);
    } else if (entry.key == PAGE_SUMMARY_KEY) {
        settings_.pageSummaryEnabled = ParseBool(entry.value, settings_.pageSummaryEnabled);
    } else if (entry.key == MAP_BASE_URL_KEY) {
        settings_.mapSyncingBaseUrl = entry.value;
    }
}

void DataManager::LoadSettings() {
    settings_.pageAltimetryEnabled = true;
    settings_.pageDistanceEnabled = true;
    settings_.pageHillsGraphEnabled = true;
    settings_.pageMapEnabled = true;
    settings_.pageRouteEnabled = true;
    settings_.pageSummaryEnabled = true;
    settings_.mapSyncingBaseUrl = MAP_BASE_URL_DEFAULT;

    PedalGuru::SettingsFileData fileData;

    if (!ReadSettingsFile(fileData)) {
        return;
    }

    for (const auto &entry : fileData.entries) {
        ApplyEntry(entry);
    }
}

PedalGuru::SettingsData &DataManager::Settings() {
    mutex_->Lock();
    bool loaded = settingsLoaded_;
    mutex_->Release();

    if (!loaded) {
        LoadSettings();
        mutex_->Lock();
        settingsLoaded_ = true;
        mutex_->Release();
    }

    return settings_;
}

void DataManager::SetRestartRequested() {
    mutex_->Lock();
    this->restartRequested_ = true;
    mutex_->Release();
}

bool DataManager::GetRestartRequested() {
    mutex_->Lock();
    bool requested = this->restartRequested_;
    mutex_->Release();

    return requested;
}

/** Initializing static members. */
DataManager* DataManager::instance_{nullptr};
Mutex* DataManager::mutex_{nullptr};

/** Static methods should be defined outside the class. */
DataManager *DataManager::GetInstance() {
    if (mutex_ == nullptr) {
        mutex_ = new Mutex();
    }

    mutex_->Lock();

    if (instance_ == nullptr)
    {
        instance_ = new DataManager();
    }

    mutex_->Release();
    return instance_;
}

}