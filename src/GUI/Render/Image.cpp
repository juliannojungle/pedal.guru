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

#include "Image.hpp"

extern "C" {
    #include "Canvas.h"
}

namespace PiRender {

void Image::AllocateImage() {
    UINT32 imageSize = this->height * this->width * 2;

    if ((this->data = (UINT16 *)malloc(imageSize)) == NULL) {
        printf("Failed to allocate memory...\r\n");
        exit(0);
    }

    UINT16 color(COLOR_LL(this->color));
    CanvasNewTexture((UINT8 *)this->data, this->width, this->height, ROTATE_0);
    CanvasSetScale(65); // no scale
}

Image::Image() {}

Image::Image(int width, int height, PiRender::Color color) {
    this->width = width;
    this->height = height;
    this->color = color;
}

Image::Image(int width, int height): width(width), height(height) {}

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
    // Paint_DrawImage(&imageData, destination.x, destination.y, origin.width, origin.height);

    // int i, j;
    // for (j = 0; j < origin.height; j++) {
    //     for (i = 0; i < origin.width; i++) {
    //         if (destination.x + i < Paint.WidthMemory && destination.y + j < Paint.HeightMemory) //Exceeded part does not display
    //             Paint_SetPixel(
    //                 destination.x + i,
    //                 destination.y + j,
    //                 (*((const unsigned char *)image.data + j * origin.width * 2 + i * 2 + 1)) << 8 | (*(image.data + j * origin.width * 2 + i * 2)));
    //     }
    // }
}

void Image::ImageDrawPixel(int posX, int posY, PiRender::Color color) {
    CanvasSetPixel(posX, posY, COLOR_LL(color));
}

}