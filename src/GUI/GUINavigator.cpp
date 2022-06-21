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
#include "Page/iPage.hpp"
#include "../HIDHandler.cpp"

namespace OpenCC {

class GUINavigator {
    private:
        HIDHandler *handler_;
        std::list<iPage*> pages_;
        std::list<iPage*>::iterator pageIndex_;
        void RegisterEvents();
        void UnregisterEvents();
        std::function<void()> GoToNextPage();
        std::function<void()> GoToPreviousPage();
    public:
        GUINavigator(OpenCC::HIDHandler *handler, std::list<OpenCC::iPage*> pages);
        ~GUINavigator();
};

GUINavigator::GUINavigator(OpenCC::HIDHandler *handler, std::list<OpenCC::iPage*> pages) {
    this->handler_ = handler;
    this->pages_ = pages;
    this->pageIndex_ = this->pages_.begin();
    RegisterEvents();
    (*pageIndex_)->Show();
}

GUINavigator::~GUINavigator() {
    UnregisterEvents();
}

void GUINavigator::RegisterEvents() {
    handler_->RegisterEventHandler(HIDEventType::ENTER_PRESSED, &GoToNextPage());
    handler_->RegisterEventHandler(HIDEventType::EXIT_PRESSED, &GoToPreviousPage());
}

void GUINavigator::UnregisterEvents() {
    handler_->UnregisterEventHandler(HIDEventType::ENTER_PRESSED, &GoToNextPage());
    handler_->UnregisterEventHandler(HIDEventType::EXIT_PRESSED, &GoToPreviousPage());
}

std::function<void()> GUINavigator::GoToNextPage() {
    if (this->pageIndex_ == this->pages_.end()) {
        this->pageIndex_ = this->pages_.begin();
    } else {
        std::advance(this->pageIndex_, 1);
    }

    (*pageIndex_)->Show();
}

std::function<void()> GUINavigator::GoToPreviousPage() {
    if (this->pageIndex_ == this->pages_.begin()) {
        this->pageIndex_ = this->pages_.end();
    } else {
        std::advance(this->pageIndex_, -1);
    }

    (*pageIndex_)->Show();
}
}