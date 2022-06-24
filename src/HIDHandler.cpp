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

#include <functional>
#include <list>
#include "Model/HIDEventType.hpp"
#include "Model/Callback.hpp"

namespace OpenCC {

class HIDHandler {
    private:
        void ExecuteHandlers(std::list<Callback> handlers);
        std::list<Callback> OnEnterDown_;
        std::list<Callback> OnEnterUp_;
        std::list<Callback> OnEnterPressed_;
        std::list<Callback> OnEnterPressed2Seconds_;
        std::list<Callback> OnEnterPressed5Seconds_;
        std::list<Callback> OnExitDown_;
        std::list<Callback> OnExitUp_;
        std::list<Callback> OnExitPressed_;
        std::list<Callback> OnExitPressed2Seconds_;
        std::list<Callback> OnExitPressed5Seconds_;

        //TODO: Functions below should be callbacks from GPIO
        void EnterDown() { ExecuteHandlers(this->OnEnterDown_); };
        void EnterUp() { ExecuteHandlers(this->OnEnterUp_); };
        void EnterPressed() { ExecuteHandlers(this->OnEnterPressed_); };
        void EnterPressed2Seconds() { ExecuteHandlers(this->OnEnterPressed2Seconds_); }
        void EnterPressed5Seconds() { ExecuteHandlers(this->OnEnterPressed5Seconds_); }
        void ExitDown() { ExecuteHandlers(this->OnExitDown_); }
        void ExitUp() { ExecuteHandlers(this->OnExitUp_); }
        void ExitPressed() { ExecuteHandlers(this->OnExitPressed_); }
        void ExitPressed2Seconds() { ExecuteHandlers(this->OnExitPressed2Seconds_); }
        void ExitPressed5Seconds() { ExecuteHandlers(this->OnExitPressed5Seconds_); }
    public:
        std::list<OpenCC::Callback>::const_iterator RegisterEventHandler(
            HIDEventType event, std::function<void()> handler);
        void UnregisterEventHandler(HIDEventType eventType, std::list<OpenCC::Callback>::const_iterator iterator);
};

void HIDHandler::ExecuteHandlers(std::list<Callback> handlers) {
    for (auto handler : handlers) {
        handler.Method();
    }
}

std::list<OpenCC::Callback>::const_iterator HIDHandler::RegisterEventHandler(
    HIDEventType eventType, std::function<void()> handler) {
    switch (eventType) {
        case ENTER_DOWN:
            OnEnterDown_.push_back(Callback(handler));
            return OnEnterDown_.end();
        case ENTER_UP:
            OnEnterUp_.push_back(Callback(handler));
            return OnEnterUp_.end();
        case ENTER_PRESSED:
            OnEnterPressed_.push_back(Callback(handler));
            return OnEnterPressed_.end();
        case ENTER_PRESSED_2_SECONDS:
            OnEnterPressed2Seconds_.push_back(Callback(handler));
            return OnEnterPressed2Seconds_.end();
        case ENTER_PRESSED_5_SECONDS:
            OnEnterPressed5Seconds_.push_back(Callback(handler));
            return OnEnterPressed5Seconds_.end();
        case EXIT_DOWN:
            OnExitDown_.push_back(Callback(handler));
            return OnExitDown_.end();
        case EXIT_UP:
            OnExitUp_.push_back(Callback(handler));
            return OnExitUp_.end();
        case EXIT_PRESSED:
            OnExitPressed_.push_back(Callback(handler));
            return OnExitPressed_.end();
        case EXIT_PRESSED_2_SECONDS:
            OnExitPressed2Seconds_.push_back(Callback(handler));
            return OnExitPressed2Seconds_.end();
        case EXIT_PRESSED_5_SECONDS:
            OnExitPressed5Seconds_.push_back(Callback(handler));
            return OnExitPressed5Seconds_.end();
    }

    // Probably never gets here, so any iterator is ok.
    return OnExitPressed_.end();
}

void HIDHandler::UnregisterEventHandler(HIDEventType eventType, std::list<OpenCC::Callback>::const_iterator iterator) {
    switch (eventType) {
        case ENTER_PRESSED:
            OnEnterPressed_.erase(iterator);
            break;
        case ENTER_DOWN:
            OnEnterDown_.erase(iterator);
            break;
        case ENTER_UP:
            OnEnterUp_.erase(iterator);
            break;
        case ENTER_PRESSED_2_SECONDS:
            OnEnterPressed2Seconds_.erase(iterator);
            break;
        case ENTER_PRESSED_5_SECONDS:
            OnEnterPressed5Seconds_.erase(iterator);
            break;
        case EXIT_DOWN:
            OnExitDown_.erase(iterator);
            break;
        case EXIT_UP:
            OnExitUp_.erase(iterator);
            break;
        case EXIT_PRESSED:
            OnExitPressed_.erase(iterator);
            break;
        case EXIT_PRESSED_2_SECONDS:
            OnExitPressed2Seconds_.erase(iterator);
            break;
        case EXIT_PRESSED_5_SECONDS:
            OnExitPressed5Seconds_.erase(iterator);
            break;
    }
}
}