#!/bin/bash
set -e

echo "=== Installing ESP32 toolchain ==="

sudo apt-get update
sudo apt-get install -y git wget flex bison gperf cmake ninja-build ccache \
    libffi-dev libssl-dev dfu-util libusb-1.0-0

echo "--- Checking Python3 ---"
if ! command -v python3 &>/dev/null; then
    echo "Python3 not found, installing..."
    sudo apt-get install -y python3 python3-pip python3-venv
else
    echo "Python3 found: $(python3 --version)"
fi

echo "--- Cloning ESP-IDF (v5.3 stable) ---"
if [ ! -d ~/esp-idf ]; then
    git clone --depth 1 --branch v5.3 https://github.com/espressif/esp-idf.git ~/esp-idf
else
    echo "esp-idf already exists at ~/esp-idf"
fi

echo "--- Installing ESP-IDF tools (xtensa toolchain, gdb, openocd, etc) ---"
python3 ~/esp-idf/tools/idf_tools.py install

echo "--- Installing ESP-IDF Python dependencies ---"
python3 ~/esp-idf/tools/idf_tools.py install-python-env

echo "--- Adding ESP-IDF to environment ---"
grep -q 'esp-idf/export.sh' ~/.bashrc || echo '. $HOME/esp-idf/export.sh' >> ~/.bashrc

echo "--- Installing espflash (Rust-based, no Python) ---"
if command -v espflash &>/dev/null; then
    echo "espflash already installed: $(espflash --version)"
else
    if ! command -v cargo &>/dev/null; then
        echo "Rust not found, installing..."
        curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
        source "$HOME/.cargo/env"
    fi
    cargo install espflash
fi

echo "=== Done! Restart terminal or run: source ~/.bashrc ==="
