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
#include <cmath>

namespace OpenCC {

class OpenStreetMapAPI {
    private:
        int LongitudeToTileX(double longitude, int zoom);
        int LatitudeToTileY(double latitude, int zoom);
    public:
        std::string GetRelativeTilePath(double latitude, double longitude, int zoom);
};

int OpenStreetMapAPI::LongitudeToTileX(double longitude, int zoom) {
    return (int)(floor((longitude + 180.0) / 360.0 * pow(2, zoom)));
   // return (Math.floor((longitude+180)/360*Math.pow(2,zoom)));
}

int OpenStreetMapAPI::LatitudeToTileY(double latitude, int zoom) {
    return (int)(floor((1.0 - log(tan(latitude * M_PI/180.0) + 1 / cos(latitude * M_PI/180.0)) / M_PI) / 2.0 * pow(2, zoom)));
    // return (Math.floor((1-Math.log(Math.tan(lat*Math.PI/180) + 1/Math.cos(lat*Math.PI/180))/Math.PI)/2 *Math.pow(2,zoom)));
}

std::string OpenStreetMapAPI::GetRelativeTilePath(double latitude, double longitude, int zoom) {
    double absLatitude = abs(latitude);
    double absLongitude = abs(longitude);
    int tileY = LatitudeToTileY(latitude, zoom);
    int tileX = LongitudeToTileX(longitude, zoom);
    return  std::to_string(zoom)+ "/" + std::to_string(tileX) + "/" + std::to_string(tileY);
}

}