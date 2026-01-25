#!/bin/bash

################################################################################
# Rebuild and Flash script for ICARUS OS
# Cleans, rebuilds the project, and then flashes it to the device
################################################################################

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

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

# Check if we're in the build directory
if [ ! -f "${SCRIPT_DIR}/Makefile" ]; then
    print_error "Makefile not found. This script must be run from the build/ directory"
    exit 1
fi

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
        print_info "DFU device found - ready to flash"
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
        return 1
    fi
}

# Rebuild the project
rebuild_project() {
    print_info "Rebuilding project (clean + build)..."
    echo ""
    
    cd "${SCRIPT_DIR}"
    
    if make rebuild; then
        print_info "Rebuild successful!"
        echo ""
        return 0
    else
        print_error "Rebuild failed!"
        exit 1
    fi
}

# Flash the device
flash_device() {
    print_info "Flashing device..."
    echo ""
    
    cd "${SCRIPT_DIR}"
    
    if [ ! -f "./flash.sh" ]; then
        print_error "flash.sh not found in build directory"
        exit 1
    fi
    
    # Check for test mode flag
    if [ "$1" == "--test" ] || [ "$1" == "-t" ]; then
        print_warn "Running in test mode (no actual flashing)"
        ./flash.sh --test
    else
        ./flash.sh
    fi
}

# Main execution
main() {
    echo "=========================================="
    echo "  ICARUS OS Rebuild & Flash Utility"
    echo "=========================================="
    echo ""
    
    # Parse arguments
    TEST_MODE=0
    if [ "$1" == "--test" ] || [ "$1" == "-t" ]; then
        TEST_MODE=1
        print_warn "TEST MODE: Will rebuild but not actually flash"
        echo ""
    else
        # Check DFU mode before rebuilding (skip in test mode)
        check_dfu_util
        if ! check_dfu_mode; then
            exit 1
        fi
        echo ""
    fi
    
    # Rebuild
    if ! rebuild_project; then
        exit 1
    fi
    
    # Flash
    if [ $TEST_MODE -eq 1 ]; then
        flash_device --test
    else
        flash_device
    fi
    
    echo ""
    print_info "Rebuild and flash complete!"
}

# Run main function
main "$@"
