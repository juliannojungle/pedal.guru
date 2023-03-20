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

#include "Image.cpp"

namespace GUIDriver {

extern "C" {
    #include "GUI_Paint.h"
}

}

namespace PiRender {

// #define TEXTURE2D_TO_RAYLIB(texture) CLITERAL(GUIDriver::Texture2D) \
//     { texture.id, texture.width, texture.height, texture.mipmaps, texture.format }

class Texture {
    public:
        unsigned int id;        // OpenGL texture id
        int width;              // Texture base width
        int height;             // Texture base height
        int mipmaps;            // Mipmap levels, 1 by default
        int format;             // Data format (PixelFormat type)
        Texture() {}
        Texture(unsigned int id, int width, int height, int mipmaps, int format)
            : id(id), width(width), height(height), mipmaps(mipmaps), format(format) {}
        void LoadTextureFromImage(PiRender::Image& image);
        void UnloadTexture();
};

// Texture2D type, same as Texture
// typedef Texture Texture2D;

// TextureCubemap type, actually, same as Texture
// typedef Texture TextureCubemap;

void Texture::LoadTextureFromImage(PiRender::Image& image) {
    // auto driverImage(IMAGE_TO_RAYLIB(image));
    // auto driverTexture = GUIDriver::LoadTextureFromImage(driverImage);
    // this->id = driverTexture.id;
    // this->width = driverTexture.width;
    // this->height = driverTexture.height;
    // this->mipmaps = driverTexture.mipmaps;
    // this->format = driverTexture.format;
}

void Texture::UnloadTexture() {
    // auto driverTexture(TEXTURE2D_TO_RAYLIB((*this)));
    // GUIDriver::UnloadTexture(driverTexture);
}

}