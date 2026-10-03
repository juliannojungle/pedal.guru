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

#include <memory>
#include <list>
#include "BasePage.hpp"
#include "Callback.hpp"

namespace PedalGuru {

class GUINavigator {
    private:
        static void ExecuteGuiDrawer();

        GUINavigator() {}
        std::unique_ptr<BasePage> currentPage_;
        std::list<AvailablePages> *pages_;
        std::list<AvailablePages>::iterator pageIndex_;
        std::list<std::shared_ptr<Callback>>::const_iterator previousPageReference_;
        std::list<std::shared_ptr<Callback>>::const_iterator nextPageReference_;
        void RegisterEvents();
        void UnregisterEvents();
        void GoToNextPage();
        void GoToPreviousPage();
        std::unique_ptr<BasePage> GetPage(AvailablePages page);
    public:
        GUINavigator(GUINavigator& other) = delete;
        void operator=(const GUINavigator&) = delete;
        static GUINavigator& GetInstance();
        void Setup(std::list<AvailablePages>& pages);
};

}