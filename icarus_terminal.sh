#!/bin/bash

# ICARUS OS Terminal Launcher
# Automatically finds STM32 USB serial device and starts serial terminal

BAUD_RATE=115200

printf "\033[0m\033[2J\033[H"
# Function to find USB serial devices
find_usb_device() {
    # Look for common USB serial device patterns on macOS
    # Prefer cu.* devices over tty.* for screen
    local devices=()
    
    # Enable nullglob so unmatched patterns don't expand to literal strings
    shopt -s nullglob 2>/dev/null || true
    
    # Check for cu.usbmodem* devices (most common for STM32)
    for dev in /dev/cu.usbmodem*; do
        if [ -e "$dev" ]; then
            devices+=("$dev")
        fi
    done
    
    # If no cu.usbmodem found, check cu.usbserial*
    if [ ${#devices[@]} -eq 0 ]; then
        for dev in /dev/cu.usbserial*; do
            if [ -e "$dev" ]; then
                devices+=("$dev")
            fi
        done
    fi
    
    # If still nothing, check tty.* devices as fallback
    if [ ${#devices[@]} -eq 0 ]; then
        for dev in /dev/tty.usbmodem*; do
            if [ -e "$dev" ]; then
                devices+=("$dev")
            fi
        done
    fi
    
    # Return the most recently modified device (most likely the one just plugged in)
    if [ ${#devices[@]} -gt 0 ]; then
        # If only one device, return it
        if [ ${#devices[@]} -eq 1 ]; then
            echo "${devices[0]}"
        else
            # Multiple devices - find the most recent one
            local latest_dev=""
            local latest_time=0
            for dev in "${devices[@]}"; do
                if [[ "$OSTYPE" == "darwin"* ]]; then
                    mtime=$(stat -f "%m" "$dev" 2>/dev/null)
                else
                    mtime=$(stat -c "%Y" "$dev" 2>/dev/null)
                fi
                if [ -n "$mtime" ] && [ "$mtime" -gt "$latest_time" ]; then
                    latest_time=$mtime
                    latest_dev="$dev"
                fi
            done
            echo "$latest_dev"
        fi
    fi
}

# Find the device
DEVICE=$(find_usb_device)

if [ -z "$DEVICE" ]; then
    printf "Error: No USB serial device found.\n" >&2
    printf "\n" >&2
    printf "Ensure your STM32 board is connected via USB.\n" >&2
    printf "\n" >&2
    printf "Searched for devices:\n" >&2
    printf "  /dev/tty.usbmodem*\n" >&2
    printf "  /dev/tty.usbserial*\n" >&2
    printf "  /dev/cu.usbmodem*\n" >&2
    printf "\n" >&2

    
    # List all available serial devices for debugging
    printf "Available serial devices:\n" >&2
    found_devices=$(ls /dev/cu.* /dev/tty.* 2>/dev/null | grep -E "(usb|serial)" | head -10)
    if [ -n "$found_devices" ]; then
        printf "%s\n" "$found_devices" | sed 's/^/  /' >&2
    else
        printf "  (none found)\n" >&2
    fi
    
    exit 1
fi

printf "Found device: %s\n" "$DEVICE"

# Function to restore cursor on exit
restore_cursor() {
    printf "\033[?25h"  # Show cursor
}

# Function to cleanup and exit
cleanup_and_exit() {
    restore_cursor
    exit 0
}

# Set trap to cleanup on exit
trap cleanup_and_exit EXIT INT TERM

# Check for available terminal programs
if command -v picocom >/dev/null 2>&1; then
    printf "Starting picocom at %s baud...\n" "$BAUD_RATE"
    printf "Press Ctrl+A then Ctrl+X to exit\n\n"
    picocom -b "$BAUD_RATE" "$DEVICE"
elif command -v screen >/dev/null 2>&1; then
    printf "Starting screen at %s baud...\n" "$BAUD_RATE"
    printf "Press Ctrl+A then K to exit (Ctrl+C sends to device)\n\n"
    screen "$DEVICE" "$BAUD_RATE"
else
    printf "Error: No suitable terminal program found.\n"
    printf "Please install one of:\n"
    printf "  - picocom: brew install picocom (macOS) or sudo apt-get install picocom (Linux)\n"
    printf "  - screen: usually pre-installed\n"
    exit 1
fi

# Restore cursor after terminal exits
restore_cursor