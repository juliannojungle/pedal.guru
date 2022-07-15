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

#include "iPage.hpp"
#include "../GUIDrawer.hpp"
#include "../../API/OpenStreetMapAPI.cpp"

namespace OpenCC {

class PageMap : public OpenCC::iPage {
    private:
        PiRender::Texture tileTexture;
    public:
        using iPage::iPage; // nothing to do here, using parent constructor
        void PreDrawPageContents() override;
        void DrawPageContents() override;
        void PostDrawPageContents() override;
        void Setup() override;
};

void PageMap::PreDrawPageContents() {
    OpenStreetMapAPI mapApi;
    auto relativePath = mapApi.GetRelativeTilePath(-22.4208101, -42.9791064, 16);
    auto imagePath = relativePath + ".png";
    PiRender::Image mapTile;
    mapTile.LoadImage(imagePath);
    tileTexture.LoadTextureFromImage(mapTile);
}

void PageMap::DrawPageContents() {
    PiRender::Window window;
    window.DrawTexture(tileTexture, 0, 0, PiRender::COLOR_WHITE);
    window.DrawText(std::string("Testando mapas!"), 50, 125, 20, PiRender::COLOR_BLACK);
}

void PageMap::PostDrawPageContents() {
}

void PageMap::Setup() {
    drawer_.SetPageContentsPreDrawMethod([this](){this->PreDrawPageContents();});
    drawer_.SetPageContentsDrawMethod([this](){this->DrawPageContents();});
    drawer_.SetPageContentsPostDrawMethod([this](){this->PostDrawPageContents();});
}

}