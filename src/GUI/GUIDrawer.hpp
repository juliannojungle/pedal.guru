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

#include <string>
#include <functional>

namespace OpenCC {

typedef struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} Color;

const OpenCC::Color LIGHTGRAY_   { 200, 200, 200, 255 };
const OpenCC::Color GRAY_        { 130, 130, 130, 255 };
const OpenCC::Color DARKGRAY_    { 80, 80, 80, 255 };
const OpenCC::Color YELLOW_      { 253, 249, 0, 255 };
const OpenCC::Color GOLD_        { 255, 203, 0, 255 };
const OpenCC::Color ORANGE_      { 255, 161, 0, 255 };
const OpenCC::Color PINK_        { 255, 109, 194, 255 };
const OpenCC::Color RED_         { 230, 41, 55, 255 };
const OpenCC::Color MAROON_      { 190, 33, 55, 255 };
const OpenCC::Color GREEN_       { 0, 228, 48, 255 };
const OpenCC::Color LIME_        { 0, 158, 47, 255 };
const OpenCC::Color DARKGREEN_   { 0, 117, 44, 255 };
const OpenCC::Color SKYBLUE_     { 102, 191, 255, 255 };
const OpenCC::Color BLUE_        { 0, 121, 241, 255 };
const OpenCC::Color DARKBLUE_    { 0, 82, 172, 255 };
const OpenCC::Color PURPLE_      { 200, 122, 255, 255 };
const OpenCC::Color VIOLET_      { 135, 60, 190, 255 };
const OpenCC::Color DARKPURPLE_  { 112, 31, 126, 255 };
const OpenCC::Color BEIGE_       { 211, 176, 131, 255 };
const OpenCC::Color BROWN_       { 127, 106, 79, 255 };
const OpenCC::Color DARKBROWN_   { 76, 63, 47, 255 };
const OpenCC::Color WHITE_       { 255, 255, 255, 255 };
const OpenCC::Color BLACK_       { 0, 0, 0, 255 };
const OpenCC::Color TRANSPARENT_ { 0, 0, 0, 0 };
const OpenCC::Color MAGENTA_     { 255, 0, 255, 255 };

class GUIDrawer {
    private:
        std::function<void()>* drawPageContentsMethod_;
    public:
        void Execute();
        void SetDrawPageContentsMethod(std::function<void()>* drawPage) { this->drawPageContentsMethod_ = drawPage; };

        // PARSER METHODS
        void DrawText(std::string text, int posX, int posY, int fontSize, OpenCC::Color color);
};
}