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
#include "iSensor.hpp"
#include "../Model/GPSFixData.hpp"
#include "../DataManager.cpp"
// #include <fstream> // file stream
#include "hardware/gpio.h"

#ifdef _DEBUG
    #include <iostream> // cout
#endif

namespace OpenCC {

class GPS : public iSensor {
    private:
        const std::string GPS_FIX = "GGA,"; // $GNGGA, $GPGGA.
        const int startingPos = 3;
        bool IsGpsFixInfo(std::string &info);
        void UartGetLine(std::string &line);
        void LogGpsData(OpenCC::GPSFixData &gpsFixData);
    public:
        void Enable() override;
        void Disable() override;
        void GetData() override;
};

void GPS::Enable() {
    // std::ifstream uart;
    // std::ios_base::iostate exceptionMask = uart.exceptions() | std::ios::failbit;
    // uart.exceptions(exceptionMask);

    // try {
    //     uart.open("/dev/serial0", std::ifstream::in);
    // } catch (std::ios_base::failure &error) {
    //     std::cout << error.what();
    //     this->enabled_ = false;
    // }

    uart_init(uart0, 9600);
    gpio_set_function(0, GPIO_FUNC_UART); // GPIO pin 0 is UART0 TX
    gpio_set_function(1, GPIO_FUNC_UART); // GPIO pin 1 is UART0 RX

#ifdef L96GPS
    uart_puts(uart0, "$PMTK353,1,1,1,0,0*2A\0"); // enable GPS, GLONASS and GALILEO satellite system.
    uart_puts(uart0, "$PMTK869,1,1*35\0"); // enable AGPS (EASY function).
    uart_puts(uart0, "$PMTK886,1*29\0"); // enable fitness mode.
    //uart_puts(uart0, "$PMTK886,0*28\0"); // enable normal mode.
#endif

    this->enabled_ = true;
}

void GPS::Disable() {
    // uart.close();
    uart_deinit(uart0);
    this->enabled_ = false;
}

void GPS::LogGpsData(OpenCC::GPSFixData &gpsFixData) {
    OpenCC::DataManager::GetInstance()->Push(gpsFixData);
}

bool GPS::IsGpsFixInfo(std::string &info) {
    return info.rfind(GPS_FIX, startingPos) == startingPos;
}

void GPS::UartGetLine(std::string &line) {
    // return std::getline(uart, line);
    char singleChar = '\0';
    line = "";

    while (true) {
        singleChar = uart_getc(uart0);

        if (singleChar == 0 || singleChar == '\0' || singleChar == '\n') break;

        line += singleChar;
    }
}

void GPS::GetData() {
    // && uart.is_open()
    if (!enabled_ || !uart_is_enabled(uart0)) return;

    std::string serial_rx = "";
    int attempts = 0;

    while (!IsGpsFixInfo(serial_rx) && (attempts < 50)) {
        attempts++;
        UartGetLine(serial_rx);
#ifdef _DEBUG
    std::cout << std::to_string(attempts) << " - GPS: " << serial_rx << std::endl;
#endif
    }

    if (!IsGpsFixInfo(serial_rx)) return;

    OpenCC::GPSFixData gpsFixData;
    gpsFixData.set(serial_rx);

    // normalize coordinates
    gpsFixData.latitude /= 100;
    gpsFixData.longitude /= 100;

    if (gpsFixData.latitudeCardinal == 'S') gpsFixData.latitude *= -1;
    if (gpsFixData.longitudeCardinal == 'W') gpsFixData.longitude *= -1;

    LogGpsData(gpsFixData);
}

}