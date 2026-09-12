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

#include "TaskManager.hpp"
#include "DataManager.hpp"
#include "GUINavigator.hpp"
#include "PageAltimetry.hpp"
#include "PageDistance.hpp"
#include "PageHillsGraph.hpp"
#include "PageMap.hpp"
#include "PageMapSync.hpp"
#include "PageProvisioning.hpp"
#include "PageRoute.hpp"
#include "PageSummary.hpp"
#include "HIDHandler.hpp"
#include "LocationModule.hpp"

extern "C" {
    #include "HAL.h"
}

namespace PedalGuru {

std::list<std::unique_ptr<Device>> TaskManager::devices_;
bool TaskManager::running_;

void TaskManager::Execute() {
    ReadSettings();
    provisioned_ = DataManager::GetInstance()->ReadCredentials(credentials_);

    CreateDevices();
    ConnectToDevices();

    /*
     * Start a parallel task to keep reading devices data,
     * while the main core keeps handling HID and GUI.
     */
    ThreadStart(GetDevicesData);

    GUIDrawer drawer;

    if (provisioned_) {
        CreatePages(drawer);
    } else {
        pages_.push_back(std::make_unique<PageProvisioning>(drawer, settings_));
    }

    HIDHandler handler;
    GUINavigator guiNavigator(handler, pages_);
    drawer.Execute();
}

void TaskManager::CreatePages(GUIDrawer& drawer) {
    /*
     * The pages order here is crucial, since it represents the pages cycle order!
     */
    if (settings_.pageMapEnabled) {
        pages_.push_back(std::make_unique<PageMap>(drawer, settings_));
    }

    if (settings_.pageRouteEnabled) {
        pages_.push_back(std::make_unique<PageRoute>(drawer, settings_));
    }

    if (settings_.pageHillsGraphEnabled) {
        pages_.push_back(std::make_unique<PageHillsGraph>(drawer, settings_));
    }

    if (settings_.pageDistanceEnabled) {
        pages_.push_back(std::make_unique<PageDistance>(drawer, settings_));
    }

    if (settings_.pageAltimetryEnabled) {
        pages_.push_back(std::make_unique<PageAltimetry>(drawer, settings_));
    }

    if (settings_.pageSummaryEnabled) {
        pages_.push_back(std::make_unique<PageSummary>(drawer, settings_));
    }

    // Settings pages aren't optional.
    pages_.push_back(std::make_unique<PageMapSync>(drawer, settings_));
}

void TaskManager::ReadSettings() {
    // TODO: Here we need saved settings.
    this->settings_.pageAltimetryEnabled = true;
    this->settings_.pageDistanceEnabled = true;
    this->settings_.pageHillsGraphEnabled = true;
    this->settings_.pageMapEnabled = true;
    this->settings_.pageRouteEnabled = true;
    this->settings_.pageSummaryEnabled = true;
    this->settings_.mapSyncingBaseUrl = "https://tile.openstreetmap.org";
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
    auto device = devices_.begin();

    while (running_)
    {
        if (device->get()->Connected())
            device->get()->GetData();

        device = (device == devices_.end()) ? devices_.begin() : device++;

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