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
#include <thread> // multithread
#include <iostream> // cout
#include <iomanip> // setprecision
#include <fstream> // file stream
#include <jsoncpp/json/json.h>

namespace OpenCC {

class GPS : public iSensor {
    private:
        const std::string GPS_FIX = "GGA,"; // $GNGGA, $GPGGA,
        int startingPos = 3;
        OpenCC::TextHelper textHelper_;
        void LastGpsLocation(double &latitude, double &longitude);
        void OutputGpsLocation(double &latitude, double &longitude, bool fixed);
        void GetData();
    public:
        void Enable() override;
        void Disable() override;
};

void GPS::Enable() {
    std::thread task([this](){ this->GetData(); });
    task.detach();
    this->enabled_ = true;
}

void GPS::Disable() {
    this->enabled_ = false;
}

void GPS::LastGpsLocation(double &latitude, double &longitude) {
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

void GPS::OutputGpsLocation(double &latitude, double &longitude, bool fixed) {
    std::ofstream file("gps.json", std::ofstream::trunc | std::ofstream::binary);

    if (!file) return;

    file << "{\"latitude\":" << std::fixed << std::setprecision(6) << latitude
        << ",\"longitude\":" << std::fixed << std::setprecision(6) << longitude
        << ",\"fixed\":" << (fixed ? "true" : "false")
        << "}\n";
    file.close();
}

void GPS::GetData() {
    std::ifstream uart;
    std::ios_base::iostate exceptionMask = uart.exceptions() | std::ios::failbit;
    uart.exceptions(exceptionMask);

    try {
        uart.open("/dev/serial0", std::ifstream::in);
    } catch (std::ios_base::failure &error) {
        std::cout << error.what();
        this->enabled_ = false;
        return;
    }

    std::string serial_rx;
    OpenCC::GPSFixData gpsFixData;
    double lastLatitude, lastLongitude;
    LastGpsLocation(lastLatitude, lastLongitude);

    while (this->enabled_ && uart.is_open()) {
        std::getline(uart, serial_rx);

        if (serial_rx.rfind(GPS_FIX, startingPos) == startingPos) {
            gpsFixData.set(serial_rx);

            if (gpsFixData.fixQuality > 0) {
                // normalize coordinates
                gpsFixData.latitude /= 100;
                gpsFixData.longitude /= 100;
                if (gpsFixData.latitudeCardinal == 'S') gpsFixData.latitude *= -1;
                if (gpsFixData.longitudeCardinal == 'W') gpsFixData.longitude *= -1;

                std::cout << "FIXED: " << gpsFixData.satellitesCount
                    << " LATITUDE: " << std::fixed << std::setprecision(6) << gpsFixData.latitude
                    << " LONGITUDE: " << std::fixed << std::setprecision(6) << gpsFixData.longitude
                    << "\n";

                lastLatitude = gpsFixData.latitude;
                lastLongitude = gpsFixData.longitude;
                OutputGpsLocation(gpsFixData.latitude, gpsFixData.longitude, true);
            } else {
                std::cout << "NOT FIXED: " << serial_rx << "\n";
                OutputGpsLocation(lastLatitude, lastLongitude, false);
            }
        }
    }

    uart.close();
}

}