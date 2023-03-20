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
#include "Texture.cpp"

namespace PiRender {

class Window {
    public:
        virtual void Init(int width, int height, std::string title) = 0
        virtual void SetTargetFPS(int frameRate) = 0
        virtual void HideCursor() = 0
        virtual bool ShouldClose() = 0
        virtual void Close() = 0
        virtual void BeginDrawing() = 0
        virtual void ClearBackground(PiRender::Color color) = 0
        virtual void EndDrawing() = 0
        virtual void DrawCircle(int centerX, int centerY, float radius, PiRender::Color color) = 0
        virtual void DrawText(std::string text, int posX, int posY, int fontSize, PiRender::Color color) = 0
        virtual void DrawTexture(PiRender::Texture& texture, int posX, int posY, PiRender::Color color) = 0
};

}