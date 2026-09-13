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
#include <string>

extern "C" {
    #include "FileSystem.h"
}

namespace PedalGuru {

const char *SETTINGS_FILE_NAME = "settings";
const char *WIFI_SSID_KEY = "WIFI_SSID";
const char *WIFI_PWD_KEY = "WIFI_PWD";
const unsigned int SETTINGS_CHUNK_SIZE = 512;
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
    mutex_.Lock();
    this->gpsFixData_.push_back(gpsFixData);
    mutex_.Release();
}

void DataManager::Pop(PedalGuru::GPSFixData &gpsFixData) {
    mutex_.Lock();

    if (!this->gpsFixData_.empty()) {
        gpsFixData = *(this->gpsFixData_.cbegin());
        this->gpsFixData_.pop_front();
    }

    mutex_.Release();
}

bool DataManager::ReadSettingsFile(PedalGuru::SettingsFileData &fileData) {
    FIL file;

    if (!OpenFile(&file, SETTINGS_FILE_NAME)) {
        return false;
    }

    std::string content;
    char chunk[SETTINGS_CHUNK_SIZE];
    unsigned int bytesRead;

    do {
        bytesRead = ReadFile(&file, chunk, SETTINGS_CHUNK_SIZE);
        content.append(chunk, bytesRead);
    } while (bytesRead == SETTINGS_CHUNK_SIZE && content.length() < SETTINGS_MAX_SIZE);

    CloseFile(&file);
    ParseSettingsContent(content, fileData);

    return true;
}

bool DataManager::ReadCredentials(PedalGuru::CredentialData &credentials) {
    mutex_.Lock();

    credentials.ssid.clear();
    credentials.password.clear();
    credentials.ssidPresent = false;
    credentials.passwordPresent = false;

    if (!PathOrFileExists(SETTINGS_FILE_NAME)) {
        mutex_.Release();
        return false;
    }

    PedalGuru::SettingsFileData fileData;

    /** fs.ll unmounts the volume when OpenFile fails, and later card access depends on it. */
    if (!ReadSettingsFile(fileData)) {
        MountSdCard();
        SelectActiveDrive();
        mutex_.Release();
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

    mutex_.Release();
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

    if (!OpenFile(&file, SETTINGS_FILE_NAME)) {
        return false;
    }

    unsigned int bytesWritten = WriteFile(&file, const_cast<char *>(content.data()), content.length());

    if (bytesWritten != content.length()) {
        CloseFile(&file);
        return false;
    }

    /** OpenFile does not truncate, so the cut belongs at the pointer left by the single write. */
    bool truncated = TruncateFile(&file);
    CloseFile(&file);

    return truncated;
}

bool DataManager::WriteCredentials(const std::string &ssid, const std::string &password) {
    mutex_.Lock();

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

    mutex_.Release();
    return written;
}

void DataManager::SetRestartRequested() {
    mutex_.Lock();
    this->restartRequested_ = true;
    mutex_.Release();
}

bool DataManager::GetRestartRequested() {
    mutex_.Lock();
    bool requested = this->restartRequested_;
    mutex_.Release();

    return requested;
}

/** Initializing static members. */
DataManager* DataManager::instance_{nullptr};
Mutex DataManager::mutex_;

/** Static methods should be defined outside the class. */
DataManager *DataManager::GetInstance() {
    mutex_.Lock();
    if (instance_ == nullptr)
    {
        instance_ = new DataManager();
    }
    mutex_.Release();

    return instance_;
}

}