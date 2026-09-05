/*
    Pedal.guru is an open-source software
    for cycle computers based on DIY hardware (MCUs like RP2040 and ESP32-S3).
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

#include "PageMapSync.hpp"
#include "Color.hpp"

namespace PedalGuru {

void PageMapSync::PreDrawPageContents() {
    mapApi_.ListTilesForArea(mapList_, -22.4701917, -22.1223827, -43.047406, -42.7110277, 16);
    totalTiles_ = mapList_.size();
    syncedTiles_ = 0;
}

void PageMapSync::DrawPageContents() {
    if (mapList_.size() > 0) {
        auto tile = mapList_.front();
        auto filePath = mapApi_.DownloadTile(tile, settings_.mapSyncingBaseUrl);
        // 256x256 tile on 240x240 display: 8 padding to center the tile.
        mapTexture_.DrawPngToArea(filePath, {{8, 8}, {240, 240}}, {0, 0});
        mapList_.pop_front();
        syncedTiles_++;
    }

    char progress[(totalTiles_ * 2) + 3];
    std::sprintf(progress, "%d / %d", syncedTiles_, totalTiles_);
    mapTexture_.DrawText(std::string(progress), 50, 125, 20, COLOR_BLACK, COLOR_TRANSPARENT);
    window_.DrawTexture(mapTexture_);
}

void PageMapSync::PostDrawPageContents() {
    mapTexture_.Release();
}

}