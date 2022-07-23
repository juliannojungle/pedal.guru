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

#include <string>
#include <functional>
#include <memory>
#include "../Model/Callback.hpp"
#include "Render/Color.cpp"
#include "Render/Rectangle.cpp"
#include "Render/Image.cpp"
#include "Render/Texture.cpp"
#include "Render/Window.cpp"

#ifdef DESKTOP_SIMULATOR
#include "Interface/DesktopSimulator.hpp"
#else
#include "Interface/Spi240x240.hpp"
#endif

namespace OpenCC {

class GUIDrawer {
    private:
        PiRender::Window window;
        std::shared_ptr<OpenCC::Callback> pageContentsPreDrawCallback_;
        std::shared_ptr<OpenCC::Callback> pageContentsDrawCallback_;
        std::shared_ptr<OpenCC::Callback> pageContentsPostDrawCallback_;
    public:
        GUIDrawer() {}
        ~GUIDrawer() {}
        void Execute();

        void SetPageContentsPreDrawMethod(std::function<void()> method);
        void SetPageContentsDrawMethod(std::function<void()> method);
        void SetPageContentsPostDrawMethod(std::function<void()> method);
};
}