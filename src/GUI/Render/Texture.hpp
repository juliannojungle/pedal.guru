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

#pragma once

#include <string>
#include "Area.hpp"
#include "Color.hpp"
#include "fonts.h"

extern "C" {
    #include "Types.h"
    #include "Canvas.h"
}

using CTexture = ::Texture;

namespace PedalGuru {

class Texture {
    private:
        CTexture cTexture;
        sFONT* GetFont(int fontSize);
    public:
        UINT8* Data();
        int Width();
        int Height();
        Texture(int width, int height);
        void Release();
        void DrawCircle(Point centerPoint, int radius, Color color, int lineWidth, bool fillCircle);
        void DrawText(std::string text, Point target, int fontSize, Color foregroundColor, Color backgroundColor);
        void DrawCurvedText(std::string text, Point centerPoint, int radius, int startAngle,
            TextOrientation orientation, int fontSize, Color foregroundColor, Color backgroundColor);
        void DrawPng(std::string filePath);
        void DrawPngToArea(std::string filePath, Rectangle source, Point target);
        void DrawQRCode(std::string url, Point target);
};

}