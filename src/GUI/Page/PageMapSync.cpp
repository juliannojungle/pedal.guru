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
#include "../../Model/MapTile.hpp"
#include <list>

namespace OpenCC {

class PageMapSync : public OpenCC::BasePage {
    private:
        OpenStreetMapAPI mapApi_;
        std::list<OpenCC::MapTile> mapList_;
        int syncedTiles_, totalTiles_;
        PiRender::Texture mapTexture_;
        void ShowTile(std::string filePath);
    public:
        using BasePage::BasePage; // nothing to do here, using parent constructor
        void PreDrawPageContents() override;
        void DrawPageContents() override;
        void PostDrawPageContents() override;
};

void PageMapSync::PreDrawPageContents() {
    mapApi_.ListTilesForArea(mapList_, -22.4701917, -22.1223827, -43.047406, -42.7110277, 16);
    totalTiles_ = mapList_.size();
    syncedTiles_ = 0;
}

void PageMapSync::DrawPageContents() {
    if (mapList_.size() > 0) {
        auto tile = mapList_.front();
        auto filePath = mapApi_.DownloadTile(tile, settings_.mapSyncingBaseUrl);
        ShowTile(filePath);
        mapList_.pop_front();
        syncedTiles_++;
    }

    char progress[100];
    std::sprintf(progress, "%d / %d", syncedTiles_, totalTiles_);
    window_.DrawText(std::string(progress), 50, 125, 20, PiRender::COLOR_BLACK);
}

void PageMapSync::PostDrawPageContents() {
    mapTexture_.UnloadTexture();
}

void PageMapSync::ShowTile(std::string filePath) {
    PiRender::Image mapTile;
    mapTile.LoadImage(filePath);
    mapTexture_.UnloadTexture();
    mapTexture_.LoadTextureFromImage(mapTile);
    mapTile.UnloadImage();
    // 256x256 tile on 240x240 display: -8 padding to center the tile.
    window_.DrawTexture(mapTexture_, -8, -8, PiRender::COLOR_WHITE);
}

}