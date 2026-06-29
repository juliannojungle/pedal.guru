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

namespace GUI_LL {

extern "C" {
    // #include "DEV_Config.h"
    #include "Canvas.h"
}

}

namespace PiRender {

class Texture {
    private:
        GUI_LL::UINT16 *data;
        void AllocateTexture();
    public:
        int width;
        int height;
        Texture() {}
        Texture(int width, int height): width(width), height(height) {}
        void LoadTextureFromImage(PiRender::Image& image);
        void UnloadTexture();
};

void Texture::AllocateTexture() {
    GUI_LL::UINT32 textureSize = this->height * this->width * 2;

    if ((this->data = (GUI_LL::UINT16 *)malloc(textureSize)) == NULL) {
        printf("Failed to allocate memory...\r\n");
        exit(0);
    }

    GUI_LL::CanvasNewTexture((GUI_LL::UINT8 *)this->data, this->width, this->height, GUI_LL::ROTATE_0);
    GUI_LL::CanvasSetScale(65); // no scale
}

void Texture::LoadTextureFromImage(PiRender::Image& image) {
    this->height = image.height;
    this->width = image.width;
    AllocateTexture();
    this->data = image.data;
}

void Texture::UnloadTexture() {
    free(this->data);
    this->data = NULL;
}

}