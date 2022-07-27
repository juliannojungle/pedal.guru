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

#include <list>
#include <memory>
#include "../../../Sensor/GPS.cpp"
#include "../../iDevice.hpp"

namespace OpenCC {

class BN180 : public iDevice {
    public:
        BN180();
        void Connect() override;
        void Disconnect() override;
};

BN180::BN180() {
    this->sensors_.push_back(std::make_unique<OpenCC::GPS>());
}

void BN180::Connect() {
    for(const auto &sensor : sensors_) {
        if (!sensor.get()->Enabled())
            sensor.get()->Enable();
    }

    this->connected_ = true;
}

void BN180::Disconnect() {
    for(const auto &sensor : sensors_) {
        if (sensor.get()->Enabled())
            sensor.get()->Disable();
    }

    this->connected_ = false;
}

}