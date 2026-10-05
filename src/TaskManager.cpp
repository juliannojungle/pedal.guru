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

#include <iterator>
#include "TaskManager.hpp"
#include "DataManager.hpp"
#include "GUINavigator.hpp"
#include "PageMap.hpp"
#include "LocationModule.hpp"

extern "C" {
    #include "HAL.h"
}

namespace PedalGuru {

std::list<std::unique_ptr<Device>> TaskManager::devices_;
std::list<AvailablePages> TaskManager::pages_;
bool TaskManager::running_;

void TaskManager::Execute() {
    bool provisioned_ = DataManager::GetInstance()->ReadCredentials(credentials_);

    if (!provisioned_) {
        pages_.push_back(AvailablePages::PAGE_PROVISIONING);
    } else {
        CreateDevices();
        ConnectToDevices();
        ThreadStart(GetDevicesData, 1024, "DevicesData"); // Separated task to "read devices data".
        CreatePages();
    }

    GUINavigator::GetInstance().Setup(pages_);
}

void TaskManager::CreatePages() {
    /*
     * The pages order here is crucial, since it represents the pages cycle order!
     */
    SettingsData &settings = DataManager::GetInstance()->Settings();

    if (settings.pageMapEnabled) {
        pages_.push_back(AvailablePages::PAGE_MAP);
    }

    if (settings.pageRouteEnabled) {
        pages_.push_back(AvailablePages::PAGE_ROUTE);
    }

    if (settings.pageHillsGraphEnabled) {
        pages_.push_back(AvailablePages::PAGE_HILLS_GRAPH);
    }

    if (settings.pageDistanceEnabled) {
        pages_.push_back(AvailablePages::PAGE_DISTANCE);
    }

    if (settings.pageAltimetryEnabled) {
        pages_.push_back(AvailablePages::PAGE_ALTIMETRY);
    }

    if (settings.pageSummaryEnabled) {
        pages_.push_back(AvailablePages::PAGE_SUMMARY);
    }

    // Settings pages aren't optional.
    pages_.push_back(AvailablePages::PAGE_MAP_SYNC);
    pages_.push_back(AvailablePages::PAGE_PROVISIONING);
}

void TaskManager::CreateDevices() {
    //TODO: condition to settings
    devices_.push_back(std::make_unique<LocationModule>());
}

void TaskManager::ConnectToDevices() {
    running_ = true;

    for(const auto &device : devices_) {
        if (!device->Connected())
            device->Connect();
    }
}

void TaskManager::GetDevicesData() {
    if (devices_.size() == 0) return;

    auto device = devices_.begin();

    while (running_)
    {
        if (device->get()->Connected())
            device->get()->GetData();

        if (device == std::prev(devices_.end())) {
            device = devices_.begin();
        } else {
            std::advance(device, 1);
        }

        Delay(1000);//TODO something better.
    }
}

TaskManager::~TaskManager() {
    running_ = false;

    for(const auto &device : devices_) {
        if (device.get()->Connected())
            device.get()->Disconnect();
    }
}

}