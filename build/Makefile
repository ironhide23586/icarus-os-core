################################################################################
# Makefile for ICARUS OS Core
# Standalone build system - works without STM32CubeIDE
# Target: STM32H750VBTx (Cortex-M7)
################################################################################

# Project name
PROJECT = icarus_os

# Toolchain
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc
LD = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE = $(PREFIX)size

# Project root directory (relative to build/)
PROJECT_ROOT = ..

# Build directories
BUILD_DIR = .
OBJ_DIR = $(BUILD_DIR)/obj
DEP_DIR = $(BUILD_DIR)/dep

# Linker script
LINKER_SCRIPT = $(PROJECT_ROOT)/STM32H750VBTX_FLASH.ld

# Source directories
CORE_SRC = $(PROJECT_ROOT)/Core/Src
CORE_BSP_SRC = $(PROJECT_ROOT)/Core/Src/bsp
CORE_KERNEL_SRC = $(PROJECT_ROOT)/Core/Src/kernel
CORE_STARTUP = $(PROJECT_ROOT)/Core/Startup
DRIVERS_HAL = $(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Src
DRIVERS_BSP = $(PROJECT_ROOT)/Drivers/BSP/ST7735
USB_DEVICE_APP = $(PROJECT_ROOT)/USB_DEVICE/App
USB_DEVICE_TARGET = $(PROJECT_ROOT)/USB_DEVICE/Target
MIDDLEWARE_USB_CORE = $(PROJECT_ROOT)/Middlewares/ST/STM32_USB_Device_Library/Core/Src
MIDDLEWARE_USB_CDC = $(PROJECT_ROOT)/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src

# Include directories
CORE_INC = $(PROJECT_ROOT)/Core/Inc
CORE_BSP_INC = $(PROJECT_ROOT)/Core/Inc/bsp
CORE_KERNEL_INC = $(PROJECT_ROOT)/Core/Inc/kernel
DRIVERS_HAL_INC = $(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Inc
DRIVERS_HAL_LEGACY_INC = $(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy
DRIVERS_CMSIS_DEVICE = $(PROJECT_ROOT)/Drivers/CMSIS/Device/ST/STM32H7xx/Include
DRIVERS_CMSIS_INC = $(PROJECT_ROOT)/Drivers/CMSIS/Include
DRIVERS_BSP_INC = $(PROJECT_ROOT)/Drivers/BSP/ST7735
USB_DEVICE_APP_INC = $(PROJECT_ROOT)/USB_DEVICE/App
USB_DEVICE_TARGET_INC = $(PROJECT_ROOT)/USB_DEVICE/Target
MIDDLEWARE_USB_CORE_INC = $(PROJECT_ROOT)/Middlewares/ST/STM32_USB_Device_Library/Core/Inc
MIDDLEWARE_USB_CDC_INC = $(PROJECT_ROOT)/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc

# Collect all source files
C_SOURCES = \
	$(wildcard $(CORE_SRC)/*.c) \
	$(wildcard $(CORE_BSP_SRC)/*.c) \
	$(wildcard $(CORE_KERNEL_SRC)/*.c) \
	$(wildcard $(DRIVERS_HAL)/*.c) \
	$(wildcard $(DRIVERS_BSP)/*.c) \
	$(wildcard $(USB_DEVICE_APP)/*.c) \
	$(wildcard $(USB_DEVICE_TARGET)/*.c) \
	$(wildcard $(MIDDLEWARE_USB_CORE)/*.c) \
	$(wildcard $(MIDDLEWARE_USB_CDC)/*.c)

ASM_SOURCES = \
	$(CORE_STARTUP)/startup_stm32h750vbtx.s \
	$(wildcard $(CORE_KERNEL_SRC)/*.s)

# Object files
C_OBJECTS = $(C_SOURCES:$(PROJECT_ROOT)/%.c=$(OBJ_DIR)/%.o)
ASM_OBJECTS = $(ASM_SOURCES:$(PROJECT_ROOT)/%.s=$(OBJ_DIR)/%.o)
ALL_OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

# Dependency files
DEP_FILES = $(C_SOURCES:$(PROJECT_ROOT)/%.c=$(DEP_DIR)/%.d)

# Include paths - use -isystem for vendor code to suppress warnings
INCLUDES = \
	-I$(CORE_INC) \
	-I$(CORE_BSP_INC) \
	-I$(CORE_KERNEL_INC) \
	-isystem $(DRIVERS_HAL_INC) \
	-isystem $(DRIVERS_HAL_LEGACY_INC) \
	-isystem $(DRIVERS_CMSIS_DEVICE) \
	-isystem $(DRIVERS_CMSIS_INC) \
	-isystem $(DRIVERS_BSP_INC) \
	-isystem $(USB_DEVICE_APP_INC) \
	-isystem $(USB_DEVICE_TARGET_INC) \
	-isystem $(MIDDLEWARE_USB_CORE_INC) \
	-isystem $(MIDDLEWARE_USB_CDC_INC)

# Compiler flags
MCU = -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard

# Enhanced warning flags for safety-critical code (DO-178C preparation)
# -Wall: Enable all standard warnings
# -Wextra: Enable extra warnings
# -Wpedantic: Strict ISO C compliance
# -Werror: Treat warnings as errors (can be disabled with WERROR=no)
# -Wstrict-prototypes: Warn about non-prototyped functions
# -Wmissing-prototypes: Warn about missing function prototypes
# -Wmissing-declarations: Warn about missing declarations
# -Wuninitialized: Warn about uninitialized variables
# -Wconversion: Warn about implicit conversions
# -Wsign-conversion: Warn about sign conversions
# -Wcast-qual: Warn about casting away qualifiers
# -Wcast-align: Warn about alignment issues
# -Wformat=2: Enhanced format string checking
# -Wundef: Warn about undefined macros
# -Wshadow: Warn about shadowed variables
# -Wpointer-arith: Warn about pointer arithmetic
# -Wbad-function-cast: Warn about function pointer casts
# -Wstrict-overflow: Warn about overflow in signed arithmetic
WERROR ?= yes
WARNING_FLAGS = -Wall \
	-Wextra \
	-Wpedantic \
	-Wstrict-prototypes \
	-Wmissing-prototypes \
	-Wmissing-declarations \
	-Wuninitialized \
	-Wconversion \
	-Wsign-conversion \
	-Wcast-qual \
	-Wcast-align \
	-Wformat=2 \
	-Wundef \
	-Wshadow \
	-Wpointer-arith \
	-Wbad-function-cast \
	-Wstrict-overflow=5

# Conditionally add -Werror
ifeq ($(WERROR),yes)
	WARNING_FLAGS += -Werror
endif

# Base CFLAGS for application code
# Using -isystem for vendor includes suppresses warnings from vendor headers
CFLAGS = $(MCU) \
	-O2 \
	$(WARNING_FLAGS) \
	-fdata-sections \
	-ffunction-sections \
	-MMD \
	-MP \
	-MF"$(@:%.o=%.d)" \
	$(INCLUDES) \
	-DUSE_HAL_DRIVER \
	-DSTM32H750xx \
	-DUSE_FLASH_ECC=0 \
	-DUSBD_USER_REGISTER_CALLBACK=0

# CFLAGS for vendor code (suppress some warnings)
VENDOR_CFLAGS = $(MCU) \
	-O2 \
	-Wall \
	-Wextra \
	-Wno-sign-conversion \
	-Wno-undef \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers \
	-fdata-sections \
	-ffunction-sections \
	-MMD \
	-MP \
	-MF"$(@:%.o=%.d)" \
	$(INCLUDES) \
	-DUSE_HAL_DRIVER \
	-DSTM32H750xx \
	-DUSE_FLASH_ECC=0 \
	-DUSBD_USER_REGISTER_CALLBACK=0

# Assembly flags
ASFLAGS = $(MCU) \
	-x assembler-with-cpp \
	-Wall \
	-MMD \
	-MP \
	-MF"$(@:%.o=%.d)" \
	$(INCLUDES) \
	-DUSE_HAL_DRIVER \
	-DSTM32H750xx

# Linker flags
LDFLAGS = $(MCU) \
	-specs=nosys.specs \
	-specs=nano.specs \
	-T$(LINKER_SCRIPT) \
	-Wl,-Map=$(BUILD_DIR)/$(PROJECT).map \
	-Wl,--gc-sections \
	-static \
	-Wl,--start-group \
	-lc \
	-lm \
	-Wl,--end-group

# Output files
ELF_FILE = $(BUILD_DIR)/$(PROJECT).elf
HEX_FILE = $(BUILD_DIR)/$(PROJECT).hex
BIN_FILE = $(BUILD_DIR)/$(PROJECT).bin
LIST_FILE = $(BUILD_DIR)/$(PROJECT).list
CPPCHECK_REPORT = $(BUILD_DIR)/cppcheck_report.xml
CPPCHECK_HTML = $(BUILD_DIR)/cppcheck_report.html

# cppcheck configuration
# Only analyze our application code, not vendor HAL drivers
CPPCHECK_SOURCES = \
	$(wildcard $(CORE_SRC)/*.c) \
	$(wildcard $(CORE_BSP_SRC)/*.c) \
	$(wildcard $(CORE_KERNEL_SRC)/*.c)

# cppcheck flags
# --enable=all: Enable all checks
# --suppress=missingIncludeSystem: Suppress warnings about system headers
# --suppress=unusedFunction: Suppress unused function warnings (common in embedded)
# --inline-suppr: Allow inline suppressions
# --xml: Output XML format
# --xml-version=2: Use XML version 2
# --platform=unix64: Platform (closest to ARM, adjust if needed)
# --std=c99: C standard
# --error-exitcode=1: Exit with error if issues found
# Note: cppcheck accepts -I flags directly, same as GCC
CPPCHECK_FLAGS = --enable=all \
	--suppress=missingIncludeSystem \
	--suppress=unusedFunction \
	--suppress=uninitStructMember:../Core/Src/bsp/lsm9ds1_reg.c \
	--suppress=uninitvar:../Core/Src/bsp/lsm9ds1_reg.c \
	--suppress=unknownMacro:../USB_DEVICE/App/usbd_desc.c \
	--suppress=constParameterPointer \
	--suppress=staticFunction \
	--inline-suppr \
	--xml \
	--xml-version=2 \
	--platform=unix64 \
	--std=c99 \
	--error-exitcode=1 \
	--force \
	$(INCLUDES)

# Default target
all: $(ELF_FILE) $(HEX_FILE) $(BIN_FILE) size

# Create object directories
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)/Core/Src/bsp
	@mkdir -p $(OBJ_DIR)/Core/Src/kernel
	@mkdir -p $(OBJ_DIR)/Core/Startup
	@mkdir -p $(OBJ_DIR)/Drivers/STM32H7xx_HAL_Driver/Src
	@mkdir -p $(OBJ_DIR)/Drivers/BSP/ST7735
	@mkdir -p $(OBJ_DIR)/USB_DEVICE/App
	@mkdir -p $(OBJ_DIR)/USB_DEVICE/Target
	@mkdir -p $(OBJ_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Src
	@mkdir -p $(OBJ_DIR)/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src

$(DEP_DIR):
	@mkdir -p $(DEP_DIR)/Core/Src/bsp
	@mkdir -p $(DEP_DIR)/Core/Src/kernel
	@mkdir -p $(DEP_DIR)/Core/Startup
	@mkdir -p $(DEP_DIR)/Drivers/STM32H7xx_HAL_Driver/Src
	@mkdir -p $(DEP_DIR)/Drivers/BSP/ST7735
	@mkdir -p $(DEP_DIR)/USB_DEVICE/App
	@mkdir -p $(DEP_DIR)/USB_DEVICE/Target
	@mkdir -p $(DEP_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Src
	@mkdir -p $(DEP_DIR)/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src

# Link ELF file
$(ELF_FILE): $(ALL_OBJECTS) | $(BUILD_DIR)
	@echo "Linking $@"
	@$(LD) $(ALL_OBJECTS) $(LDFLAGS) -o $@
	@echo "Build complete: $@"

# Compile C sources - use VENDOR_CFLAGS for vendor code, CFLAGS for application code
$(OBJ_DIR)/%.o: $(PROJECT_ROOT)/%.c | $(OBJ_DIR) $(DEP_DIR)
	@echo "Compiling $<"
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(@:$(OBJ_DIR)/%.o=$(DEP_DIR)/%.d))
	@if echo "$<" | grep -qE "(Drivers/|Middlewares/|USB_DEVICE/|lsm9ds1_reg\.c)"; then \
		$(CC) $(VENDOR_CFLAGS) -c $< -o $@; \
	else \
		$(CC) $(CFLAGS) -c $< -o $@; \
	fi

# Assemble ASM sources
$(OBJ_DIR)/%.o: $(PROJECT_ROOT)/%.s | $(OBJ_DIR) $(DEP_DIR)
	@echo "Assembling $<"
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(@:$(OBJ_DIR)/%.o=$(DEP_DIR)/%.d))
	@$(AS) $(ASFLAGS) -c $< -o $@

# Generate HEX file
$(HEX_FILE): $(ELF_FILE)
	@echo "Creating $@"
	@$(OBJCOPY) -O ihex $< $@

# Generate BIN file
$(BIN_FILE): $(ELF_FILE)
	@echo "Creating $@"
	@$(OBJCOPY) -O binary $< $@

# Generate list file
$(LIST_FILE): $(ELF_FILE)
	@echo "Creating $@"
	@$(OBJDUMP) -h -S $< > $@

# Show size information
size: $(ELF_FILE)
	@echo ""
	@echo "Size information:"
	@$(SIZE) $<
	@echo ""

# Clean build artifacts
clean:
	@echo "Cleaning..."
	@rm -rf $(OBJ_DIR)
	@rm -rf $(DEP_DIR)
	@rm -f $(ELF_FILE) $(HEX_FILE) $(BIN_FILE) $(LIST_FILE) $(BUILD_DIR)/$(PROJECT).map
	@echo "Clean complete"

# Rebuild: clean and build everything
rebuild: clean all

# Flash target (uses DFU mode via flash.sh script)
flash: $(ELF_FILE)
	@./flash.sh

# Build and flash target
build-and-flash: all
	@./build_and_flash.sh

# Rebuild and flash target
rebuild-and-flash: rebuild
	@./rebuild_and_flash.sh

# Static analysis with cppcheck
cppcheck:
	@echo "Running cppcheck static analysis..."
	@if ! command -v cppcheck >/dev/null 2>&1; then \
		echo "ERROR: cppcheck not found. Install it with:"; \
		echo "  macOS:   brew install cppcheck"; \
		echo "  Ubuntu:  sudo apt-get install cppcheck"; \
		exit 1; \
	fi
	@mkdir -p $(BUILD_DIR)
	@cppcheck $(CPPCHECK_FLAGS) --output-file=$(CPPCHECK_REPORT) $(CPPCHECK_SOURCES) 2>&1 | tee $(BUILD_DIR)/cppcheck_output.txt || true
	@echo ""
	@echo "cppcheck analysis complete. Report saved to: $(CPPCHECK_REPORT)"
	@echo "Text output saved to: $(BUILD_DIR)/cppcheck_output.txt"
	@if [ -f $(CPPCHECK_REPORT) ]; then \
		echo "To view HTML report, install cppcheck-htmlreport or use:"; \
		echo "  cppcheck-htmlreport --file=$(CPPCHECK_REPORT) --report-dir=$(BUILD_DIR)/cppcheck_html --source-dir=$(PROJECT_ROOT)"; \
	fi

# Run cppcheck and build (static analysis before compilation)
check-build: cppcheck all

# Clean cppcheck reports
clean-cppcheck:
	@echo "Cleaning cppcheck reports..."
	@rm -f $(CPPCHECK_REPORT) $(CPPCHECK_HTML) $(BUILD_DIR)/cppcheck_output.txt
	@rm -rf $(BUILD_DIR)/cppcheck_html
	@echo "cppcheck reports cleaned"

# Clean everything including cppcheck
clean-all: clean clean-cppcheck

# Include dependency files
-include $(DEP_FILES)

.PHONY: all clean rebuild size flash build-and-flash rebuild-and-flash cppcheck check-build clean-cppcheck clean-all
