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

#include "GUIDrawer.hpp"

extern "C" {
    #include "LCDSetup.h"
}

namespace PedalGuru {

void GUIDrawer::SetPageContentsPreDrawMethod(std::function<void()> method) {
    pageContentsPreDrawCallback_ = std::make_shared<PedalGuru::Callback>(method);
};

void GUIDrawer::SetPageContentsDrawMethod(std::function<void()> method) {
    pageContentsDrawCallback_ = std::make_shared<PedalGuru::Callback>(method);
};

void GUIDrawer::SetPageContentsPostDrawMethod(std::function<void()> method) {
    pageContentsPostDrawCallback_ = std::make_shared<PedalGuru::Callback>(method);
};

void GUIDrawer::Execute() {
    window.Init(LCD.WIDTH, LCD.HEIGHT, std::string("PedalGuru").c_str());

    if (pageContentsPreDrawCallback_ != nullptr)
        pageContentsPreDrawCallback_->Method();

    while (!window.ShouldClose())
    {
        window.BeginDrawing();
        {
            window.ClearBackground(Render::COLOR_WHITE);

            if (pageContentsDrawCallback_ != nullptr)
                pageContentsDrawCallback_->Method();
        }
        window.EndDrawing();
    }

    if (pageContentsPostDrawCallback_ != nullptr)
        pageContentsPostDrawCallback_->Method();
    window.Close();
}

}