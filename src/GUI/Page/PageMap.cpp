/*
    Pedal.guru is an open-source software
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

#include "PageMap.hpp"
#include "DataManager.hpp"
#include "GPSFixData.hpp"

extern "C" {
    #include "Canvas.h"
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

void PageMap::LoadGridImage() {
    Image gridImage(512, 512, COLOR_BLUE);
    int latitude, longitude;
    Rectangle tileRectangle(0, 0, 256, 256);
    Rectangle gridRectangle(0, 0, 256, 256);

    for (int latitude = 0; latitude < 2; latitude++)
    {
        for (int longitude = 0; longitude < 2; longitude++)
        {
            auto imagePath = mapApi_.XyZoomToHashPath(
                mapGrid_.tiles[latitude][longitude].x,
                mapGrid_.tiles[latitude][longitude].y,
                mapGrid_.tiles[latitude][longitude].zoom) + ".png";
            Image tileImage(imagePath);
            gridRectangle.x = longitude * 256;
            gridRectangle.y = latitude * 256;
            gridImage.ImageDraw(tileImage, tileRectangle, gridRectangle, COLOR_WHITE);
            tileImage.UnloadImage();
        }
    }

    mapTexture_.UnloadTexture();
    mapTexture_.LoadTextureFromImage(gridImage);
    gridImage.UnloadImage();
}

void PageMap::DrawPageContents() {
    double latitude, longitude;
    bool fixed;
    InputGpsLocation(latitude, longitude, fixed);

    if ((previousLatitude != latitude) || (previousLongitude != longitude)) {
        previousLatitude = latitude;
        previousLongitude = longitude;
        mapApi_.MapGridForCoordinate(mapGrid_, latitude, longitude, 16);
        LoadGridImage();
    }

    // mapTexture_.DrawTexture(mapTexture_, mapGrid_.offsetX, mapGrid_.offsetY, COLOR_WHITE);
    mapTexture_.DrawCircle(120, 120, 4, (fixed ? COLOR_GREEN : COLOR_ORANGE), 1, false);
    window_.DrawTexture(mapTexture_);

    // We only get gps readings once per second.
    Time::Delay(1000);
}

void PageMap::PostDrawPageContents() {
    mapTexture_.UnloadTexture();
}

}