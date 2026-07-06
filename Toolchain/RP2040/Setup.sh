#!/bin/bash
set -e

echo "=== Installing RP2040 toolchain ==="
echo "--- Cloning Pico SDK ---"
if [ ! -d ~/pico-sdk ]; then
    sudo apt-get update
    sudo apt-get install -y git cmake gcc-arm-none-eabi libnewlib-arm-none-eabi \
        build-essential libstdc++-arm-none-eabi-newlib
    git clone --depth 1 https://github.com/raspberrypi/pico-sdk.git ~/pico-sdk
    cd ~/pico-sdk && git submodule update --init
else
    echo "pico-sdk already exists at ~/pico-sdk"
fi

echo "--- Adding PICO_SDK_PATH to environment ---"
grep -q 'PICO_SDK_PATH' ~/.bashrc || echo 'export PICO_SDK_PATH=$HOME/pico-sdk' >> ~/.bashrc

echo "--- Installing picotool ---"
if [ ! -d ~/picotool ]; then
    sudo apt update
    sudo apt install build-essential pkg-config libusb-1.0-0-dev cmake git
    export PICO_SDK_PATH=$HOME/pico-sdk
    cd ~
    git clone https://github.com/raspberrypi/picotool.git
    cd picotool
    mkdir build
    cd build
    cmake ..
    make
    sudo make install
    cd ~/picotool
    sudo cp udev/60-picotool.rules /etc/udev/rules.d/
    sudo udevadm control --reload-rules
else
    echo "picotool already exists at ~/picotool"
fi

echo "=== Done! Restart terminal or run: source ~/.bashrc ==="