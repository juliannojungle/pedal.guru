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
#include "Color.cpp"
#include "Rectangle.cpp"

namespace PiRender {

class Image {
    public:
        void *data;
        int width;
        int height;
        int mipmaps; // Mipmap levels, 1 by default
        int format;  // Data format (PixelFormat type)
        Image() {}
        virtual Image(int width, int height, PiRender::Color color) = 0
        virtual Image(void *data, int width, int height, int mipmaps, int format)
            : data(data), width(width), height(height), mipmaps(mipmaps), format(format) {}
        virtual Image(std::string path) = 0
        virtual void LoadImage(std::string path) = 0
        virtual void UnloadImage() = 0
        virtual void ImageDraw(Image image, Rectangle origin, Rectangle destination, Color tint) = 0
        virtual void ImageDrawPixel(int posX, int posY, PiRender::Color color) = 0
};

}