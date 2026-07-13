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
set(PLATFORM_NAME "Simulator" CACHE STRING "Build platform, one of: ${PLATFORMS}")
set_property(CACHE PLATFORM_NAME PROPERTY STRINGS ${PLATFORMS})

string(TOUPPER ${PLATFORM_NAME} PLATFORM_NAME_UPPER)
add_compile_definitions(${PLATFORM_NAME_UPPER})

# pedal.guru
set(SOURCES
    ${SOURCES}
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

set(INCLUDE_DIRS
    ${INCLUDE_DIRS}
    "${CMAKE_CURRENT_LIST_DIR}/src"
    "${CMAKE_CURRENT_LIST_DIR}/src/API"
    "${CMAKE_CURRENT_LIST_DIR}/src/Device"
    "${CMAKE_CURRENT_LIST_DIR}/src/Device/Generic/LocationModule"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI/Interface"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI/Page"
    "${CMAKE_CURRENT_LIST_DIR}/src/GUI/Render"
    "${CMAKE_CURRENT_LIST_DIR}/src/Helper"
    "${CMAKE_CURRENT_LIST_DIR}/src/Model"
    "${CMAKE_CURRENT_LIST_DIR}/src/Platform/${PLATFORM_NAME}"
    "${CMAKE_CURRENT_LIST_DIR}/src/Sensor")

include(${CMAKE_CURRENT_LIST_DIR}/src/Dependency/gui.ll/gui.ll.cmake)

add_compile_definitions(L96GPS)