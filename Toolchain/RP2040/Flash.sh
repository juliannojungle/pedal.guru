#!/usr/bin/env bash
# Flashes the RP2040 .uf2 over USB via picotool. Bash port of Flash.ps1 — meant
# to be run from inside WSL. It calls the Windows "usbipd.exe" (through WSL
# interop) and "picotool" (native, inside WSL), so no PowerShell is needed.
#
# Sequence:
#   attach run-mode (COM) device to WSL ->
#   picotool reboot into BOOTSEL ->
#   attach BOOTSEL (RP2 Boot) device to WSL ->
#   picotool load (the -x flag reboots the board into the app afterwards).
#
# Requires the device to be bound first (run Bind.sh / the "Setup: Bind RP2040
# (admin)" task once). attach is idempotent and does not require admin.

set -uo pipefail

RUN_PATTERN='\(COM[0-9]+\)'
BOOTSEL_PATTERN='RP2 Boot'
UF2_PATH="$HOME/pedal.guru/build/pedal.guru.uf2"

# Prints the matching "Connected:" line (CR stripped) for a device pattern, or
# nothing if not found. Only lines that start with a busid like "3-3" match;
# the "Persisted:" section starts with a GUID and is therefore ignored.
find_device_line() {
    local pattern="$1"
    usbipd.exe list 2>/dev/null | tr -d '\r' \
        | grep -E "^[[:space:]]*[0-9]+-[0-9]+[[:space:]].*${pattern}" \
        | head -n1 || true
}

device_busid() { awk '{print $1}' <<<"$1"; }

# Shared unless the STATE column reads "Not shared".
device_shared() { [[ "$1" != *"Not shared"* ]]; }
device_attached() { [[ "$1" == *"Attached"* ]]; }

wait_device() {
    local pattern="$1" label="$2" timeout="${3:-20}" i=0 line
    echo "Waiting for device ($label)..." >&2
    while ((i < timeout)); do
        line="$(find_device_line "$pattern")"
        if [[ -n "$line" ]]; then
            echo "$line"
            return 0
        fi
        sleep 1
        i=$((i + 1))
    done
    echo "Timed out waiting for device ($label)." >&2
    exit 1
}

invoke_attach() {
    local line="$1" busid
    busid="$(device_busid "$line")"
    if device_attached "$line"; then
        echo "Bus ID $busid is already attached to WSL."
        return 0
    fi
    if ! device_shared "$line"; then
        echo "Bus ID $busid is not shared yet. Run the \"Setup: Bind RP2040 (admin)\" task once, then retry." >&2
        exit 1
    fi
    echo "Attaching bus ID $busid to WSL..."
    usbipd.exe attach --wsl --busid="$busid" \
        || { echo "usbipd attach failed for bus ID $busid." >&2; exit 1; }
    # Give WSL a moment to enumerate the device before picotool talks to it.
    sleep 2
}

# --- Attach run mode (COM) ---
line="$(find_device_line "$RUN_PATTERN")"
if [[ -z "$line" ]]; then
    echo "RP2040 not found in run mode. Make sure the board is connected and try again." >&2
    exit 1
fi
echo "Found RP2040 (run mode) at bus ID: $(device_busid "$line")"
invoke_attach "$line"

# --- Reboot into BOOTSEL ---
echo "Rebooting RP2040 into BOOTSEL..."
picotool reboot -u -f || { echo "picotool reboot into BOOTSEL failed." >&2; exit 1; }

# --- Attach BOOTSEL (RP2 Boot) ---
line="$(wait_device "$BOOTSEL_PATTERN" "BOOTSEL / RP2 Boot")"
echo "Found RP2040 (BOOTSEL) at bus ID: $(device_busid "$line")"
invoke_attach "$line"

# --- Load the firmware (-x reboots into the app afterwards) ---
echo "Loading firmware..."
picotool load -v -x "$UF2_PATH" || { echo "picotool load failed." >&2; exit 1; }

echo "Done. Firmware flashed and RP2040 rebooted into the application."
