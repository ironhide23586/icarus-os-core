#!/bin/bash

################################################################################
# Flash script for ICARUS OS
# Flashes the .elf file to STM32H750 via DFU mode
################################################################################

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Project configuration
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT="icarus_os"
ELF_FILE="${BUILD_DIR}/${PROJECT}.elf"
BIN_FILE="${BUILD_DIR}/${PROJECT}.bin"
DFU_VID="0483"  # STMicroelectronics
DFU_PID="df11"  # DFU mode PID

# Function to print colored messages
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if dfu-util is installed
check_dfu_util() {
    if ! command -v dfu-util &> /dev/null; then
        print_error "dfu-util is not installed"
        echo "Install it with:"
        echo "  macOS:   brew install dfu-util"
        echo "  Ubuntu:  sudo apt-get install dfu-util"
        exit 1
    fi
}

# Check if device is in DFU mode
check_dfu_mode() {
    print_info "Checking for DFU device..."
    
    if dfu-util -l 2>/dev/null | grep -q "Found DFU"; then
        print_info "DFU device found"
        dfu-util -l
        return 0
    else
        print_error "No DFU device found"
        echo ""
        print_warn "Please put your device in DFU mode:"
        echo "  1. Hold BOOT button"
        echo "  2. Press and release RESET button"
        echo "  3. Release BOOT button"
        echo ""
        echo "Or use: stm32-dfu-util --enter-dfu"
        exit 1
    fi
}

# Convert ELF to BIN if needed
convert_elf_to_bin() {
    if [ ! -f "${ELF_FILE}" ]; then
        print_error "ELF file not found: ${ELF_FILE}"
        echo "Build the project first with: make"
        exit 1
    fi
    
    if [ ! -f "${BIN_FILE}" ] || [ "${ELF_FILE}" -nt "${BIN_FILE}" ]; then
        print_info "Converting ELF to BIN..."
        arm-none-eabi-objcopy -O binary "${ELF_FILE}" "${BIN_FILE}"
        if [ $? -ne 0 ]; then
            print_error "Failed to convert ELF to BIN"
            exit 1
        fi
        print_info "BIN file created: ${BIN_FILE}"
    else
        print_info "BIN file is up to date"
    fi
}

# Flash the device
flash_device() {
    print_info "Flashing ${BIN_FILE} to device..."
    echo ""
    
    # Flash using dfu-util
    # -a 0: Alternate setting 0 (internal flash)
    # -s 0x08000000: Start address (STM32H750 flash start)
    # -D: Download file
    dfu-util -a 0 -s 0x08000000 -D "${BIN_FILE}"
    
    if [ $? -eq 0 ]; then
        print_info "Flash successful!"
        reset_device
    else
        print_error "Flash failed"
        exit 1
    fi
}

# Reset the device
reset_device() {
    print_info "Resetting device..."
    
    # Method 1: Try to detach and reset using dfu-util
    # The -R flag attempts to reset, but may timeout (which is often harmless)
    # We suppress stderr since timeout errors are common but device usually resets anyway
    if dfu-util -a 0 -s 0x08000000 -R 2>/dev/null; then
        print_info "Device reset complete - firmware should now be running"
    else
        # Timeout is common - device may have already reset
        # Try alternative: just detach (device will exit DFU mode)
        print_info "Attempting alternative reset method..."
        dfu-util -a 0 -e 2>/dev/null || true
        
        print_warn "Reset command had timeout (this is common and usually harmless)"
        print_info "Device should have reset - check if firmware is running"
        print_info "If not, manually reset the device (press RESET button)"
    fi
    
    # Small delay to allow device to exit DFU mode
    sleep 0.5
}

# Main execution
main() {
    echo "=========================================="
    echo "  ICARUS OS Flash Utility"
    echo "=========================================="
    echo ""
    
    check_dfu_util
    check_dfu_mode
    convert_elf_to_bin
    flash_device
    
    echo ""
    print_info "Done!"
}

# Run main function
main "$@"
