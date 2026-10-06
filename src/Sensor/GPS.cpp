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

#include "GPS.hpp"
#include "DataManager.hpp"
#include "TextHelper.hpp"
extern "C" {
    #include "HAL.h"
}

namespace PedalGuru {

void GPS::Enable() {
    UARTInit(GPS_UART, GPS_UART_BAUDRATE, GPS_UART_TX_PIN, GPS_UART_RX_PIN);

// #ifdef L96GPS
    /* Configuration commands for the Quectel L96 module. */
    UARTPuts(GPS_UART, "$PMTK353,1,1,1,0,0*2A\0"); // enable GPS, GLONASS and GALILEO satellite system.
    UARTPuts(GPS_UART, "$PMTK869,1,1*35\0"); // enable AGPS (EASY function).
    UARTPuts(GPS_UART, "$PMTK886,1*29\0"); // enable fitness mode.
    //// UARTPuts(GPS_UART, "$PMTK886,0*28\0"); // enable normal mode.
// #endif

    this->enabled_ = true;
}

void GPS::Disable() {
    UARTDeinit(GPS_UART);
    this->enabled_ = false;
}

void GPS::ParseGGA(std::string serial_rx, GPSFixData &gpsFixData) {
    char data[16][16];
    TextHelper::Tokenize(serial_rx, ',', '*', data);

    gpsFixData.UTCTime = atof(data[1]);
    gpsFixData.latitude = atof(data[2]);
    gpsFixData.latitudeCardinal = data[3][0];
    gpsFixData.longitude = atof(data[4]);
    gpsFixData.longitudeCardinal = data[5][0];
    gpsFixData.fixQuality = atoi(data[6]);
    gpsFixData.satellitesCount = atoi(data[7]);
    gpsFixData.horizontalAccuracy = atof(data[8]);
    gpsFixData.altitude = atof(data[9]);
    gpsFixData.altitudeUnit = data[10][0];
    gpsFixData.geoidalSeparation = data[11];
    gpsFixData.geoidalSeparationUnit = data[12][0];
    gpsFixData.differentialGPSLastUpdate = atof(data[13]);
    gpsFixData.differentialGPSStationId = data[14];
    gpsFixData.checksum = data[15];
}

double GPS::NMEA2DecimalDegrees(double coordinate, char cardinal) {
    double degrees = (int)(coordinate / 100);
    double decimalDegrees = degrees + (coordinate - degrees * 100) / 60.0;

    if (cardinal == 'S' || cardinal == 'W') decimalDegrees *= -1;

    return decimalDegrees;
}

bool GPS::IsGpsFixData(std::string &info) {
    return info.rfind(GPS_FIX_SUFIX, startingPos) == startingPos;
}

void GPS::UartGetLine(std::string &line) {
    char singleChar = '\0';
    line = "";

    while (true) {
        if (!UARTIsReadable(GPS_UART)) {
            Delay(1);
            continue;
        }

        singleChar = UARTGetChar(GPS_UART);

        if (singleChar == '\0' || singleChar == '\n') break;
        if (singleChar == '\r') continue;

        line += singleChar;
    }
}

void GPS::GetData() {
    if (!enabled_ || !UARTIsEnabled(GPS_UART)) return;

    std::string serial_rx = "";
    int attempts = 0;

    while (!IsGpsFixData(serial_rx) && (attempts < 50)) {
        attempts++;
        UartGetLine(serial_rx);
    }

    if (!IsGpsFixData(serial_rx)) return;

    GPSFixData gpsFixData;
    ParseGGA(serial_rx, gpsFixData);

    if (gpsFixData.fixQuality > 0) {
        DataManager::GetInstance()->SetLastGpsFixData(serial_rx);
    } else {
        serial_rx = DataManager::GetInstance()->GetLastGpsFixData();
        ParseGGA(serial_rx, gpsFixData);
        gpsFixData.fixQuality = 0;
    }

    gpsFixData.latitude = NMEA2DecimalDegrees(gpsFixData.latitude, gpsFixData.latitudeCardinal);
    gpsFixData.longitude = NMEA2DecimalDegrees(gpsFixData.longitude, gpsFixData.longitudeCardinal);
    DataManager::GetInstance()->PushGpsFixData(gpsFixData);
}

}
