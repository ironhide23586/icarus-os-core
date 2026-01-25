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

The Makefile includes a `flash` target, but you need to configure it for your programmer:

```bash
make flash
```

Currently, it's a placeholder. You can modify the `flash` target in the Makefile to use:
- `st-flash` (ST-Link)
- `openocd`
- `JLinkExe` (SEGGER J-Link)
- Or your preferred flashing tool

Example modification for st-flash:
```makefile
flash: $(HEX_FILE)
	st-flash write $(HEX_FILE) 0x08000000
```

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

## Differences from STM32CubeIDE Build

This standalone Makefile:
- Works without IDE dependencies
- Uses relative paths (works from any location)
- Automatically discovers source files
- Generates dependency files for incremental builds
- Provides clean, flash, and size targets
