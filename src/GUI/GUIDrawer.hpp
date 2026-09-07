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

#include <functional>
#include <memory>
#include "Callback.hpp"
#include "Window.hpp"

namespace PedalGuru {

class GUIDrawer {
    private:
        Window window;
        std::shared_ptr<PedalGuru::Callback> pageContentsPreDrawCallback_;
        std::shared_ptr<PedalGuru::Callback> pageContentsDrawCallback_;
        std::shared_ptr<PedalGuru::Callback> pageContentsPostDrawCallback_;
        bool closeRequested_ {false};
    public:
        void Execute();
        void RequestClose();
        void SetPageContentsPreDrawMethod(std::function<void()> method);
        void SetPageContentsDrawMethod(std::function<void()> method);
        void SetPageContentsPostDrawMethod(std::function<void()> method);
};
}