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

namespace RAYLIB {

/* The raylib dependency must be the last one, so it doesn't cause building problems due it's dependencies */
extern "C" {
    #include "../../Dependency/raylib/src/raylib.h"
}

}

namespace PiRender {

class Window {
    public:
        void Init(int width, int height, std::string title);
        void SetTargetFPS(int frameRate);
        void HideCursor();
        bool ShouldClose();
        void Close();
        void BeginDrawing();
        void ClearBackground(PiRender::Color color);
        void EndDrawing();
        void DrawCircle(int centerX, int centerY, float radius, PiRender::Color color);
        void DrawText(std::string text, int posX, int posY, int fontSize, PiRender::Color color);
        void DrawTexture(PiRender::Texture& texture, int posX, int posY, PiRender::Color color);
};

void Window::Init(int width, int height, std::string title) {
    RAYLIB::InitWindow(width, height, title.c_str());
}

void Window::SetTargetFPS(int frameRate) {
    RAYLIB::SetTargetFPS(frameRate);
}

void Window::HideCursor() {
    RAYLIB::HideCursor();
}

bool Window::ShouldClose() {
    return RAYLIB::WindowShouldClose();
}

void Window::Close() {
    RAYLIB::CloseWindow();
}

void Window::BeginDrawing() {
    RAYLIB::BeginDrawing();
}

void Window::ClearBackground(PiRender::Color color) {
    RAYLIB::ClearBackground(COLOR_TO_RAYLIB(color));
}

void Window::EndDrawing() {
    RAYLIB::EndDrawing();
}

void Window::DrawCircle(int centerX, int centerY, float radius, PiRender::Color color) {
    RAYLIB::DrawCircle(centerX, centerY, radius, COLOR_TO_RAYLIB(color));
}

void Window::DrawText(std::string text, int posX, int posY, int fontSize, PiRender::Color color) {
    RAYLIB::DrawText(text.c_str(), posX, posY, fontSize, COLOR_TO_RAYLIB(color));
}

void Window::DrawTexture(PiRender::Texture& texture, int posX, int posY, PiRender::Color color) {
    auto driverTexture(TEXTURE2D_TO_RAYLIB(texture));
    RAYLIB::DrawTexture(driverTexture, posX, posY, COLOR_TO_RAYLIB(color));
}

}