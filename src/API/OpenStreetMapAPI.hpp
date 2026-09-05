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

#pragma once

#include <list>
#include <string>
#include <cmath>
#include "Time.hpp"
#include "MapTile.hpp"
#include "MapGrid.hpp"

namespace PedalGuru {

class OpenStreetMapAPI {
    private:
        void Swap(int &a, int &b);
    public:
        std::string LatLongZoomToHashPath(double latitude, double longitude, int zoom);
        std::string LatLongZoomToXyzPath(double latitude, double longitude, int zoom);
        int LongitudeToTileX(double longitude, int zoom);
        int LatitudeToTileY(double latitude, int zoom);
        double TilexToLongitude(int x, int zoom);
        double TileyToLatitude(int y, int zoom);
        std::string XyZoomToHashPath(int x, int y, int zoom);
        std::string DownloadTile(PedalGuru::MapTile mapTile, std::string baseUrl);
        void ListTilesForArea(std::list<PedalGuru::MapTile> &mapList,
            double latitudeMin, double latitudeMax, double longitudeMin, double longitudeMax, int zoom);
        void MapGridForCoordinate(PedalGuru::MapGrid &mapGrid, double latitude, double longitude, int zoom);
};

}