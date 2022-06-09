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
#include "opencc.hpp"
#include "devices/iDevice.hpp"
#include "gui/pages/iPage.hpp"

class TaskManager {
    private:
        SettingsData *_settings;
        std::list<iDevice*> _devices;
        std::list<iPage*> _pages;
        SettingsData ReadSettings();
        void DisplayPage(iPage *page);
        void StartDevice(iDevice *device);
    public:
        void Run();
};

void TaskManager::Run() {
    //TODO
}

SettingsData TaskManager::ReadSettings() {
    //TODO
}

void TaskManager::DisplayPage(iPage *page) {
    //TODO
}

void TaskManager::StartDevice(iDevice *device) {
    //TODO
}