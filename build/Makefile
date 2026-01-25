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

# Include paths
INCLUDES = \
	-I$(CORE_INC) \
	-I$(CORE_BSP_INC) \
	-I$(CORE_KERNEL_INC) \
	-I$(DRIVERS_HAL_INC) \
	-I$(DRIVERS_HAL_LEGACY_INC) \
	-I$(DRIVERS_CMSIS_DEVICE) \
	-I$(DRIVERS_CMSIS_INC) \
	-I$(DRIVERS_BSP_INC) \
	-I$(USB_DEVICE_APP_INC) \
	-I$(USB_DEVICE_TARGET_INC) \
	-I$(MIDDLEWARE_USB_CORE_INC) \
	-I$(MIDDLEWARE_USB_CDC_INC)

# Compiler flags
MCU = -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard

CFLAGS = $(MCU) \
	-O2 \
	-Wall \
	-fdata-sections \
	-ffunction-sections \
	-MMD \
	-MP \
	-MF"$(@:%.o=%.d)" \
	$(INCLUDES) \
	-DUSE_HAL_DRIVER \
	-DSTM32H750xx

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

# Compile C sources
$(OBJ_DIR)/%.o: $(PROJECT_ROOT)/%.c | $(OBJ_DIR) $(DEP_DIR)
	@echo "Compiling $<"
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(@:$(OBJ_DIR)/%.o=$(DEP_DIR)/%.d))
	@$(CC) $(CFLAGS) -c $< -o $@

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

# Flash target (requires st-flash or openocd)
flash: $(HEX_FILE)
	@echo "Flashing $(HEX_FILE) to target..."
	@echo "Note: Configure flash command for your programmer (st-flash, openocd, etc.)"
	@echo "Example: st-flash write $(HEX_FILE) 0x08000000"

# Include dependency files
-include $(DEP_FILES)

.PHONY: all clean size flash
