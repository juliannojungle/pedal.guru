#[[
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
]]

cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generate compile_commands.json (consumed by clangd for code intelligence)
set(PROJECT_NAME "pedal.guru")

if(DEFINED DEBUGMSGS) # Optional debug messages: pass -DDEBUGMSGS (no value needed)
    add_compile_definitions(DEBUGMSGS _DEBUG)
endif()

set(PLATFORMS "Simulator" "RP2040" "ESP32")
set(PLATFORM_NAME "Simulator" CACHE STRING "Build platform, one of: ${PLATFORMS}")
set_property(CACHE PLATFORM_NAME PROPERTY STRINGS ${PLATFORMS})

# Build system per platform
if(PLATFORM_NAME STREQUAL "ESP32")
    # ESP-IDF orchestrates the build: EXTRA_COMPONENT_DIRS adds our component keeping ESP-IDF built-in components.
    set(EXTRA_COMPONENT_DIRS "${CMAKE_SOURCE_DIR}/src/Platform/ESP32")
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(${PROJECT_NAME})

elseif(PLATFORM_NAME STREQUAL "Simulator")
    set(CMAKE_C_STANDARD 11)
    project(${PROJECT_NAME})
    include(${CMAKE_SOURCE_DIR}/pedal.guru.cmake)
    set(SOURCES ${SOURCES})
    add_executable(${PROJECT_NAME} ${SOURCES})
    include_directories(${INCLUDE_DIRS})
    set_source_files_properties(${PEDAL_GURU_SOURCES} PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")

    find_package(OpenSSL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC
        ${SDL2_LIBRARIES} m OpenSSL::SSL OpenSSL::Crypto ${PLATFORM_LIBRARIES})

elseif(PLATFORM_NAME STREQUAL "RP2040")
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD 14)
    set(PICO_TOOLCHAIN_PATH /usr/bin)

    # Custom board definition: Waveshare RP2040-LCD-1.28 + expansion board (with cyw43 radio).
    set(PICO_BOARD_HEADER_DIRS "${CMAKE_SOURCE_DIR}/src/Platform/RP2040/Boards")
    set(PICO_BOARD WaveshareRP2040LCD1In28Extended)

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
    set_source_files_properties(${PEDAL_GURU_SOURCES} PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")

    if(PLATFORM_DEFINITIONS)
        add_compile_definitions(${PLATFORM_DEFINITIONS})
    endif()

    target_link_libraries(${PROJECT_NAME} PUBLIC ${PLATFORM_LIBRARIES})

    pico_enable_stdio_usb(${PROJECT_NAME} 1)  # enable usb output
    pico_enable_stdio_uart(${PROJECT_NAME} 0) # disable uart output
    pico_add_extra_outputs(${PROJECT_NAME})   # create map/bin/hex/uf2 files in addition to elf

else()
    string(REPLACE ";" ", " PLATFORMS_PRETTY "${PLATFORMS}")
    message(FATAL_ERROR "PLATFORM_NAME must be one of: ${PLATFORMS_PRETTY} (got '${PLATFORM_NAME}')")
endif()
