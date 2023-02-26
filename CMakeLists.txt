cmake_minimum_required(VERSION 3.13)
include(src/Dependency/pico-sdk/pico_sdk_init.cmake)
project(opencc)

pico_sdk_init()

add_executable(opencc src/OpenCC.cpp)

add_compile_definitions(
    TARGET_RP2040
    _DEBUG
)

target_link_libraries(opencc pico_stdlib)
target_link_libraries(opencc pico_multicore)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

pico_enable_stdio_usb(opencc 1) # enable usb output
pico_enable_stdio_uart(opencc 0) # disable uart output
pico_add_extra_outputs(opencc) # create map/bin/hex/uf2 files in addition to elf