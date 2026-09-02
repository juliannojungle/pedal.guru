#[[
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
]]

set(PLATFORMS "Simulator" "RP2040" "ESP32") # Platform definitions

# ESP-IDF reads this file in script mode (cmake -P), where there is no cache: an unguarded
# set(... CACHE ...) is not skipped there and would overwrite the caller's PLATFORM_NAME.
if(NOT PLATFORM_NAME)
    set(PLATFORM_NAME "Simulator" CACHE STRING "Build platform, one of: ${PLATFORMS}")
endif()
if(DEFINED CACHE{PLATFORM_NAME})
    set_property(CACHE PLATFORM_NAME PROPERTY STRINGS ${PLATFORMS})
endif()

# Kept as its own list so the build can raise the warning level on our code only, without
# drowning it in warnings from the vendored dependencies.
set(PEDAL_GURU_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/DataManager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/HIDHandler.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/PedalGuru.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/TaskManager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/API/OpenStreetMapAPI.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/Device/Generic/LocationModule/LocationModule.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI/GUIDrawer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI/GUINavigator.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI/Page/BasePage.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI/Page/PageAltimetry.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI/Page/PageDistance.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI/Page/PageHillsGraph.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI/Page/PageMap.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI/Page/PageMapSync.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI/Page/PageRoute.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI/Page/PageSummary.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI/Render/Texture.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI/Render/Window.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/Helper/TextHelper.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/Model/GPSFixData.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/Platform/${PLATFORM_NAME}/HttpClient.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/Platform/${PLATFORM_NAME}/Thread.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/Platform/${PLATFORM_NAME}/Time.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/Sensor/GPS.cpp"
)

set(SOURCES ${SOURCES} ${PEDAL_GURU_SOURCES})

set(INCLUDE_DIRS
    ${INCLUDE_DIRS}
    "${CMAKE_CURRENT_LIST_DIR}/src"
    "${CMAKE_CURRENT_LIST_DIR}/src/API"
    "${CMAKE_CURRENT_LIST_DIR}/src/Device"
    "${CMAKE_CURRENT_LIST_DIR}/src/Device/Generic/LocationModule"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI/Page"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI/Render"
    "${CMAKE_CURRENT_LIST_DIR}/src/Helper"
    "${CMAKE_CURRENT_LIST_DIR}/src/Model"
    "${CMAKE_CURRENT_LIST_DIR}/src/Platform/${PLATFORM_NAME}"
    "${CMAKE_CURRENT_LIST_DIR}/src/Sensor")

# Each contract defaults its path to a folder next to itself, so all three have to be pinned
# here or the nested includes would clone second copies instead of using the submodules.
set(HAL_LL_PATH "${CMAKE_CURRENT_LIST_DIR}/src/Dependency/hal.ll" CACHE PATH "hal.ll root directory" FORCE)
set(FS_LL_PATH "${CMAKE_CURRENT_LIST_DIR}/src/Dependency/fs.ll" CACHE PATH "fs.ll root directory" FORCE)
set(GUI_LL_PATH "${CMAKE_CURRENT_LIST_DIR}/src/Dependency/gui.ll" CACHE PATH "gui.ll root directory" FORCE)

# Only gui.ll's contract is included: it pulls in hal.ll's and fs.ll's itself, so the whole
# dependency stack enters through this single line.
include(${GUI_LL_PATH}/gui.ll.cmake)

