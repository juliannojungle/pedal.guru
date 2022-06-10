<!---
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
--->

# Class diagram
- Basic structure of the project.

<!---
    For syntax reference about diagram's markdown, see <https://mermaid-js.github.io/mermaid/#/./classDiagram>.
--->
```mermaid
classDiagram

class SettingsData

link SettingsData "https://github.com/juliannojungle/OpenCC/blob/main/src/opencc.hpp"

class TaskManager {
    -SettingsData *settings_
    -List~*iDevice~ devices_
    -List~*iPage~ pages_
    -ReadSettings() SettingsData
    -DisplayPage(iPage *page)
    -StartDevice(iDevice *device)
    +Run()
}

link TaskManager "https://github.com/juliannojungle/OpenCC/blob/main/src/task_manager.cpp"
TaskManager ..|> iPage
TaskManager ..|> iDevice
TaskManager ..> SettingsData : Reads

class iPage {
    <<interface>>
    -SensorData *data_
    -SettingsData *settings_
    -ReadData() SensorData
    -WriteSettings(SettingsData *settings)
    -ReadSettings() SettingsData
    +PreviousPage()
    +NextPage()
}

link iPage "https://github.com/juliannojungle/OpenCC/blob/main/src/gui/pages/iPage.hpp"

class iSensor {
    <<interface>>
    #bool enabled_
    +Enabled()
    +Enable()
    +Disable()
}

link iSensor "https://github.com/juliannojungle/OpenCC/blob/main/src/sensors/iSensor.hpp"

class iDevice {
    <<interface>>
    #bool connected_
    #List~iSensor~ sensors_
    -SettingsData *settings
    +Connect()
    +Connected()
}

link iDevice "https://github.com/juliannojungle/OpenCC/blob/main/src/devices/iDevice.hpp"

class SensorData

link SensorData "https://github.com/juliannojungle/OpenCC/blob/main/src/opencc.hpp"

iDevice "1" --* "1..*" iSensor : Contains
SensorData <.. iDevice : Writes

iPage "1" ..> "1..*" SensorData : Reads
SettingsData <.. iPage : Reads/Writes
iDevice ..> SettingsData : Reads
```