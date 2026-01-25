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

# Signal handler for Ctrl+C during flash - tracks attempts
flash_interrupted() {
    # Read and increment count from file (persists across signal handler calls)
    local current_count=0
    if [ -f /tmp/flash_interrupt_count.$$ ]; then
        current_count=$(cat /tmp/flash_interrupt_count.$$ 2>/dev/null || echo "0")
    fi
    current_count=$((current_count + 1))
    echo $current_count > /tmp/flash_interrupt_count.$$
    
    echo ""
    echo ""
    
    if [ $current_count -lt 3 ]; then
        # First two attempts - warn but continue (signal is ignored by parent)
        print_warn "Ctrl+C pressed (attempt $current_count/3) - IGNORED"
        print_error "Flash operation in progress - interruption may corrupt device!"
        print_warn "Press Ctrl+C 3 times total to force exit (NOT RECOMMENDED)"
        echo ""
        print_info "Continuing flash operation..."
        echo ""
        # Signal is ignored by parent trap - dfu-util won't see it
        return
    else
        # Third attempt - allow signal through to kill dfu-util
        print_error "Flash operation FORCE INTERRUPTED after 3 Ctrl+C presses!"
        print_warn "WARNING: Interrupting flash may corrupt the device firmware!"
        print_warn "DO NOT disconnect the device"
        echo ""
        print_info "Attempting to clean up..."
        
        # Kill any remaining dfu-util processes (only if not in test mode)
        if [ -z "$TEST_MODE" ]; then
            pkill -f "dfu-util.*-D" 2>/dev/null || true
        fi
        
        print_warn "Device may be in an unstable state"
        print_info "To recover: put device back in DFU mode and flash again"
        echo ""
        
        # Clean up temp file
        rm -f /tmp/flash_interrupt_count.$$ 2>/dev/null || true
        
        # Restore default handler and allow signal through
        trap - INT
        # Send SIGINT to ourselves to actually exit
        kill -INT $$ 2>/dev/null || exit 1
    fi
}

# Flash the device
flash_device() {
    print_info "Flashing ${BIN_FILE} to device..."
    print_warn "DO NOT press Ctrl+C during flash - it may corrupt the device!"
    print_info "Press Ctrl+C 3 times to force exit (NOT RECOMMENDED)"
    echo ""
    
    # Clean up any old temp files
    rm -f /tmp/flash_interrupt_count.$$ 2>/dev/null || true
    echo "0" > /tmp/flash_interrupt_count.$$
    
    # Set up handler to track Ctrl+C presses
    trap 'flash_interrupted' INT
    
    # Run dfu-util in a new process group to better control signals
    # Use setsid or run in subshell with ignored signal
    (
        # In subshell: ignore SIGINT so dfu-util doesn't see it
        # Parent handler will still catch it and increment counter
        trap '' INT
        dfu-util -a 0 -s 0x08000000 -D "${BIN_FILE}"
    ) &
    local dfu_pid=$!
    
    # Monitor dfu-util and check for interrupts
    while kill -0 $dfu_pid 2>/dev/null; do
        # Check interrupt count
        if [ -f /tmp/flash_interrupt_count.$$ ]; then
            local count=$(cat /tmp/flash_interrupt_count.$$ 2>/dev/null || echo "0")
            if [ "$count" -ge 3 ]; then
                # 3rd press - kill dfu-util and exit
                print_warn "Force interrupting flash after 3 Ctrl+C presses..."
                kill $dfu_pid 2>/dev/null
                kill -9 $dfu_pid 2>/dev/null
                wait $dfu_pid 2>/dev/null
                flash_status=1
                break
            fi
        fi
        # Brief sleep to avoid busy-waiting
        sleep 0.1
    done
    
    # Get final status
    if [ -z "$flash_status" ]; then
        wait $dfu_pid 2>/dev/null
        flash_status=$?
    fi
    
    # Check if we were force interrupted
    if [ -f /tmp/flash_interrupt_count.$$ ]; then
        local saved_count=$(cat /tmp/flash_interrupt_count.$$ 2>/dev/null || echo "0")
        if [ "$saved_count" -ge 3 ]; then
            flash_status=1
        fi
    fi
    
    # Clean up
    rm -f /tmp/flash_interrupt_count.$$ 2>/dev/null || true
    trap - INT
    
    if [ $flash_status -eq 0 ]; then
        print_info "Flash successful!"
        reset_device
    else
        if [ -f /tmp/flash_interrupt_count.$$ ] && [ "$(cat /tmp/flash_interrupt_count.$$ 2>/dev/null)" -ge 3 ] 2>/dev/null; then
            print_error "Flash was interrupted by user"
        else
            print_error "Flash failed"
        fi
        exit 1
    fi
}

# Reset the device
reset_device() {
    print_info "Resetting device..."
    
    # Allow Ctrl+C during reset (less critical than during flash)
    # But still provide a handler
    trap 'echo ""; print_warn "Reset interrupted - device may need manual reset"; exit 1' INT
    
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
    
    # Restore default interrupt handler
    trap - INT
    
    # Small delay to allow device to exit DFU mode
    sleep 0.5
}

# Test mode - simulates flash without actually writing to device
test_mode() {
    export TEST_MODE=1
    echo "=========================================="
    echo "  ICARUS OS Flash Utility - TEST MODE"
    echo "=========================================="
    echo ""
    print_warn "TEST MODE: No actual flashing will occur"
    print_info "This mode tests the Ctrl+C handler safely"
    echo ""
    
    # Check if ELF exists
    if [ ! -f "${ELF_FILE}" ]; then
        print_error "ELF file not found: ${ELF_FILE}"
        echo "Build the project first with: make"
        exit 1
    fi
    
    convert_elf_to_bin
    
    print_info "Simulating flash operation..."
    print_warn "DO NOT press Ctrl+C during flash - it may corrupt the device!"
    print_info "Press Ctrl+C 3 times to force exit (NOT RECOMMENDED)"
    echo ""
    
    # Initialize interrupt counter
    INTERRUPT_COUNT=0
    
    # Trap SIGINT to test the handler
    trap flash_interrupted INT
    
    # Simulate flash with a long sleep (you can press Ctrl+C here to test)
    print_info "Simulating flash (15 seconds) - try pressing Ctrl+C to test..."
    for i in {1..15}; do
        echo -n "."
        sleep 1
    done
    echo ""
    
    # Restore handler
    trap - INT
    INTERRUPT_COUNT=0
    
    print_info "Simulated flash complete!"
    print_info "In real mode, device would be reset now"
    echo ""
    unset TEST_MODE
}

# Main execution
main() {
    # Check for test mode flag
    if [ "$1" == "--test" ] || [ "$1" == "-t" ]; then
        test_mode
        return
    fi
    
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
