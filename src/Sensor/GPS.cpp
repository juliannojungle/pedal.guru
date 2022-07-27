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
#include <future>
#include <chrono>
#include <thread>
#include <iostream>

namespace OpenCC {

class GPS : public iSensor {
    private:
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

void GPS::GetData() {
    while (this->enabled_) {
        //Read actual GPS data from GPIO.
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << "GPS data aquired inside thread!\n";
    }
}
}