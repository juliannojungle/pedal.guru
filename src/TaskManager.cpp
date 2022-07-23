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

#include <memory>
#include <list>
#include "Device/iDevice.hpp"
#include "GUI/GUIDrawer.cpp"
#include "GUI/GUINavigator.cpp"
#include "GUI/Page/BasePage.cpp"
#include "GUI/Page/PageAltimetry.cpp"
#include "GUI/Page/PageDistance.cpp"
#include "GUI/Page/PageHillsGraph.cpp"
#include "GUI/Page/PageMap.cpp"
#include "GUI/Page/PageMapSync.cpp"
#include "GUI/Page/PageRoute.cpp"
#include "GUI/Page/PageSummary.cpp"
#include "HIDHandler.cpp"
#include "Model/SettingsData.hpp"

namespace OpenCC {

class TaskManager {
    private:
        OpenCC::SettingsData settings_;
        std::list<std::unique_ptr<OpenCC::iDevice>> devices_;
        std::list<std::unique_ptr<OpenCC::BasePage>> pages_;
        void ReadSettings();
        void StartDevice(OpenCC::iDevice& device);
        void CreatePages(OpenCC::GUIDrawer& drawer);
    public:
        ~TaskManager() {}
        void Execute();
};

void TaskManager::Execute() {
    ReadSettings();
    OpenCC::GUIDrawer drawer;
    CreatePages(drawer);
    OpenCC::HIDHandler handler;
    OpenCC::GUINavigator guiNavigator(handler, pages_);
    drawer.Execute();
}

void TaskManager::CreatePages(OpenCC::GUIDrawer& drawer) {
    /*
     * The pages order here is crucial, since it represents the pages cycle order!
     */
    if (settings_.pageMapEnabled) {
        pages_.push_back(std::make_unique<OpenCC::PageMap>(drawer, settings_));
    }

    if (settings_.pageRouteEnabled) {
        pages_.push_back(std::make_unique<OpenCC::PageRoute>(drawer, settings_));
    }

    if (settings_.pageHillsGraphEnabled) {
        pages_.push_back(std::make_unique<OpenCC::PageHillsGraph>(drawer, settings_));
    }

    if (settings_.pageDistanceEnabled) {
        pages_.push_back(std::make_unique<OpenCC::PageDistance>(drawer, settings_));
    }

    if (settings_.pageAltimetryEnabled) {
        pages_.push_back(std::make_unique<OpenCC::PageAltimetry>(drawer, settings_));
    }

    if (settings_.pageSummaryEnabled) {
        pages_.push_back(std::make_unique<OpenCC::PageSummary>(drawer, settings_));
    }

    // Settings pages aren't optional.
    pages_.push_back(std::make_unique<OpenCC::PageMapSync>(drawer, settings_));
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

void TaskManager::StartDevice(OpenCC::iDevice& device) {
    //TODO
}
}