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

#include <list>
#include "Model/HIDEventType.hpp"

namespace OpenCC {

class HIDHandler {
    private:
        std::list<void*> OnEnterPressed_;
        std::list<void*> OnEnterPressed2Seconds_;
        std::list<void*> OnEnterPressed5Seconds_;
        std::list<void*> OnExitPressed_;
        std::list<void*> OnExitPressed2Seconds_;
        std::list<void*> OnExitPressed5Seconds_;
    public:
        void RegisterEventHandler(HIDEventType event, void* handler);
        void UnregisterEventHandler(HIDEventType event, void* handler);
};

void HIDHandler::RegisterEventHandler(HIDEventType eventType, void* handler) {
    switch (eventType) {
        case ENTER_PRESSED:
            OnEnterPressed_.push_back(handler);
            break;
        case ENTER_PRESSED_2_SECONDS:
            OnEnterPressed2Seconds_.push_back(handler);
            break;
        case ENTER_PRESSED_5_SECONDS:
            OnEnterPressed5Seconds_.push_back(handler);
            break;
        case EXIT_PRESSED:
            OnExitPressed_.push_back(handler);
            break;
        case EXIT_PRESSED_2_SECONDS:
            OnExitPressed2Seconds_.push_back(handler);
            break;
        case EXIT_PRESSED_5_SECONDS:
            OnExitPressed5Seconds_.push_back(handler);
            break;
        default:
            break;
    }
}

void HIDHandler::UnregisterEventHandler(HIDEventType eventType, void* handler) {
    switch (eventType) {
        case ENTER_PRESSED:
            OnEnterPressed_.remove(handler);
            break;
        case ENTER_PRESSED_2_SECONDS:
            OnEnterPressed2Seconds_.remove(handler);
            break;
        case ENTER_PRESSED_5_SECONDS:
            OnEnterPressed5Seconds_.remove(handler);
            break;
        case EXIT_PRESSED:
            OnExitPressed_.remove(handler);
            break;
        case EXIT_PRESSED_2_SECONDS:
            OnExitPressed2Seconds_.remove(handler);
            break;
        case EXIT_PRESSED_5_SECONDS:
            OnExitPressed5Seconds_.remove(handler);
            break;
        default:
            break;
    }
}

}