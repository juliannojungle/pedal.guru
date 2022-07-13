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
#include <memory>
#include "iPage.hpp"
#include "../../API/OpenStreetMapAPI.cpp"

namespace OpenCC {

class PageMap : public OpenCC::iPage {
    private:
        std::shared_ptr<OpenCC::GUITexture2D> tileTexture;
        void LoadImage();
    public:
        using iPage::iPage; // nothing to do here, using parent constructor
        void DrawPageContents() override;
        void Setup() override;
};

void PageMap::DrawPageContents() {
    drawer_.DrawTexture((*tileTexture), 0, 0, drawer_.COLOR_WHITE);
    drawer_.DrawText(std::string("Map test!"), 50, 125, 20, drawer_.COLOR_BLACK);
}

void PageMap::Setup() {
    drawer_.SetDrawPageContentsMethod([this](){this->DrawPageContents();});
    LoadImage();
}

void PageMap::LoadImage() {
    OpenStreetMapAPI mapApi;
    auto relativePath = mapApi.GetRelativeTilePath(-22.4208101, -42.9791064, 16);
    auto imagePath = relativePath + ".png";
    auto tile = drawer_.LoadImage(imagePath);
    drawer_.ImageCrop(tile, (OpenCC::GUIRectangle){ 0, 0, 240, 240 });
    auto texture = drawer_.LoadTextureFromImage(tile);
    tileTexture = std::make_shared<OpenCC::GUITexture>(texture);
}
}