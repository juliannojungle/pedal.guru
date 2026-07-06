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

#include "Window.hpp"

extern "C" {
    #include "LCDRenderer.h"
}

namespace Render {

void Window::Init(int width, int height, std::string title) {
    LCDClear(COLOR_LL(COLOR_BLACK));
}

bool Window::ShouldClose() {
    // return WindowShouldClose();
    return false;
}

void Window::Close() {
    // CloseWindow();
}

void Window::BeginDrawing() {
    // BeginDrawing();
}

void Window::ClearBackground(Render::Color color) {
    LCDClear(COLOR_LL(color));
}

void Window::EndDrawing() {
    // EndDrawing();
}

void Window::DrawCircle(int centerX, int centerY, float radius, Render::Color color) {
    // DrawCircle(centerX, centerY, radius, COLOR_LL(color));
}

void Window::DrawText(std::string text, int posX, int posY, int fontSize, Render::Color color) {
    // DrawText(text.c_str(), posX, posY, fontSize, COLOR_LL(color));
}

void Window::DrawTexture(Render::Texture& texture, int posX, int posY, Render::Color color) {
    // auto driverTexture(TEXTURE2D_TO_RAYLIB(texture));
    // DrawTexture(driverTexture, posX, posY, COLOR_LL(color));
}

}