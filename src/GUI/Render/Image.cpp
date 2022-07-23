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

namespace GUIDriver {
/* The raylib dependency must be the last one, so it doesn't cause building problems due it's dependencies */
#ifdef USE_RAYLIB
extern "C" {
    #include "../../Dependency/raylib/src/raylib.h"
}
#endif
}

namespace PiRender {

#define IMAGE_TO_RAYLIB(image) CLITERAL(GUIDriver::Image) \
    { image.data, image.width, image.height, image.mipmaps, image.format }

class Image {
    public:
        void *data;
        int width;
        int height;
        int mipmaps; // Mipmap levels, 1 by default
        int format;  // Data format (PixelFormat type)
        Image() {}
        Image(void *data, int width, int height, int mipmaps, int format)
            : data(data), width(width), height(height), mipmaps(mipmaps), format(format) {}
        void LoadImage(std::string path);
        void UnloadImage();
        void ImageCrop(PiRender::Rectangle crop);
        void ImageDrawPixel(int posX, int posY, PiRender::Color color);
};

void Image::LoadImage(std::string path) {
    auto driverImage = GUIDriver::LoadImage(path.c_str());
    this->data = driverImage.data;
    this->width = driverImage.width;
    this->height = driverImage.height;
    this->mipmaps = driverImage.mipmaps;
    this->format = driverImage.format;
}

void Image::UnloadImage() {
    auto driverImage(IMAGE_TO_RAYLIB((*this)));
    GUIDriver::UnloadImage(driverImage);
}

void Image::ImageCrop(PiRender::Rectangle crop) {
    auto driverImage(IMAGE_TO_RAYLIB((*this)));
    GUIDriver::ImageCrop(&driverImage, RECTANGLE_TO_RAYLIB(crop));
    this->data = driverImage.data;
    this->width = driverImage.width;
    this->height = driverImage.height;
    this->mipmaps = driverImage.mipmaps;
    this->format = driverImage.format;
}

void Image::ImageDrawPixel(int posX, int posY, PiRender::Color color) {
    auto driverImage(IMAGE_TO_RAYLIB((*this)));
    GUIDriver::ImageDrawPixel(&driverImage, posX, posY, COLOR_TO_RAYLIB(color));
    this->data = driverImage.data;
    this->width = driverImage.width;
    this->height = driverImage.height;
    this->mipmaps = driverImage.mipmaps;
    this->format = driverImage.format;
}

}