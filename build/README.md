# Standalone Build System for ICARUS OS

This directory contains a standalone Makefile that allows you to build the ICARUS OS project without STM32CubeIDE.

## Prerequisites

1. **ARM GCC Toolchain**: Install GNU Tools for STM32 (arm-none-eabi-gcc)
   - Download from: https://developer.arm.com/downloads/-/gnu-rm
   - Or install via package manager:
     - macOS: `brew install arm-none-eabi-gcc`
     - Ubuntu/Debian: `sudo apt-get install gcc-arm-none-eabi`
     - Windows: Use the installer from ARM website

2. **Make**: Should be pre-installed on macOS/Linux. On Windows, use WSL or install via MSYS2.

## Building

From the `build/` directory:

```bash
cd build
make
```

This will:
- Compile all source files
- Link the ELF binary
- Generate HEX and BIN files
- Display size information

## Build and Flash

To build and flash in one command:

```bash
cd build
./build_and_flash.sh
# or
make build-and-flash
```

This will:
- Build the project
- Flash it to the device (if in DFU mode)
- Reset the device automatically

**Test mode** (builds but doesn't flash):
```bash
./build_and_flash.sh --test
```

## Rebuild and Flash

To clean, rebuild, and flash in one command:

```bash
cd build
./rebuild_and_flash.sh
# or
make rebuild-and-flash
```

This will:
- Clean all build artifacts
- Rebuild the project from scratch
- Flash it to the device (if in DFU mode)
- Reset the device automatically

**Test mode** (rebuilds but doesn't flash):
```bash
./rebuild_and_flash.sh --test
```

## Build Output

The build artifacts are placed in the `build/` directory:

- `icarus_os.elf` - ELF binary (for debugging)
- `icarus_os.hex` - Intel HEX format (for flashing)
- `icarus_os.bin` - Binary format (for flashing)
- `icarus_os.map` - Memory map
- `icarus_os.list` - Disassembly listing
- `obj/` - Object files
- `dep/` - Dependency files

## Cleaning

To clean all build artifacts:

```bash
make clean
```

## Flashing

The project includes a flash script for DFU mode:

```bash
# Using the script directly
./flash.sh

# Or using make
make flash
```

**Prerequisites:**
- Install `dfu-util`: 
  - macOS: `brew install dfu-util`
  - Ubuntu/Debian: `sudo apt-get install dfu-util`

**Put device in DFU mode:**
1. Hold BOOT button
2. Press and release RESET button
3. Release BOOT button

The script will:
- Check if `dfu-util` is installed
- Verify device is in DFU mode
- Convert ELF to BIN if needed
- Flash to STM32H750 at address 0x08000000

**Alternative flashing methods:**
- ST-Link: `st-flash write build/icarus_os.hex 0x08000000`
- OpenOCD: Configure for your setup
- J-Link: Use SEGGER J-Flash

## Project Structure

The Makefile automatically finds all source files from:
- `../Core/Src/` - Application and kernel code
- `../Core/Src/bsp/` - Board support package
- `../Core/Src/kernel/` - Kernel implementation
- `../Drivers/` - STM32 HAL drivers
- `../Middlewares/` - USB Device Library
- `../USB_DEVICE/` - USB configuration

## Compiler Flags

The Makefile uses optimized settings for STM32H750:
- CPU: Cortex-M7
- FPU: FPv5-D16 (hard float)
- Optimization: -O2
- Link-time garbage collection enabled

### Enhanced Warnings (DO-178C Preparation)

The Makefile includes comprehensive warning flags for safety-critical code:
- `-Wall -Wextra -Wpedantic`: Standard and extra warnings
- `-Wstrict-prototypes`: Enforce function prototypes
- `-Wmissing-prototypes`: Warn about missing prototypes
- `-Wuninitialized`: Detect uninitialized variables
- `-Wconversion -Wsign-conversion`: Warn about type conversions
- `-Wformat=2`: Enhanced format string checking
- `-Wshadow`: Warn about variable shadowing
- And many more...

**By default, warnings are treated as errors** (`-Werror`). To disable this:
```bash
make WERROR=no
```

## Static Analysis

### cppcheck Integration

The Makefile includes cppcheck static analysis for detecting bugs, undefined behavior, and code quality issues.

**Install cppcheck:**
```bash
# macOS
brew install cppcheck

# Ubuntu/Debian
sudo apt-get install cppcheck
```

**Run static analysis:**
```bash
# Run cppcheck only
make cppcheck

# Run cppcheck and then build
make check-build
```

**Output:**
- XML report: `build/cppcheck_report.xml`
- Text output: `build/cppcheck_output.txt`

**View HTML report (optional):**
```bash
# Install cppcheck-htmlreport (if available)
cppcheck-htmlreport --file=build/cppcheck_report.xml \
  --report-dir=build/cppcheck_html \
  --source-dir=..
```

**Clean cppcheck reports:**
```bash
make clean-cppcheck
# or clean everything
make clean-all
```

**Note:** cppcheck analyzes only application code (`Core/Src/`), not vendor HAL drivers, to reduce false positives.

## Troubleshooting

### "arm-none-eabi-gcc: command not found"
- Ensure the ARM toolchain is installed and in your PATH
- Verify with: `arm-none-eabi-gcc --version`

### Linker script not found
- Ensure `STM32H750VBTX_FLASH.ld` exists in the project root
- The Makefile expects it at `../STM32H750VBTX_FLASH.ld`

### Build fails with include errors
- Check that all include directories are correct
- Verify source files exist in expected locations

### Out of memory errors
- The linker script defines memory regions for STM32H750
- Check that your code fits within available flash/RAM

## Makefile Targets

Available targets:
- `make` or `make all`: Build the project
- `make clean`: Remove build artifacts
- `make rebuild`: Clean and rebuild
- `make size`: Show memory usage
- `make flash`: Flash to device (requires DFU mode)
- `make build-and-flash`: Build and flash
- `make rebuild-and-flash`: Clean, rebuild, and flash
- `make cppcheck`: Run static analysis
- `make check-build`: Run static analysis then build
- `make clean-cppcheck`: Remove cppcheck reports
- `make clean-all`: Clean everything including cppcheck

## Differences from STM32CubeIDE Build

This standalone Makefile:
- Works without IDE dependencies
- Uses relative paths (works from any location)
- Automatically discovers source files
- Generates dependency files for incremental builds
- Provides clean, flash, and size targets
- Includes enhanced compiler warnings for safety-critical code
- Integrates cppcheck static analysis
