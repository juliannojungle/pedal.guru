/*
    Pedal.guru is an open-source software
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

#include "Callback.hpp"
#include <list>
#include <memory>
#include "HIDEventType.hpp"

namespace PedalGuru {

class HIDHandler {
    private:
        void ExecuteHandlers(std::list<std::shared_ptr<PedalGuru::Callback>> handlers);
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

        //TODO: Functions below should be interruption callbacks from GPIO
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
        std::list<std::shared_ptr<PedalGuru::Callback>>::const_iterator RegisterEventHandler(
            HIDEventType eventType, std::function<void()> handler);
        void UnregisterEventHandler(
            HIDEventType eventType, std::list<std::shared_ptr<PedalGuru::Callback>>::const_iterator iterator);
};

}