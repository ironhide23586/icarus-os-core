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
# From build/ directory:
cppcheck-htmlreport --file=./cppcheck_report.xml \
  --report-dir=./cppcheck_html \
  --source-dir=.
```

**Note:** The cppcheck configuration suppresses "missingInclude" warnings for vendor HAL/USB headers, as these are expected in embedded code and don't indicate real issues.

**Clean cppcheck reports:**
```bash
make clean-cppcheck
# or clean everything
make clean-all
```

**Note:** cppcheck analyzes only application code (`Core/Src/`), not vendor HAL drivers, to reduce false positives.

## Code Coverage (DO-178C)

### gcov/lcov Integration

The Makefile includes code coverage support using GCC's `gcov` and `lcov` for generating coverage reports. This is essential for DO-178C compliance as it provides statement, branch, and function coverage metrics.

**Install lcov:**
```bash
# macOS
brew install lcov

# Ubuntu/Debian
sudo apt-get install lcov
```

**⚠️ Important:** Coverage instrumentation significantly increases code size and **cannot be used for embedded target builds**. Coverage is only for **host-based unit tests**.

**Build with coverage enabled (for tests only):**
```bash
# When test framework is set up, use:
# make TEST=yes COVERAGE=yes clean all

# For now, coverage requires a unit testing framework to be set up first.
# See "Unit Testing" section below.
```

**After running tests:**
```bash
# Generate coverage report
make coverage

# Generate HTML coverage report
make coverage-html

# View summary
make coverage-summary
```

**Coverage Workflow:**
1. Build with `COVERAGE=yes` to instrument code with coverage markers
2. Run your test suite (unit tests, integration tests, etc.)
3. Generate coverage report with `make coverage` or `make coverage-html`
4. Review coverage metrics and identify untested code

**Output:**
- Coverage info file: `build/coverage/coverage.info`
- HTML report: `build/coverage/html/index.html`
- Coverage data files: `.gcda` and `.gcno` files in `build/obj/`

**Coverage Features:**
- **Statement Coverage**: Tracks which lines of code are executed
- **Branch Coverage**: Tracks which branches (if/else, switch) are taken
- **Function Coverage**: Tracks which functions are called
- **Excludes Vendor Code**: Only analyzes application code (`Core/Src/`)

**Clean coverage files:**
```bash
make clean-coverage
# or clean everything
make clean-all
```

**Important Notes:**
- ⚠️ **Coverage cannot be used for embedded target builds** - it makes code too large for flash
- Coverage requires `-O0` optimization (automatically set when `COVERAGE=yes`)
- Coverage is **only** for host-based unit tests, not target execution
- `.gcda` files are generated at runtime when instrumented code executes
- You must set up a unit testing framework first (Unity, CppUTest, etc.)
- Use `TEST=yes COVERAGE=yes` when building tests (prevents accidental target builds)

**DO-178C Coverage Requirements:**
- **Level A**: 100% statement coverage + Modified Condition/Decision Coverage (MC/DC)
- **Level B**: 100% statement coverage + decision coverage
- **Level C**: 100% statement coverage
- **Level D**: Statement coverage (target: 100%)

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
- `make coverage`: Generate coverage report (requires COVERAGE=yes build)
- `make coverage-html`: Generate HTML coverage report
- `make coverage-summary`: Show coverage summary
- `make clean-coverage`: Remove coverage files
- `make clean-all`: Clean everything including cppcheck and coverage

## Differences from STM32CubeIDE Build

This standalone Makefile:
- Works without IDE dependencies
- Uses relative paths (works from any location)
- Automatically discovers source files
- Generates dependency files for incremental builds
- Provides clean, flash, and size targets
- Includes enhanced compiler warnings for safety-critical code
- Integrates cppcheck static analysis
- Supports code coverage with gcov/lcov for DO-178C compliance
