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
#include "../../Model/MapGrid.hpp"

/*MOCK*/
#include <jsoncpp/json/json.h>
#include <fstream>
/*MOCK*/

namespace OpenCC {

class PageMap : public OpenCC::BasePage {
    private:
        OpenCC::OpenStreetMapAPI mapApi_;
        PiRender::Texture mapTexture_;
        OpenCC::MapGrid mapGrid_;
        double previousLatitude, previousLongitude;
        void LoadGridImage();
    public:
        using BasePage::BasePage; // nothing to do here, using parent constructor
        void PreDrawPageContents() override;
        void DrawPageContents() override;
        void PostDrawPageContents() override;
};

void PageMap::PreDrawPageContents() {
}

/*MOCK*/
void MockGpsLocation(double &latitude, double &longitude) {
    std::ifstream file("gps.json", std::ifstream::binary);

    if (!file) return;

    Json::Reader reader;
    Json::Value gpsData;

    if (reader.parse(file, gpsData)) {
        latitude = std::stod(gpsData["latitude"].asString());
        longitude = std::stod(gpsData["longitude"].asString());
    }

    file.close();
}
/*MOCK*/

void PageMap::LoadGridImage() {
    PiRender::Image gridImage(512, 512, PiRender::COLOR_BLUE);
    int latitude, longitude;
    PiRender::Rectangle tileRectangle(0, 0, 256, 256);
    PiRender::Rectangle gridRectangle(0, 0, 256, 256);
    for (int latitude = 0; latitude < 2; latitude++)
    {
        for (int longitude = 0; longitude < 2; longitude++)
        {
            auto imagePath = mapApi_.XyZoomToHashPath(
                mapGrid_.tiles[latitude][longitude].x,
                mapGrid_.tiles[latitude][longitude].y,
                mapGrid_.tiles[latitude][longitude].zoom) + ".png";
            PiRender::Image tileImage(imagePath);
            gridRectangle.x = longitude * 256;
            gridRectangle.y = latitude * 256;
            gridImage.ImageDraw(tileImage, tileRectangle, gridRectangle, PiRender::COLOR_WHITE);
            tileImage.UnloadImage();
        }
    }

    mapTexture_.UnloadTexture();
    mapTexture_.LoadTextureFromImage(gridImage);
    gridImage.UnloadImage();
}

void PageMap::DrawPageContents() {
    double latitude, longitude;
    MockGpsLocation(latitude, longitude);

    if ((previousLatitude != latitude) || (previousLongitude != longitude)) {
        previousLatitude = latitude;
        previousLongitude = longitude;
        mapApi_.MapGridForCoordinate(mapGrid_, latitude, longitude, 16);
        LoadGridImage();
    }

    window_.DrawTexture(mapTexture_, mapGrid_.offsetX, mapGrid_.offsetY, PiRender::COLOR_WHITE);
    window_.DrawCircle(120, 120, 4, PiRender::COLOR_ORANGE);
}

void PageMap::PostDrawPageContents() {
    mapTexture_.UnloadTexture();
}

}