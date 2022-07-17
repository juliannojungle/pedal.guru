# set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_CXX_COMPILER C:/dev/gcc-arm/bin/arm-none-eabi-g++.exe)
# 
# project(OpenCC)
# 
# cmake_minimum_required(VERSION 3.13)
# 
# set(RAYLIB_LIB ${CMAKE_CURRENT_SOURCE_DIR}/src/Dependency/raylib/src)
# set(RAYLIB_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/src/Dependency/raylib/src)
# set(VIDEOCORE ${CMAKE_SYSROOT}/src/Dependency/opt/vc/lib)
# set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/OpenCC.cpp)
# 
# add_executable(${PROJECT_NAME} ${SRC})
# target_include_directories(
    # ${PROJECT_NAME}
    # PUBLIC
    # ${RAYLIB_LIB}
    # ${RAYLIB_INCLUDE}
    # PRIVATE
    # ${VIDEOCORE})
# 
# find_library(RAYLIB NAMES raylib PATHS ${RAYLIB_LIB})
# target_link_libraries(${PROJECT_NAME} PRIVATE ${RAYLIB})
# 
# find_library(BRCMEGL NAMES brcmEGL PATHS ${VIDEOCORE})
# target_link_libraries(${PROJECT_NAME} PUBLIC ${BRCMEGL})
# 
# find_library(BRCMGLESV2 NAMES brcmGLESv2 PATHS ${VIDEOCORE})
# target_link_libraries(${PROJECT_NAME} PUBLIC ${BRCMGLESV2})
# 
# find_library(PTHREAD NAMES pthread PATHS ${VIDEOCORE})
# target_link_libraries(${PROJECT_NAME} PUBLIC ${PTHREAD})
# 
# find_library(BCM_HOST NAMES bcm_host PATHS ${VIDEOCORE})
# target_link_libraries(${PROJECT_NAME} PUBLIC ${BCM_HOST})
# 
# add_custom_command(
    # TARGET ${PROJECT_NAME} POST_BUILD
    # COMMAND sudo ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
    # WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})