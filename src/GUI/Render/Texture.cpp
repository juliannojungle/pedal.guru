/*
    Pedal.guru is an open-source software
    for cycle computers based on DIY hardware (MCUs like RP2040 and ESP32-S3).
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
#include <string>

extern "C" {
    #include "FileSystem.h"
}

namespace PedalGuru {
UINT8* Texture::Data() {
    return this->cTexture.Data;
}

int Texture::Width() {
    return this->cTexture.Width;
}

int Texture::Height() {
    return this->cTexture.Height;
}

Texture::Texture(int width, int height) {
    this->cTexture = CanvasNewTexture(width, height);
}

void Texture::Release() {
    free(this->cTexture.Data);
    this->cTexture.Data = NULL;
}

void Texture::DrawCircle(int xCenter, int yCenter, int radius, Color color, int lineWidth, bool fillCircle) {
    CanvasDrawCircle(
        this->cTexture,
        xCenter,
        yCenter,
        radius,
        COLOR_LL(color),
        (PixelSize)lineWidth,
        (DrawFillStyle)fillCircle);
}

sFONT* Texture::GetFont(int fontSize) {
    switch (fontSize) {
        case 8: return &Font8; break;
        case 12: return &Font12; break;
        case 16: return &Font16; break;
        case 20: return &Font20; break;
        case 24: return &Font24; break;
        default: return &Font8; break;
    }
}

void Texture::DrawText(std::string text, int x, int y, int fontSize, Color foregroundColor, Color backgroundColor) {
    CanvasDrawText(
        this->cTexture,
        x,
        y,
        text.c_str(),
        GetFont(fontSize),
        COLOR_LL(foregroundColor),
        COLOR_LL(backgroundColor));
}

void Texture::DrawPng(std::string filePath) {
    FIL file;
    if (OpenFile(&file, filePath.c_str())) {
        CanvasDrawPng(cTexture, &file);
        CloseFile(&file);
    }
}

void Texture::DrawPngToArea(std::string filePath, Rectangle source, Point target) {
    FIL file;
    if (OpenFile(&file, filePath.c_str())) {
        CanvasDrawPngToArea(
            cTexture,
            &file,
            source.point.x, source.point.y,
            source.size.width, source.size.height,
            target.x, target.y);
        CloseFile(&file);
    }
}

}