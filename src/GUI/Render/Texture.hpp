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

#include "Image.hpp"
#include "fonts.h"

extern "C" {
    #include "Types.h"
}

namespace PedalGuru {

class Texture {
    private:
    void Allocate();
    public:
        UINT16 *data;
        int width;
        int height;
        Texture() {}
        Texture(int width, int height): width(width), height(height) {}
        void LoadTextureFromImage(Image& image);
        void Release();
        void DrawCircle(int xCenter, int yCenter, int radius, Color color, int lineWidth, bool fillCircle);
        void DrawText(std::string text, int x, int y, sFONT* fontSize, Color foregroundColor, Color backgroundColor);
};

}