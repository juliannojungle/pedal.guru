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

#include "iSensor.hpp"
#include "../Model/GPSFixData.hpp"
#include "../DataManager.cpp"
#include <iostream> // cout
#include <iomanip> // setprecision
#include <fstream> // file stream

namespace OpenCC {

class GPS : public iSensor {
    private:
        const std::string GPS_FIX = "GGA,"; // $GNGGA, $GPGGA,
        int startingPos = 3;
        OpenCC::TextHelper textHelper_;
        void GetGpsFixData(OpenCC::GPSFixData &gpsFixData);
        void SetGpsFixData(OpenCC::GPSFixData &gpsFixData);
    public:
        void Enable() override;
        void Disable() override;
        void GetData() override;
};

void GPS::Enable() {
    this->enabled_ = true;
}

void GPS::Disable() {
    this->enabled_ = false;
}

void GPS::GetGpsFixData(OpenCC::GPSFixData &gpsFixData) {
    OpenCC::DataManager::GetInstance()->LastOrDefault(gpsFixData);
}

void GPS::SetGpsFixData(OpenCC::GPSFixData &gpsFixData) {
    OpenCC::DataManager::GetInstance()->Push(gpsFixData);
}

void GPS::GetData() {
    if (!enabled_) return;

    // std::ifstream uart;
    // std::ios_base::iostate exceptionMask = uart.exceptions() | std::ios::failbit;
    // uart.exceptions(exceptionMask);

    // try {
    //     uart.open("/dev/serial0", std::ifstream::in);
    // } catch (std::ios_base::failure &error) {
    //     std::cout << error.what();
    //     this->enabled_ = false;
    //     return;
    // }

    std::string serial_rx;
    OpenCC::GPSFixData gpsFixData;
    GetGpsFixData(gpsFixData);
    double lastLatitude = gpsFixData.latitude;
    double lastLongitude = gpsFixData.longitude;

    // while (this->enabled_ && uart.is_open()) {
    //     std::getline(uart, serial_rx);
    serial_rx = "$GPGGA,164353.000,2225.2115,S,04258.7947,W,2,4,2.95,971.3,M,-5.7,M,,*49"; //DEBUG TEST

    if (serial_rx.rfind(GPS_FIX, startingPos) == startingPos) {
        gpsFixData.set(serial_rx);

        // normalize coordinates
        gpsFixData.latitude /= 100;
        gpsFixData.longitude /= 100;
        if (gpsFixData.latitudeCardinal == 'S') gpsFixData.latitude *= -1;
        if (gpsFixData.longitudeCardinal == 'W') gpsFixData.longitude *= -1;

        lastLatitude = gpsFixData.latitude;
        lastLongitude = gpsFixData.longitude;
        SetGpsFixData(gpsFixData);
    }
    // }

    // uart.close();
}

}