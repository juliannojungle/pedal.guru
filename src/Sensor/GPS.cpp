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
extern "C" {
    #include "HAL.h"
}

namespace PedalGuru {

void GPS::Enable() {
    UARTInit(GPS_UART, GPS_UART_BAUDRATE, GPS_UART_TX_PIN, GPS_UART_RX_PIN);

#ifdef L96GPS
    // Real configuration commands for the Quectel L96 module, validated on RP2040.
    // The strings must not change; only the API to send them was updated.
    // UARTPuts(GPS_UART, "$PMTK353,1,1,1,0,0*2A\0"); // enable GPS, GLONASS and GALILEO satellite system.
    // UARTPuts(GPS_UART, "$PMTK869,1,1*35\0");        // enable AGPS (EASY function).
    // UARTPuts(GPS_UART, "$PMTK886,1*29\0");           // enable fitness mode.
    //#UARTPuts(GPS_UART, "$PMTK886,0*28\0");           // enable normal mode.
#endif

    this->enabled_ = true;
}

void GPS::Disable() {
    UARTDeinit(GPS_UART);
    this->enabled_ = false;
}

void GPS::LogGpsData(PedalGuru::GPSFixData &gpsFixData) {
    PedalGuru::DataManager::GetInstance()->Push(gpsFixData);
}

bool GPS::IsGpsFixInfo(std::string &info) {
    return info.rfind(GPS_FIX, startingPos) == startingPos;
}

void GPS::UartGetLine(std::string &line) {
    char singleChar = '\0';
    line = "";

    while (UARTIsReadable(GPS_UART)) {
        singleChar = UARTGetChar(GPS_UART);

        if (singleChar == '\0' || singleChar == '\n') break;

        line += singleChar;
    }
}

void GPS::GetData() {
    if (!enabled_ || !UARTIsEnabled(GPS_UART)) return;

    std::string serial_rx = "";
    int attempts = 0;

    while (!IsGpsFixInfo(serial_rx) && (attempts < 50)) {
        attempts++;
        UartGetLine(serial_rx);
    }

    if (!IsGpsFixInfo(serial_rx)) return;

    PedalGuru::GPSFixData gpsFixData;
    gpsFixData.set(serial_rx);

    // convert NMEA (DDMM.MMMM) to decimal degrees
    double latDegrees = (int)(gpsFixData.latitude / 100);
    gpsFixData.latitude = latDegrees + (gpsFixData.latitude - latDegrees * 100) / 60.0;

    double lonDegrees = (int)(gpsFixData.longitude / 100);
    gpsFixData.longitude = lonDegrees + (gpsFixData.longitude - lonDegrees * 100) / 60.0;

    if (gpsFixData.latitudeCardinal == 'S') gpsFixData.latitude *= -1;
    if (gpsFixData.longitudeCardinal == 'W') gpsFixData.longitude *= -1;

    LogGpsData(gpsFixData);
}

}
