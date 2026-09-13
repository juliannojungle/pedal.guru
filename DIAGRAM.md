<!---
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
--->

# Class diagram
- Basic structure of the project.

<!---
    For syntax reference about diagram's markdown, see <https://mermaid-js.github.io/mermaid/#/./classDiagram>.
--->
```mermaid
classDiagram

class SettingsData

link SettingsData "https://github.com/juliannojungle/PedalGuru/blob/main/src/PedalGuru.hpp"

class TaskManager {
    -SettingsData *settings_
    -List~*Device~ devices_
    -List~*BasePage~ pages_
    -ReadSettings() SettingsData
    -DisplayPage(BasePage *page)
    -StartDevice(Device *device)
    +Run()
}

link TaskManager "https://github.com/juliannojungle/PedalGuru/blob/main/src/task_manager.cpp"
TaskManager ..|> BasePage
TaskManager ..|> Device
TaskManager ..> SettingsData : Reads

class BasePage {
    <<interface>>
    -SensorData *data_
    -SettingsData *settings_
    -ReadData() SensorData
    -WriteSettings(SettingsData *settings)
    -ReadSettings() SettingsData
    +PreviousPage()
    +NextPage()
}

link BasePage "https://github.com/juliannojungle/PedalGuru/blob/main/src/gui/pages/BasePage.cpp"

class Sensor {
    <<interface>>
    #bool enabled_
    +Enabled()
    +Enable()
    +Disable()
}

link Sensor "https://github.com/juliannojungle/PedalGuru/blob/main/src/sensors/Sensor.hpp"

class Device {
    <<interface>>
    #bool connected_
    #List~Sensor~ sensors_
    -SettingsData *settings
    +Connect()
    +Connected()
}

link Device "https://github.com/juliannojungle/PedalGuru/blob/main/src/devices/Device.hpp"

class SensorData

link SensorData "https://github.com/juliannojungle/PedalGuru/blob/main/src/PedalGuru.hpp"

Device "1" --* "1..*" Sensor : Contains
SensorData <.. Device : Writes

BasePage "1" ..> "1..*" SensorData : Reads
SettingsData <.. BasePage : Reads/Writes
Device ..> SettingsData : Reads
```