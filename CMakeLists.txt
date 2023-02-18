cmake_minimum_required(VERSION 3.13)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(src/Dependency/pico-sdk/pico_sdk_init.cmake)

project(opencc)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project