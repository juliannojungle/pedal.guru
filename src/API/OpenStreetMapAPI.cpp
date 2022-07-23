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
#include <chrono>
#include <thread>
#include "../Model/MapTile.hpp"
#include "../Helper/HTTPHelper.cpp"

/* define the constant since it's not standard c++ and some compilers does not include it */
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

namespace OpenCC {

class OpenStreetMapAPI {
    private:
        HTTPHelper httpHelper_;
        void Swap(int &a, int &b);
    public:
        std::string LatLongZoomToHashPath(double latitude, double longitude, int zoom);
        std::string LatLongZoomToXyzPath(double latitude, double longitude, int zoom);
        int LongitudeToTileX(double longitude, int zoom);
        int LatitudeToTileY(double latitude, int zoom);
        std::string XyZoomToHashPath(int x, int y, int zoom);
        std::string DownloadTile(OpenCC::MapTile mapTile, std::string baseUrl);
        void ListTilesForArea(std::list<OpenCC::MapTile> &mapList,
            double latitudeMin, double latitudeMax, double longitudeMin, double longitudeMax, int zoom);
};

std::string OpenStreetMapAPI::LatLongZoomToHashPath(double latitude, double longitude, int zoom) {
    int tileY = LatitudeToTileY(latitude, zoom);
    int tileX = LongitudeToTileX(longitude, zoom);
    return XyZoomToHashPath(tileX, tileY, zoom);
}

std::string OpenStreetMapAPI::LatLongZoomToXyzPath(double latitude, double longitude, int zoom) {
    int tileY = LatitudeToTileY(latitude, zoom);
    int tileX = LongitudeToTileX(longitude, zoom);
    return std::to_string(zoom)+ "/" + std::to_string(tileX) + "/" + std::to_string(tileY);
}

int OpenStreetMapAPI::LongitudeToTileX(double longitude, int zoom) {
    return (int)(floor((longitude + 180.0) / 360.0 * pow(2, zoom)));
}

int OpenStreetMapAPI::LatitudeToTileY(double latitude, int zoom) {
    return (int)(floor(
        (1.0 - log(tan(latitude * M_PI/180.0) + 1 / cos(latitude * M_PI/180.0)) / M_PI) / 2.0 * pow(2, zoom)
    ));
}

/*
* Reference:
*  https://github.com/openstreetmap/mod_tile/#details-about-mod_tile-tile-serving
*
* Algorithm from OpenStreetMap's `mod_tile` storage structure, to avoid directories with too many files.
* Quoting https://github.com/openstreetmap/mod_tile/blob/master/src/store_file_utils.c:
*     "We attempt to cluster the tiles so that a 16x16 square of tiles will be in a single directory
*     Hash stores our 40 bit result of mixing the 20 bits of the x & y co-ordinates
*     4 bits of x & y are used per byte of output".
*/
std::string OpenStreetMapAPI::XyZoomToHashPath(int x, int y, int zoom)
{
    unsigned char i, hash[5];

    for (i = 0; i < 5; i++) {
        hash[i] = ((x & 0x0f) << 4) | (y & 0x0f);
        x >>= 4;
        y >>= 4;
    }

    char path[1024];
    std::sprintf(path, "%d/%u/%u/%u/%u/%u", zoom, hash[4], hash[3], hash[2], hash[1], hash[0]);
    return std::string(path);
}

void OpenStreetMapAPI::Swap(int &a, int &b) {
    int temp = a;
    a = b;
    b = temp;
}

void OpenStreetMapAPI::ListTilesForArea(std::list<OpenCC::MapTile> &mapList,
    double latitudeMin, double latitudeMax, double longitudeMin, double longitudeMax, int zoom) {
    auto tileYMin = LatitudeToTileY(latitudeMin, zoom);
    auto tileYMax = LatitudeToTileY(latitudeMax, zoom);
    auto tileXMin = LongitudeToTileX(longitudeMin, zoom);
    auto tileXMax = LongitudeToTileX(longitudeMax, zoom);
    int x = 0, y = 0;

    if (tileYMin > tileYMax) Swap(tileYMin, tileYMax);
    if (tileXMin > tileXMax) Swap(tileXMin, tileXMax);

    for (int x = tileXMin; x <= tileXMax; x++)
    {
        for (int y = tileYMin; y <= tileYMax; y++)
        {
            mapList.push_back(OpenCC::MapTile(x, y, zoom));
        }
    }
}

std::string OpenStreetMapAPI::DownloadTile(OpenCC::MapTile mapTile, std::string baseUrl) {
    char tileUrl[1024];
    std::sprintf(tileUrl, "%s/%d/%u/%u.png", baseUrl.c_str(), mapTile.zoom, mapTile.x, mapTile.y);
    auto fileHashPath = XyZoomToHashPath(mapTile.x, mapTile.y, mapTile.zoom) + ".png";

    httpHelper_.DownloadFile(std::string(tileUrl), fileHashPath);

    /*
     * Please be aware of the tile usage policy: https://operations.osmfoundation.org/policies/tiles/
     * Only two requests per second, as OSM API requires low brandwidth usage.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    return fileHashPath;
}

}