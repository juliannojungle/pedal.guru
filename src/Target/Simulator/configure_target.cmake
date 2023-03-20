#[[
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
]]

set(CMAKE_CXX_COMPILER /usr/bin/g++)

set(RAYLIB_LIB ${CMAKE_SOURCE_DIR}/src/Dependency/raylib/lib)
set(RAYLIB_INCLUDE ${CMAKE_SOURCE_DIR}/src/Dependency/raylib/include)
set(VIDEOCORE ${CMAKE_SYSROOT}/opt/vc/lib)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${RAYLIB_LIB}
    ${RAYLIB_INCLUDE}
    PRIVATE
    ${VIDEOCORE})

find_library(RAYLIB NAMES raylib PATHS ${RAYLIB_LIB})
target_link_libraries(${PROJECT_NAME} PRIVATE ${RAYLIB})

find_library(BRCMEGL NAMES brcmEGL PATHS ${VIDEOCORE})
target_link_libraries(${PROJECT_NAME} PUBLIC ${BRCMEGL})

find_library(BRCMGLESV2 NAMES brcmGLESv2 PATHS ${VIDEOCORE})
target_link_libraries(${PROJECT_NAME} PUBLIC ${BRCMGLESV2})

find_library(PTHREAD NAMES pthread PATHS ${VIDEOCORE})
target_link_libraries(${PROJECT_NAME} PUBLIC ${PTHREAD})

find_library(BCM_HOST NAMES bcm_host PATHS ${VIDEOCORE})
target_link_libraries(${PROJECT_NAME} PUBLIC ${BCM_HOST})

# execute simulator after build
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND sudo ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})