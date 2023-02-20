cmake_minimum_required(VERSION 3.13)
include(src/Dependency/pico-sdk/pico_sdk_init.cmake)
project(opencc)

pico_sdk_init()

add_executable(opencc
    src/OpenCC.cpp
)

set(TARGET_RP2040)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(opencc pico_stdlib)
target_compile_features(opencc PUBLIC cxx_std_14)

# enable usb output, disable uart output
pico_enable_stdio_usb(opencc 1)
pico_enable_stdio_uart(opencc 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(opencc)