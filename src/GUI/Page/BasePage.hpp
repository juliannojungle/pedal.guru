/*
    Pedal.guru is an open-source software
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

#include "SettingsData.hpp"
#include "GUIDrawer.hpp"

namespace PedalGuru {

class BasePage {
    protected:
        PedalGuru::SettingsData& settings_;
        PedalGuru::GUIDrawer& drawer_;
        Window window_;
    public:
        virtual ~BasePage() = default; // make it polymorphic
        BasePage(PedalGuru::GUIDrawer& drawer, PedalGuru::SettingsData& settings)
            : settings_(settings), drawer_(drawer) {}
        virtual void PreDrawPageContents() = 0;
        virtual void DrawPageContents() = 0;
        virtual void PostDrawPageContents() = 0;
        virtual void Setup();
};

}