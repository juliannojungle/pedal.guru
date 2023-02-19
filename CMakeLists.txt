cmake_minimum_required(VERSION 3.13)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(src/Dependency/pico-sdk/pico_sdk_init.cmake)

project(opencc)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(opencc
    OpenCC.cpp
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(opencc pico_stdlib)

# enable usb output, disable uart output
pico_enable_stdio_usb(opencc 1)
pico_enable_stdio_uart(opencc 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(opencc)

# add url via pico_set_program_url
example_auto_set_url(opencc)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(opencc)