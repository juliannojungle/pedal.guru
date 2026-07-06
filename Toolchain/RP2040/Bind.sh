#!/usr/bin/env bash
# One-time setup: shares the RP2040 with WSL via usbipd, in BOTH USB modes.
# Bash port of Bind.ps1 — meant to be run from inside WSL.
#
# It calls the Windows "usbipd.exe" (through WSL interop) and "picotool"
# (native, inside WSL), so no PowerShell scripting is needed. The only step
# that still touches Windows elevation is "usbipd bind", which requires
# administrator rights: it is raised through a single UAC prompt, the only
# mechanism able to elevate from a non-elevated shell.
#
# The RP2040 enumerates as two different USB devices:
#   - Run mode:     "...USB (COMx)..."                  -> matched by '\(COM\d+\)'
#   - BOOTSEL mode: "USB Mass Storage Device, RP2 Boot" -> matched by 'RP2 Boot'
#
# Each has a distinct VID:PID, so each needs its own usbipd bind. picotool runs
# inside WSL and can only see the board once it is attached, so the flow is:
#   bind+attach (run mode) -> picotool reboot into BOOTSEL ->
#   bind+attach (BOOTSEL)  -> picotool reboot back to run mode.
#
# bind requires administrator rights and is persistent across reboots, so it is
# skipped when the device is already shared. attach does not require admin.

set -uo pipefail

RUN_PATTERN='\(COM[0-9]+\)'
BOOTSEL_PATTERN='RP2 Boot'

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

invoke_bind() {
    local busid="$1"
    echo "Binding bus ID $busid (requires administrator)..."
    # Raise a UAC prompt to run the privileged "usbipd bind". This is the only
    # step that needs Windows elevation; everything else is plain bash.
    powershell.exe -NoProfile -Command \
        "Start-Process usbipd -Verb RunAs -Wait -ArgumentList 'bind','--busid=$busid'"
}

ensure_shared() {
    local line="$1" busid
    busid="$(device_busid "$line")"
    if device_shared "$line"; then
        echo "Bus ID $busid is already shared; skipping bind."
    else
        invoke_bind "$busid"
    fi
}

invoke_attach() {
    local busid="$1"
    echo "Attaching bus ID $busid to WSL..."
    usbipd.exe attach --wsl --busid="$busid" \
        || { echo "usbipd attach failed for bus ID $busid." >&2; exit 1; }
    # Give WSL a moment to enumerate the device before picotool talks to it.
    sleep 2
}

# --- Run mode (COM) ---
line="$(find_device_line "$RUN_PATTERN")"
if [[ -z "$line" ]]; then
    echo "RP2040 not found in run mode. Make sure the board is connected and try again." >&2
    exit 1
fi
busid="$(device_busid "$line")"
echo "Found RP2040 (run mode) at bus ID: $busid"
ensure_shared "$line"
invoke_attach "$busid"

# --- Reboot into BOOTSEL ---
echo "Rebooting RP2040 into BOOTSEL..."
picotool reboot -u -f || { echo "picotool reboot into BOOTSEL failed." >&2; exit 1; }

# --- BOOTSEL mode (RP2 Boot) ---
line="$(wait_device "$BOOTSEL_PATTERN" "BOOTSEL / RP2 Boot")"
busid="$(device_busid "$line")"
echo "Found RP2040 (BOOTSEL) at bus ID: $busid"
ensure_shared "$line"
invoke_attach "$busid"

# --- Reboot back to run mode ---
echo "Rebooting RP2040 back to run mode..."
picotool reboot -a -f || { echo "picotool reboot back to run mode failed." >&2; exit 1; }

echo "Done. The RP2040 is now shared with WSL in both run and BOOTSEL modes."
