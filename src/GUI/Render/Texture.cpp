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

#include "Texture.hpp"
#include "Canvas.h"
#include "fonts.h"

namespace PedalGuru {

void Texture::Allocate() {
    UINT32 textureSize = this->height * this->width * 2;

    if ((this->data = (UINT16 *)malloc(textureSize)) == NULL) {
        printf("Failed to allocate memory...\r\n");
        exit(EXIT_FAILURE);
    }

    CanvasNewTexture((UINT8 *)this->data, this->width, this->height, ROTATE_0);
}

void Texture::LoadTextureFromImage(Image& image) {
    this->height = image.height;
    this->width = image.width;
    Allocate();
    this->data = image.data;
}

void Texture::Release() {
    free(this->data);
    this->data = NULL;
}

void Texture::DrawCircle(int xCenter, int yCenter, int radius, Color color, int lineWidth, bool fillCircle) {
    CanvasDrawCircle(
        xCenter,
        yCenter,
        radius,
        COLOR_LL(color),
        (PixelSize)lineWidth,
        (DrawFillStyle)fillCircle);
}

void Texture::DrawText(std::string text, int x, int y, sFONT* fontSize, Color foregroundColor, Color backgroundColor) {
    CanvasDrawText(
        x,
        y,
        text.c_str(),
        fontSize,
        COLOR_LL(foregroundColor),
        COLOR_LL(backgroundColor));
}

}