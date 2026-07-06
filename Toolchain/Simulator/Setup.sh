#!/bin/bash

PACKAGES=""

dpkg -s libsdl2-dev >/dev/null 2>&1 || PACKAGES="libsdl2-dev"
dpkg -s gdb >/dev/null 2>&1 || PACKAGES="$PACKAGES gdb"
dpkg -s dosfstools >/dev/null 2>&1 || PACKAGES="$PACKAGES dosfstools"
dpkg -s mtools >/dev/null 2>&1 || PACKAGES="$PACKAGES mtools"

if [ -z "$PACKAGES" ]; then
    echo "All dependencies already installed."
else
    echo "Installing:$PACKAGES"
    sudo apt-get update
    sudo apt-get install -y $PACKAGES
fi

if ! grep -q "set debuginfod enabled off" ~/.gdbinit 2>/dev/null; then
    echo "set debuginfod enabled off" >> ~/.gdbinit
    echo "GDB: disabled debuginfod in ~/.gdbinit"
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

SIZE_MB=$(du -sk "$PROJECT_ROOT/sample/sdcard/" | awk '{s=int(($1/1024)+1); if(s<1) s=1; print s}')

echo "Creating disk image: sample/sdcard.img (${SIZE_MB} MB)"
rm -f "$PROJECT_ROOT/sample/sdcard.img"
dd if=/dev/zero of="$PROJECT_ROOT/sample/sdcard.img" bs=1M count=$SIZE_MB status=none
mkfs.fat "$PROJECT_ROOT/sample/sdcard.img" >/dev/null
mcopy -i "$PROJECT_ROOT/sample/sdcard.img" "$PROJECT_ROOT/sample/sdcard/"* ::
echo "Disk image created successfully."
