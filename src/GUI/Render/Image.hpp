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

#include <string>
#include "Color.hpp"
#include "Rectangle.hpp"
#include <stdlib.h>

extern "C" {
    #include "Types.h"
}

namespace Render {

class Image {
    private:
        void AllocateImage();
    public:
        UINT16 *data;
        int width;
        int height;
        Render::Color color;
        Image();
        Image(int width, int height, Render::Color color);
        Image(int width, int height);
        Image(std::string path);
        void LoadImage(std::string path);
        void UnloadImage();
        void ImageDraw(Image image, Rectangle origin, Rectangle destination, Color tint);
        void ImageDrawPixel(int posX, int posY, Render::Color color);
};

}