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

#include <memory>
#include <list>
#include "Page/BasePage.cpp"
#include "../HIDHandler.cpp"

namespace OpenCC {

class GUINavigator {
    private:
        OpenCC::HIDHandler& handler_;
        std::list<std::unique_ptr<OpenCC::BasePage>>& pages_;
        std::list<std::unique_ptr<OpenCC::BasePage>>::iterator pageIndex_;
        void RegisterEvents();
        void UnregisterEvents();
        void GoToNextPage();
        void GoToPreviousPage();
        std::list<std::shared_ptr<OpenCC::Callback>>::const_iterator previousPageReference_;
        std::list<std::shared_ptr<OpenCC::Callback>>::const_iterator nextPageReference_;
    public:
        GUINavigator(OpenCC::HIDHandler& handler, std::list<std::unique_ptr<OpenCC::BasePage>>& pages)
            : handler_(handler), pages_(pages) {
            RegisterEvents();

            if (pages_.size() == 0) return;

            pageIndex_ = pages_.begin();
            (*pageIndex_)->Setup();
        }
        ~GUINavigator() {
            //TODO: This is throwing an invalid pointer exception. Check the iterators reference.
            // UnregisterEvents();
        }
};

void GUINavigator::RegisterEvents() {
    nextPageReference_ = handler_.RegisterEventHandler(HIDEventType::ENTER_PRESSED, [this](){this->GoToNextPage();});
    previousPageReference_ = handler_.RegisterEventHandler(HIDEventType::EXIT_PRESSED, [this](){this->GoToPreviousPage();});
}

void GUINavigator::UnregisterEvents() {
    handler_.UnregisterEventHandler(HIDEventType::ENTER_PRESSED, nextPageReference_);
    handler_.UnregisterEventHandler(HIDEventType::EXIT_PRESSED, previousPageReference_);
}

void GUINavigator::GoToNextPage() {
    if (pages_.size() == 0) return;

    if (pageIndex_ == pages_.end()) {
        pageIndex_ = pages_.begin();
    } else {
        std::advance(pageIndex_, 1);
    }

    (*pageIndex_)->Setup();
}

void GUINavigator::GoToPreviousPage() {
    if (pages_.size() == 0) return;

    if (pageIndex_ == pages_.begin()) {
        pageIndex_ = pages_.end();
    } else {
        std::advance(pageIndex_, -1);
    }

    (*pageIndex_)->Setup();
}

}