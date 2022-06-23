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
#include "../Model/Callback.hpp"

namespace OpenCC {

struct GUIColor {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
    GUIColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
        : r(red), g(green), b(blue), a(alpha) {}
};

class GUIDrawer {
    private:
        Callback drawPageContentsCallback_;
    public:
        GUIDrawer() {}
        void Execute();
        void SetDrawPageContentsMethod(std::function<void()> drawPage);

        // PARSER MEMBERS: ONLY the GUIDrawer should know the implementations types!
        const OpenCC::GUIColor COLOR_LIGHT_GRAY  {200, 200, 200, 255};
        const OpenCC::GUIColor COLOR_GRAY        {130, 130, 130, 255};
        const OpenCC::GUIColor COLOR_DARK_GRAY   {80, 80, 80, 255};
        const OpenCC::GUIColor COLOR_YELLOW      {253, 249, 0, 255};
        const OpenCC::GUIColor COLOR_GOLD        {255, 203, 0, 255};
        const OpenCC::GUIColor COLOR_ORANGE      {255, 161, 0, 255};
        const OpenCC::GUIColor COLOR_PINK        {255, 109, 194, 255};
        const OpenCC::GUIColor COLOR_RED         {230, 41, 55, 255};
        const OpenCC::GUIColor COLOR_MAROON      {190, 33, 55, 255};
        const OpenCC::GUIColor COLOR_GREEN       {0, 228, 48, 255};
        const OpenCC::GUIColor COLOR_LIME        {0, 158, 47, 255};
        const OpenCC::GUIColor COLOR_DARK_GREEN  {0, 117, 44, 255};
        const OpenCC::GUIColor COLOR_SKY_BLUE    {102, 191, 255, 255};
        const OpenCC::GUIColor COLOR_BLUE        {0, 121, 241, 255};
        const OpenCC::GUIColor COLOR_DARK_BLUE   {0, 82, 172, 255};
        const OpenCC::GUIColor COLOR_PURPLE      {200, 122, 255, 255};
        const OpenCC::GUIColor COLOR_VIOLET      {135, 60, 190, 255};
        const OpenCC::GUIColor COLOR_DARK_PURPLE {112, 31, 126, 255};
        const OpenCC::GUIColor COLOR_BEIGE       {211, 176, 131, 255};
        const OpenCC::GUIColor COLOR_BROWN       {127, 106, 79, 255};
        const OpenCC::GUIColor COLOR_DARK_BROWN  {76, 63, 47, 255};
        const OpenCC::GUIColor COLOR_WHITE       {255, 255, 255, 255};
        const OpenCC::GUIColor COLOR_BLACK       {0, 0, 0, 255};
        const OpenCC::GUIColor COLOR_TRANSPARENT {0, 0, 0, 0};
        const OpenCC::GUIColor COLOR_MAGENTA     {255, 0, 255, 255};
        void DrawText(std::string text, int posX, int posY, int fontSize, OpenCC::GUIColor color);
};
}