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

namespace GUILib {
#ifdef GUI240X240
    #include "Interface/Spi240x240.hpp"
#else
    #include "Interface/DesktopSimulator.hpp"
#endif
}

#include <string>
#include <functional>
#include "GUIDrawer.hpp"

namespace OpenCC {

#define COLOR2RAYLIB(color) CLITERAL(GUILib::Color){ color.r, color.g, color.b, color.a }

void GUIDrawer::Execute() {
    GUILib::InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, std::string("OpenCC Simulator").c_str());

    GUILib::SetTargetFPS(FRAME_RATE);

    while (GUILib::WindowShouldClose())
    {
        GUILib::BeginDrawing();
        {
            GUILib::ClearBackground(GUILib::WHITE);

            if (drawPageContentsMethod_ != nullptr)
                std::invoke(*drawPageContentsMethod_);
        }
        GUILib::EndDrawing();
    }
}

void GUIDrawer::DrawText(std::string text, int posX, int posY, int fontSize, OpenCC::Color color) {
    GUILib::DrawText(text.c_str(), posX, posY, fontSize, COLOR2RAYLIB(color));
}
}