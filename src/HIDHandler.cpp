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
        void ExecuteHandlers(std::list<std::shared_ptr<OpenCC::Callback>> handlers);
        std::list<std::shared_ptr<Callback>> OnEnterDown_;
        std::list<std::shared_ptr<Callback>> OnEnterUp_;
        std::list<std::shared_ptr<Callback>> OnEnterPressed_;
        std::list<std::shared_ptr<Callback>> OnEnterPressed2Seconds_;
        std::list<std::shared_ptr<Callback>> OnEnterPressed5Seconds_;
        std::list<std::shared_ptr<Callback>> OnExitDown_;
        std::list<std::shared_ptr<Callback>> OnExitUp_;
        std::list<std::shared_ptr<Callback>> OnExitPressed_;
        std::list<std::shared_ptr<Callback>> OnExitPressed2Seconds_;
        std::list<std::shared_ptr<Callback>> OnExitPressed5Seconds_;

        //TODO: Functions below should be callbacks from GPIO
        void EnterDown() { ExecuteHandlers(OnEnterDown_); };
        void EnterUp() { ExecuteHandlers(OnEnterUp_); };
        void EnterPressed() { ExecuteHandlers(OnEnterPressed_); };
        void EnterPressed2Seconds() { ExecuteHandlers(OnEnterPressed2Seconds_); }
        void EnterPressed5Seconds() { ExecuteHandlers(OnEnterPressed5Seconds_); }
        void ExitDown() { ExecuteHandlers(OnExitDown_); }
        void ExitUp() { ExecuteHandlers(OnExitUp_); }
        void ExitPressed() { ExecuteHandlers(OnExitPressed_); }
        void ExitPressed2Seconds() { ExecuteHandlers(OnExitPressed2Seconds_); }
        void ExitPressed5Seconds() { ExecuteHandlers(OnExitPressed5Seconds_); }
    public:
        std::list<std::shared_ptr<OpenCC::Callback>>::const_iterator RegisterEventHandler(
            HIDEventType eventType, std::function<void()> handler);
        void UnregisterEventHandler(
            HIDEventType eventType, std::list<std::shared_ptr<OpenCC::Callback>>::const_iterator iterator);
};

void HIDHandler::ExecuteHandlers(std::list<std::shared_ptr<OpenCC::Callback>> handlers) {
    for (auto handler : handlers) {
        handler->Method();
    }
}

std::list<std::shared_ptr<OpenCC::Callback>>::const_iterator HIDHandler::RegisterEventHandler(
    HIDEventType eventType, std::function<void()> handler) {
    switch (eventType) {
        case ENTER_DOWN:
            OnEnterDown_.push_back(std::make_unique<OpenCC::Callback>(handler));
            return OnEnterDown_.end();
        case ENTER_UP:
            OnEnterUp_.push_back(std::make_unique<OpenCC::Callback>(handler));
            return OnEnterUp_.end();
        case ENTER_PRESSED:
            OnEnterPressed_.push_back(std::make_unique<OpenCC::Callback>(handler));
            return OnEnterPressed_.end();
        case ENTER_PRESSED_2_SECONDS:
            OnEnterPressed2Seconds_.push_back(std::make_unique<OpenCC::Callback>(handler));
            return OnEnterPressed2Seconds_.end();
        case ENTER_PRESSED_5_SECONDS:
            OnEnterPressed5Seconds_.push_back(std::make_unique<OpenCC::Callback>(handler));
            return OnEnterPressed5Seconds_.end();
        case EXIT_DOWN:
            OnExitDown_.push_back(std::make_unique<OpenCC::Callback>(handler));
            return OnExitDown_.end();
        case EXIT_UP:
            OnExitUp_.push_back(std::make_unique<OpenCC::Callback>(handler));
            return OnExitUp_.end();
        case EXIT_PRESSED:
            OnExitPressed_.push_back(std::make_unique<OpenCC::Callback>(handler));
            return OnExitPressed_.end();
        case EXIT_PRESSED_2_SECONDS:
            OnExitPressed2Seconds_.push_back(std::make_unique<OpenCC::Callback>(handler));
            return OnExitPressed2Seconds_.end();
        case EXIT_PRESSED_5_SECONDS:
            OnExitPressed5Seconds_.push_back(std::make_unique<OpenCC::Callback>(handler));
            return OnExitPressed5Seconds_.end();
    }

    // Probably never gets here, so any iterator is ok.
    return OnExitPressed_.end();
}

void HIDHandler::UnregisterEventHandler(
    HIDEventType eventType, std::list<std::shared_ptr<OpenCC::Callback>>::const_iterator iterator) {
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