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

#pragma once

#include "Mutex.hpp"
#include <list>
#include <string>
#include "GPSFixData.hpp"
#include "SettingsEntry.hpp"
#include "CredentialData.hpp"

namespace PedalGuru {

/**
 * The DataManager class is a singleton with a `GetInstance` static method
 * to return always the same instance.
 */
class DataManager {
    /**
     * The singleton's constructor/destructor should always be private to
     * prevent direct construction/desctruction calls with the `new`/`delete`
     * operator.
     */
private:
    static DataManager * instance_;
    static Mutex mutex_;
    DataManager() {}
    ~DataManager() {}
    std::list<PedalGuru::GPSFixData> gpsFixData_;
    bool restartRequested_{false};
    bool ReadSettingsFile(PedalGuru::SettingsFileData &fileData);
    bool WriteSettingsFile(const PedalGuru::SettingsFileData &fileData);

public:
    /** Singletons should not be cloneable. */
    DataManager(DataManager &other) = delete;

    /** Singletons should not be assignable. */
    void operator=(const DataManager &) = delete;

    void Push(PedalGuru::GPSFixData &gpsFixData);
    void Pop(PedalGuru::GPSFixData &gpsFixData);
    bool ReadCredentials(PedalGuru::CredentialData &credentials);
    bool WriteCredentials(const std::string &ssid, const std::string &password);
    void SetRestartRequested();
    bool GetRestartRequested();

    /**
     * This is the static method that controls the access to the singleton
     * instance. On the first run, it creates a singleton object and places it
     * into the static field. On subsequent runs, it returns existing object
     * stored in the static field.
     */
    static DataManager *GetInstance();
};

}