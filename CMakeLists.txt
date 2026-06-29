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

# version requirements
cmake_minimum_required(VERSION 3.16)

# Generate compile_commands.json (consumed by clangd for code intelligence)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform definitions
set(PLATFORMS "RP2040" "ESP32")
set(PLATFORM_NAME "RP2040" CACHE STRING "Build platform, one of: ${PLATFORMS}")
set_property(CACHE PLATFORM_NAME PROPERTY STRINGS ${PLATFORMS})

# Default platform selection
if(NOT DEFINED PLATFORM_NAME)
    set(PLATFORM_NAME "RP2040")
endif()

# Platform-agnostic translation units.
include(${CMAKE_SOURCE_DIR}/src/Dependency/gui.ll/gui.ll.cmake)

# Optional debug messages: pass -DDEBUGMSGS (no value needed) at configure time to
# define the DEBUGMSGS macro for the build (used via #ifdef DEBUGMSGS in the sources).
# Guarded with DEFINED so it triggers whether CMake stores it as "ON" or as an empty value.
if(DEFINED DEBUGMSGS)
    add_compile_definitions(DEBUGMSGS)
endif()

set(PEDAL_GURU "pedal.guru")
set(MAIN_EXECUTABLE src/OpenCC.cpp)

# Build system per platform
if(PLATFORM_NAME STREQUAL "ESP32")
    # ESP-IDF orchestrates the build.
    # EXTRA_COMPONENT_DIRS adds our component without losing ESP-IDF built-in components.
    set(EXTRA_COMPONENT_DIRS
        ${MAIN_EXECUTABLE}
        ${EXTRA_COMPONENT_DIRS}
    )
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(${PEDAL_GURU})

elseif(PLATFORM_NAME STREQUAL "RP2040")
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD 14)

    if(NOT DEFINED PICO_SDK_PATH)
        set(PICO_SDK_PATH ~/pico-sdk)
    endif()
    include(${CMAKE_SOURCE_DIR}/src/Dependency/gui.ll/src/Dependency/pico_sdk_import.cmake)

    project(${PEDAL_GURU})
    pico_sdk_init()

    # Include platform-specific cmake fragments
    include(${CMAKE_SOURCE_DIR}/src/Dependency/gui.ll/src/lib/Platform/RP2040/PreExecutable.cmake)

    add_executable(${PROJECT_NAME}
        ${MAIN_EXECUTABLE}
        ${GUILL_RP2040_SRCS}
    )

    # Include platform-specific cmake fragments
    include(${CMAKE_SOURCE_DIR}/src/Dependency/gui.ll/src/lib/Platform/RP2040/PostExecutable.cmake)
    include(${CMAKE_SOURCE_DIR}/src/Target/RP2040/configure_target.cmake)
endif()
