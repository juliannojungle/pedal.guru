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

namespace GUIDriver {
/* The raylib dependency must be the last one, so it doesn't cause building problems due it's dependencies */
#ifdef USE_RAYLIB
extern "C" {
    #include "../../Dependency/raylib/src/raylib.h"
}
#endif
}

namespace PiRender {

#define COLOR_TO_RAYLIB(color) CLITERAL(GUIDriver::Color) \
    { color.red, color.green, color.blue, color.alpha }

class Color {
    public:
        unsigned char red;
        unsigned char green;
        unsigned char blue;
        unsigned char alpha;
        Color(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
            : red(red), green(green), blue(blue), alpha(alpha) {}
};

const PiRender::Color COLOR_LIGHT_GRAY  {200, 200, 200, 255};
const PiRender::Color COLOR_GRAY        {130, 130, 130, 255};
const PiRender::Color COLOR_DARK_GRAY   {80, 80, 80, 255};
const PiRender::Color COLOR_YELLOW      {253, 249, 0, 255};
const PiRender::Color COLOR_GOLD        {255, 203, 0, 255};
const PiRender::Color COLOR_ORANGE      {255, 161, 0, 255};
const PiRender::Color COLOR_PINK        {255, 109, 194, 255};
const PiRender::Color COLOR_RED         {230, 41, 55, 255};
const PiRender::Color COLOR_MAROON      {190, 33, 55, 255};
const PiRender::Color COLOR_GREEN       {0, 228, 48, 255};
const PiRender::Color COLOR_LIME        {0, 158, 47, 255};
const PiRender::Color COLOR_DARK_GREEN  {0, 117, 44, 255};
const PiRender::Color COLOR_SKY_BLUE    {102, 191, 255, 255};
const PiRender::Color COLOR_BLUE        {0, 121, 241, 255};
const PiRender::Color COLOR_DARK_BLUE   {0, 82, 172, 255};
const PiRender::Color COLOR_PURPLE      {200, 122, 255, 255};
const PiRender::Color COLOR_VIOLET      {135, 60, 190, 255};
const PiRender::Color COLOR_DARK_PURPLE {112, 31, 126, 255};
const PiRender::Color COLOR_BEIGE       {211, 176, 131, 255};
const PiRender::Color COLOR_BROWN       {127, 106, 79, 255};
const PiRender::Color COLOR_DARK_BROWN  {76, 63, 47, 255};
const PiRender::Color COLOR_WHITE       {255, 255, 255, 255};
const PiRender::Color COLOR_BLACK       {0, 0, 0, 255};
const PiRender::Color COLOR_TRANSPARENT {0, 0, 0, 0};
const PiRender::Color COLOR_MAGENTA     {255, 0, 255, 255};

}