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
#include "../Helper/TextHelper.cpp"

namespace OpenCC {

class GPSFixData {
    public:
        double UTCTime;
        double latitude;
        char latitudeCardinal;
        double longitude;
        char longitudeCardinal;
        int fixQuality;
        int satellitesCount;
        double horizontalAccuracy;
        double altitude;
        char altitudeUnit;
        char *geoidalSeparation;
        char geoidalSeparationUnit;
        double differentialGPSLastUpdate;
        char *differentialGPSStationId;
        char *checksum;
        GPSFixData() :
            UTCTime(0),
            latitude(0),
            latitudeCardinal('\0'),
            longitude(0),
            longitudeCardinal('\0'),
            fixQuality(0),
            satellitesCount(0),
            horizontalAccuracy(0),
            altitude(0),
            altitudeUnit('\0'),
            geoidalSeparation(nullptr),
            geoidalSeparationUnit('\0'),
            differentialGPSLastUpdate(0),
            differentialGPSStationId(nullptr),
            checksum(nullptr) {}
        void set(std::string serial_rx);
};

void GPSFixData::set(std::string serial_rx) {
    char data[16][16];
    TextHelper::Tokenize(serial_rx, ',', '*', data);

    this->UTCTime = atof(data[1]);
    this->latitude = atof(data[2]);
    this->latitudeCardinal = data[3][0];
    this->longitude = atof(data[4]);
    this->longitudeCardinal = data[5][0];
    this->fixQuality = atoi(data[6]);
    this->satellitesCount = atoi(data[7]);
    this->horizontalAccuracy = atof(data[8]);
    this->altitude = atof(data[9]);
    this->altitudeUnit = data[10][0];
    this->geoidalSeparation = data[11];
    this->geoidalSeparationUnit = data[12][0];
    this->differentialGPSLastUpdate = atof(data[13]);
    this->differentialGPSStationId = data[14];
    this->checksum = data[15];
}

}