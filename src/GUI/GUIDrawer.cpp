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

#include "GUIDrawer.hpp"

namespace GUIDriver {
#ifdef GUISIMULATOR
    #include "Interface/DesktopSimulator.hpp"
#else
    #include "Interface/Spi240x240.hpp"
#endif
}

namespace OpenCC {

#define COLOR2RAYLIB(color) CLITERAL(GUIDriver::Color){ color.r, color.g, color.b, color.a }
#define IMAGE2RAYLIB(image) CLITERAL(GUIDriver::Image){ image.data, image.width, image.height, image.mipmaps, image.format }
#define TEXTURE2D2RAYLIB(texture) CLITERAL(GUIDriver::Texture2D){ texture.id, texture.width, texture.height, texture.mipmaps, texture.format }
#define RECTANGLE2RAYLIB(rectangle) CLITERAL(GUIDriver::Rectangle){ crop.x, crop.y, crop.width, crop.height }

void GUIDrawer::SetDrawPageContentsMethod(std::function<void()> drawPage) {
    drawPageContentsCallback_ = std::make_shared<OpenCC::Callback>(drawPage);
};

void GUIDrawer::Execute() {
    GUIDriver::InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, std::string("OpenCC Simulator").c_str());
    GUIDriver::SetTargetFPS(FRAME_RATE);
    GUIDriver::HideCursor();

    while (!GUIDriver::WindowShouldClose())
    {
        GUIDriver::BeginDrawing();
        {
            GUIDriver::ClearBackground(GUIDriver::WHITE);
            drawPageContentsCallback_->Method();
        }
        GUIDriver::EndDrawing();
    }
}

void GUIDrawer::DrawText(std::string text, int posX, int posY, int fontSize, OpenCC::GUIColor color) {
    GUIDriver::DrawText(text.c_str(), posX, posY, fontSize, COLOR2RAYLIB(color));
}

OpenCC::GUIImage GUIDrawer::LoadImage(std::string path) {
    GUIDriver::Image image = GUIDriver::LoadImage(path.c_str());
    return OpenCC::GUIImage(image.data, image.width, image.height, image.mipmaps, image.format);
}

OpenCC::GUITexture2D GUIDrawer::LoadTextureFromImage(OpenCC::GUIImage& image) {
    auto driverImage(IMAGE2RAYLIB(image));
    auto texture = GUIDriver::LoadTextureFromImage(driverImage);
    return OpenCC::GUITexture2D(texture.id, texture.width, texture.height, texture.mipmaps, texture.format);
}

void GUIDrawer::DrawTexture(OpenCC::GUITexture2D& texture, int posX, int posY, OpenCC::GUIColor color) {
    auto driverTexture(TEXTURE2D2RAYLIB(texture));
    GUIDriver::DrawTexture(driverTexture, posX, posY, COLOR2RAYLIB(color));
}

void GUIDrawer::ImageCrop(OpenCC::GUIImage& image, OpenCC::GUIRectangle crop) {
    auto driverImage(IMAGE2RAYLIB(image));
    GUIDriver::ImageCrop(&driverImage, RECTANGLE2RAYLIB(crop));
    image.data = driverImage.data;
    image.width = driverImage.width;
    image.height = driverImage.height;
    image.mipmaps = driverImage.mipmaps;
    image.format = driverImage.format;
}

void GUIDrawer::ImageDrawPixel(OpenCC::GUIImage& image, int posX, int posY, OpenCC::GUIColor color) {
    auto driverImage(IMAGE2RAYLIB(image));
    GUIDriver::ImageDrawPixel(&driverImage, posX, posY, COLOR2RAYLIB(color));
}
}