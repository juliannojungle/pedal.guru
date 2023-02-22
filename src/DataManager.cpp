/*
    Open Cycle Computer (aka OpenCC) is an open-source software
    for cycle computers based on DIY hardware (primarily Raspberry Pi).
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

#include <mutex>
#include <list>
#include "Model/GPSFixData.hpp"

namespace OpenCC {

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
    static DataManager * pinstance_;
    static std::mutex mutex_;
    DataManager() {}
    ~DataManager() {}
    std::list<OpenCC::GPSFixData> gpsFixData_;

public:
    /** Singletons should not be cloneable. */
    DataManager(DataManager &other) = delete;

    /** Singletons should not be assignable. */
    void operator=(const DataManager &) = delete;

    void Push(OpenCC::GPSFixData &gpsFixData);
    void Pop(OpenCC::GPSFixData &gpsFixData);

    /**
     * This is the static method that controls the access to the singleton
     * instance. On the first run, it creates a singleton object and places it
     * into the static field. On subsequent runs, it returns the client existing
     * object stored in the static field.
     */
    static DataManager *GetInstance();
};

void DataManager::Push(OpenCC::GPSFixData &gpsFixData) {
    this->gpsFixData_.push_back(gpsFixData);
}

void DataManager::Pop(OpenCC::GPSFixData &gpsFixData) {
    if (this->gpsFixData_.empty()) return;

    gpsFixData = *(this->gpsFixData_.cbegin());
    this->gpsFixData_.pop_front();
}

/** Initializing static members. */
DataManager* DataManager::pinstance_{nullptr};
std::mutex DataManager::mutex_;

/**
 * Static methods should be defined outside the class.
 */
DataManager *DataManager::GetInstance() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (pinstance_ == nullptr)
    {
        pinstance_ = new DataManager();
    }
    return pinstance_;
}

}