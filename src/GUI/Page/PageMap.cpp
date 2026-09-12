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

#include "PageMap.hpp"
#include "Area.hpp"
#include "DataManager.hpp"
#include "GPSFixData.hpp"
#include <algorithm>

extern "C" {
    #include "HAL.h"
}

namespace PedalGuru {

void PageMap::PreDrawPageContents() {
}

void PageMap::InputGpsLocation(double &latitude, double &longitude, bool &fixed) {
    GPSFixData gpsFixData;
    DataManager::GetInstance()->Pop(gpsFixData);
    latitude = gpsFixData.latitude;
    longitude = gpsFixData.longitude;
    fixed = gpsFixData.fixQuality > 0;
}

void PageMap::LoadGridTexture() {
    Rectangle sourceTile({0, 0}, {256, 256});
    Point gridTarget({0, 0});

    for (int row = 0; row < 2; row++)
    {
        for (int col = 0; col < 2; col++)
        {
            // Tile position in the virtual grid
            int tileX = col * 256;
            int tileY = row * 256;

            // Intersection of visible area (240x240) with this tile (256x256)
            sourceTile.point.x = std::max(mapGrid_.offsetX - tileX, 0);
            sourceTile.point.y = std::max(mapGrid_.offsetY - tileY, 0);
            sourceTile.size.width = std::min(tileX + 256, mapGrid_.offsetX + 240) - std::max(tileX, mapGrid_.offsetX);
            sourceTile.size.height = std::min(tileY + 256, mapGrid_.offsetY + 240) - std::max(tileY, mapGrid_.offsetY);

            gridTarget.x = std::max(tileX - mapGrid_.offsetX, 0);
            gridTarget.y = std::max(tileY - mapGrid_.offsetY, 0);

            if (sourceTile.size.width <= 0 || sourceTile.size.height <= 0) continue;

            auto imagePath = mapApi_.XyZoomToHashPath(
                mapGrid_.tiles[row][col].x,
                mapGrid_.tiles[row][col].y,
                mapGrid_.tiles[row][col].zoom) + ".png";

            mapTexture_.DrawPngToArea(
                imagePath,
                sourceTile,
                gridTarget);
        }
    }
}

void PageMap::DrawPageContents() {
    double latitude, longitude;
    bool fixed;
    InputGpsLocation(latitude, longitude, fixed);

    if ((previousLatitude != latitude) || (previousLongitude != longitude)) {
        previousLatitude = latitude;
        previousLongitude = longitude;
        mapApi_.MapGridForCoordinate(mapGrid_, latitude, longitude, 16);
        LoadGridTexture();
    }

    mapTexture_.DrawCircle({120, 120}, 4, (fixed ? COLOR_GREEN : COLOR_ORANGE), 1, false);
    window_.DrawTexture(mapTexture_);

    // We only get gps readings once per second.
    Delay(1000);
}

void PageMap::PostDrawPageContents() {
    mapTexture_.Release();
}

}