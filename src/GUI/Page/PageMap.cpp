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

#include "BasePage.cpp"
#include "../../API/OpenStreetMapAPI.cpp"

namespace OpenCC {

class PageMap : public OpenCC::BasePage {
    private:
        PiRender::Texture mapTexture_;
    public:
        using BasePage::BasePage; // nothing to do here, using parent constructor
        void PreDrawPageContents() override;
        void DrawPageContents() override;
        void PostDrawPageContents() override;
};

void PageMap::PreDrawPageContents() {
    OpenStreetMapAPI mapApi;
    auto relativePath = mapApi.LatLongZoomToHashPath(-22.4208101, -42.9791064, 16);
    auto imagePath = relativePath + ".png";
    PiRender::Image mapTile;
    mapTile.LoadImage(imagePath);
    mapTexture_.LoadTextureFromImage(mapTile);
    mapTile.UnloadImage();
}

void PageMap::DrawPageContents() {
    window_.DrawTexture(mapTexture_, -16, -16, PiRender::COLOR_WHITE);
    window_.DrawText(std::string("Testando mapas!"), 50, 125, 20, PiRender::COLOR_BLACK);
}

void PageMap::PostDrawPageContents() {
    mapTexture_.UnloadTexture();
}

}