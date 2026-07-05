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

cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generate compile_commands.json (consumed by clangd for code intelligence)
set(PROJECT_NAME "pedal.guru")

if(DEFINED DEBUGMSGS) # Optional debug messages: pass -DDEBUGMSGS (no value needed)
    add_compile_definitions(DEBUGMSGS _DEBUG)
endif()

# Build system per platform
if(PLATFORM_NAME STREQUAL "ESP32")
    # ESP-IDF orchestrates the build: EXTRA_COMPONENT_DIRS adds our component keeping ESP-IDF built-in components.
    set(EXTRA_COMPONENT_DIRS "${CMAKE_SOURCE_DIR}/src/lib/Platform/ESP32")
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(${PROJECT_NAME})

elseif(PLATFORM_NAME STREQUAL "Simulator")
    set(CMAKE_C_STANDARD 11)
    project(${PROJECT_NAME})
    include(${CMAKE_SOURCE_DIR}/pedal.guru.cmake)
    set(SOURCES ${SOURCES})
    add_executable(${PROJECT_NAME} ${SOURCES})
    include_directories(${INCLUDE_DIRS})

    find_package(CURL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${SDL2_LIBRARIES} m CURL::libcurl)

elseif(PLATFORM_NAME STREQUAL "RP2040")
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD 14)
    set(PICO_TOOLCHAIN_PATH /usr/bin)

    if(NOT DEFINED PICO_SDK_PATH)
        set(PICO_SDK_PATH ~/pico-sdk)
    endif()
    include(${CMAKE_SOURCE_DIR}/src/Dependency/pico_sdk_import.cmake)

    project(${PROJECT_NAME})
    pico_sdk_init()
    include(${CMAKE_SOURCE_DIR}/pedal.guru.cmake)
    set(SOURCES ${SOURCES})
    add_executable(${PROJECT_NAME} ${SOURCES})
    include_directories(${INCLUDE_DIRS})

    target_link_libraries(${PROJECT_NAME} PUBLIC
        pico_stdlib pico_multicore hardware_spi hardware_gpio hardware_pwm hardware_adc hardware_rtc)

    pico_enable_stdio_usb(${PROJECT_NAME} 1)  # enable usb output
    pico_enable_stdio_uart(${PROJECT_NAME} 0) # disable uart output
    pico_add_extra_outputs(${PROJECT_NAME})   # create map/bin/hex/uf2 files in addition to elf
endif()
