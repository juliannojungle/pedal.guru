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

#include <list>
#include "Device/iDevice.hpp"
#include "GUI/GUIDrawer.cpp"
#include "GUI/GUINavigator.cpp"
#include "GUI/Page/iPage.hpp"
#include "GUI/Page/PageAltimetry.cpp"
#include "GUI/Page/PageDistance.cpp"
#include "GUI/Page/PageHillsGraph.cpp"
#include "GUI/Page/PageMap.cpp"
#include "GUI/Page/PageRoute.cpp"
#include "GUI/Page/PageSummary.cpp"
#include "HIDHandler.cpp"
#include "Model/SettingsData.hpp"

namespace OpenCC {

class TaskManager {
    private:
        OpenCC::SettingsData *settings_;
        std::list<OpenCC::iDevice*> devices_;
        std::list<OpenCC::iPage*> pages_;
        void ReadSettings();
        void StartDevice(OpenCC::iDevice *device);
        void CreatePages(OpenCC::GUIDrawer *drawer);
    public:
        void Execute();
};

void TaskManager::Execute() {
    ReadSettings();
    OpenCC::GUIDrawer drawer;
    CreatePages(&drawer);
    OpenCC::HIDHandler hidHandler;
    OpenCC::GUINavigator guiNavigator(&hidHandler, this->pages_);
    drawer.Execute();
}

void TaskManager::CreatePages(OpenCC::GUIDrawer *drawer) {
    if (settings_->pageAltimetryEnabled) {
        OpenCC::iPage *page;
        OpenCC::PageAltimetry pageAltimetry(drawer, this->settings_);
        page = &pageAltimetry;
        pages_.push_back(page);
    }

    if (settings_->pageDistanceEnabled) {
        OpenCC::iPage *page;
        OpenCC::PageDistance pageDistance(drawer, this->settings_);
        page = &pageDistance;
        pages_.push_back(page);
    }

    if (settings_->pageHillsGraphEnabled) {
        OpenCC::iPage *page;
        OpenCC::PageHillsGraph pageHillsGraph(drawer, this->settings_);
        page = &pageHillsGraph;
        pages_.push_back(page);
    }

    if (settings_->pageMapEnabled) {
        OpenCC::iPage *page;
        OpenCC::PageMap pageMap(drawer, this->settings_);
        page = &pageMap;
        pages_.push_back(page);
    }

    if (settings_->pageRouteEnabled) {
        OpenCC::iPage *page;
        OpenCC::PageRoute pageRoute(drawer, this->settings_);
        page = &pageRoute;
        pages_.push_back(page);
    }

    if (settings_->pageSummaryEnabled) {
        OpenCC::iPage *page;
        OpenCC::PageSummary pageSummary(drawer, this->settings_);
        page = &pageSummary;
        pages_.push_back(page);
    }
}

void TaskManager::ReadSettings() {
    OpenCC::SettingsData settings; // TODO: Here we need saved settings.
    settings.pageAltimetryEnabled = true;
    settings.pageDistanceEnabled = true;
    settings.pageHillsGraphEnabled = true;
    settings.pageMapEnabled = true;
    settings.pageRouteEnabled = true;
    settings.pageSummaryEnabled = true;
    this->settings_ = &settings;
}

void TaskManager::StartDevice(OpenCC::iDevice *device) {
    //TODO
}
}