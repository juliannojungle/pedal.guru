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
#include <stdlib.h>

namespace GUIDriver {

extern "C" {
    #include "GUI_Paint.h"
}

}

namespace PiRender {

class Image {
    private:
        void AllocateImage();
    public:
        uint16_t *data;
        int width;
        int height;
        PiRender::Color color;
        Image() {}
        Image(int width, int height, PiRender::Color color);
        Image(int width, int height): width(width), height(height) {}
        Image(std::string path);
        void LoadImage(std::string path);
        void UnloadImage();
        void ImageDraw(Image image, Rectangle origin, Rectangle destination, Color tint);
        void ImageDrawPixel(int posX, int posY, PiRender::Color color);
};

void Image::AllocateImage() {
    uint32_t imageSize = this->height * this->width * 2;

    if ((this->data = (uint16_t *)malloc(imageSize)) == NULL) {
        printf("Failed to allocate memory...\r\n");
        exit(0);
    }

    uint16_t color(COLOR_TO_PICOCODE(this->color));
    GUIDriver::Paint_NewImage((uint8_t *)this->data, this->width, this->height, ROTATE_0, color);
    GUIDriver::Paint_SetScale(65); // no scale
}

Image::Image(int width, int height, PiRender::Color color) {
    this->width = width;
    this->height = height;
    this->color = color;
}

Image::Image(std::string path) {
    LoadImage(path);
}

void Image::LoadImage(std::string path) {
    AllocateImage();
}

void Image::UnloadImage() {
    free(this->data);
    this->data = NULL;
}

void Image::ImageDraw(Image image, Rectangle origin, Rectangle destination, Color tint) {
    // const unsigned char imageData = image.data;
    // GUIDriver::Paint_DrawImage(&imageData, destination.x, destination.y, origin.width, origin.height);

    // int i, j;
    // for (j = 0; j < origin.height; j++) {
    //     for (i = 0; i < origin.width; i++) {
    //         if (destination.x + i < GUIDriver::Paint.WidthMemory && destination.y + j < GUIDriver::Paint.HeightMemory) //Exceeded part does not display
    //             Paint_SetPixel(
    //                 destination.x + i,
    //                 destination.y + j,
    //                 (*((const unsigned char *)image.data + j * origin.width * 2 + i * 2 + 1)) << 8 | (*(image.data + j * origin.width * 2 + i * 2)));
    //     }
    // }
}

void Image::ImageDrawPixel(int posX, int posY, PiRender::Color color) {
    GUIDriver::Paint_SetPixel(posX, posY, COLOR_TO_PICOCODE(color));
}

}