/*
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
*/

#include <iterator>
#include "GUINavigator.hpp"
#include "GUIDrawer.hpp"
#include "HIDHandler.hpp"
#include "PageAltimetry.hpp"
#include "PageDistance.hpp"
#include "PageHillsGraph.hpp"
#include "PageMap.hpp"
#include "PageMapSync.hpp"
#include "PageProvisioning.hpp"
#include "PageRoute.hpp"
#include "PageSummary.hpp"

extern "C" {
    #include "HAL.h"
}

namespace PedalGuru {

GUINavigator& GUINavigator::GetInstance() {
    static GUINavigator instance;
    return instance;
}

void GUINavigator::Setup(std::list<AvailablePages>& pages) {
    pages_ = &pages;
    RegisterEvents();

    if (pages_->size() == 0) return;

    pageIndex_ = pages_->begin();
    currentPage_ = GetPage(*pageIndex_);
    currentPage_->Setup();
    ThreadStart(ExecuteGuiDrawer, 3072, "GuiDrawer"); // Separated task to "handle HID and GUI".
}

void GUINavigator::RegisterEvents() {
    auto& handler = HIDHandler::GetInstance();
    nextPageReference_ = handler.RegisterEventHandler(HIDEventType::ENTER_PRESSED, [this](){this->GoToNextPage();});
    previousPageReference_ = handler.RegisterEventHandler(HIDEventType::EXIT_PRESSED, [this](){this->GoToPreviousPage();});
}

void GUINavigator::UnregisterEvents() {
    auto& handler = HIDHandler::GetInstance();
    handler.UnregisterEventHandler(HIDEventType::ENTER_PRESSED, nextPageReference_);
    handler.UnregisterEventHandler(HIDEventType::EXIT_PRESSED, previousPageReference_);
}

void GUINavigator::GoToNextPage() {
    if (pages_->size() == 0) return;

    if (pageIndex_ == std::prev(pages_->end())) {
        pageIndex_ = pages_->begin();
    } else {
        std::advance(pageIndex_, 1);
    }

    currentPage_ = GetPage(*pageIndex_);
    currentPage_->Setup();
}

void GUINavigator::GoToPreviousPage() {
    if (pages_->size() == 0) return;

    if (pageIndex_ == pages_->begin()) {
        pageIndex_ = std::prev(pages_->end());
    } else {
        std::advance(pageIndex_, -1);
    }

    currentPage_ = GetPage(*pageIndex_);
    currentPage_->Setup();
}

std::unique_ptr<BasePage> GUINavigator::GetPage(AvailablePages page) {
    switch (page) {
        case AvailablePages::PAGE_MAP:
            return std::make_unique<PageMap>(); break;
        case AvailablePages::PAGE_ROUTE:
            return std::make_unique<PageRoute>(); break;
        case AvailablePages::PAGE_HILLS_GRAPH:
            return std::make_unique<PageHillsGraph>(); break;
        case AvailablePages::PAGE_DISTANCE:
            return std::make_unique<PageDistance>(); break;
        case AvailablePages::PAGE_ALTIMETRY:
            return std::make_unique<PageAltimetry>(); break;
        case AvailablePages::PAGE_SUMMARY:
            return std::make_unique<PageSummary>(); break;
        case AvailablePages::PAGE_MAP_SYNC:
            return std::make_unique<PageMapSync>(); break;
        case AvailablePages::PAGE_PROVISIONING:
            return std::make_unique<PageProvisioning>(); break;
        default:
            return nullptr;
    }
}

void GUINavigator::ExecuteGuiDrawer() {
    GUIDrawer::GetInstance().Execute();
}

}