cmake_minimum_required(VERSION 3.13)
include(src/Dependency/pico-sdk/pico_sdk_init.cmake)
project(opencc)

pico_sdk_init()

add_executable(opencc src/OpenCC.cpp)

set(TARGET_RP2040)
add_definitions(-DTARGET_RP2040)
add_compile_definitions(TARGET_RP2040)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(opencc pico_stdlib)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)


# enable usb output, disable uart output
pico_enable_stdio_usb(opencc 1)
pico_enable_stdio_uart(opencc 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(opencc)