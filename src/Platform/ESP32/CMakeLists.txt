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

set(PROJ_ROOT ${CMAKE_SOURCE_DIR})

# Fallback: if CMAKE_SOURCE_DIR doesn't point to project root (ESP-IDF context),
# derive it from this file's location (src/Platform/ESP32/CMakeLists.txt)
if(NOT EXISTS "${PROJ_ROOT}/src/Dependency/pico_sdk_import.cmake")
    get_filename_component(PROJ_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
endif()

set(PLATFORM_NAME "ESP32")
include(${PROJ_ROOT}/pedal.guru.cmake)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDE_DIRS}
    REQUIRES ${PLATFORM_REQUIRES})

# Disable -Werror for third-party dependencies that produce
# warnings with the xtensa toolchain under ESP-IDF's strict -Werror=all.
# Using target_compile_options ensures -Wno-error comes after -Werror=all.
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=maybe-uninitialized
    -Wno-error=unused-but-set-variable)
