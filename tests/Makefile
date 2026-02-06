################################################################################
# Test Makefile for ICARUS OS Core
# Host-based unit tests using Unity framework
# For DO-178C code coverage and verification
################################################################################

# Project root (relative to tests/)
PROJECT_ROOT = ..

# Test directories
TEST_DIR = .
UNITY_DIR = $(TEST_DIR)/unity
CMOCK_DIR = $(TEST_DIR)/cmock
SRC_DIR = $(TEST_DIR)/src
MOCKS_DIR = $(TEST_DIR)/mocks
RUNNERS_DIR = $(TEST_DIR)/runners
BUILD_DIR = $(TEST_DIR)/build
OBJ_DIR = $(BUILD_DIR)/obj

# Source code to test (all application code that can be tested)
KERNEL_SRC = \
	$(PROJECT_ROOT)/Core/Src/icarus/kernel.c \
	$(PROJECT_ROOT)/Core/Src/icarus/scheduler.c \
	$(PROJECT_ROOT)/Core/Src/icarus/task.c \
	$(PROJECT_ROOT)/Core/Src/icarus/semaphore.c \
	$(PROJECT_ROOT)/Core/Src/icarus/pipe.c
BSP_SRC = \
	$(PROJECT_ROOT)/Core/Src/bsp/error.c \
	$(PROJECT_ROOT)/Core/Src/bsp/retarget_stdio.c \
	$(PROJECT_ROOT)/Core/Src/bsp/retarget_hal.c \
	$(PROJECT_ROOT)/Core/Src/bsp/stm32h7xx_it.c \
	$(PROJECT_ROOT)/Core/Src/bsp/display.c
# Note: gpio.c, tim.c, i2c.c, spi.c, rtc.c are STM32CubeMX-generated initialization code
# Note: context_switch.s is assembly and cannot be tested on host (requires ARM hardware)

# All application source files
APP_SRC = $(KERNEL_SRC) $(BSP_SRC)

KERNEL_INC = $(PROJECT_ROOT)/Core/Inc/icarus

# Test sources (all test_*.c files)
TEST_SOURCES = $(wildcard $(SRC_DIR)/test_*.c)
# Mock sources - exclude mocks for files we're testing (main, retarget_hal, display)
MOCK_SOURCES = $(filter-out $(MOCKS_DIR)/mock_main.c $(MOCKS_DIR)/mock_retarget_hal.c $(MOCKS_DIR)/mock_display.c, $(wildcard $(MOCKS_DIR)/*.c))

# Include directories
# MOCKS_DIR must come FIRST to override real headers with mocks
INCLUDES = \
	-I$(UNITY_DIR) \
	-I$(MOCKS_DIR) \
	-I$(KERNEL_INC) \
	-I$(PROJECT_ROOT)/Core/Inc \
	-I$(PROJECT_ROOT)/Core/Inc/bsp \
	-I$(SRC_DIR) \
	-I$(MOCKS_DIR) \
	-DHOST_TEST \
	-DTEST

# Compiler (host compiler, not ARM)
CC = gcc

# Coverage support
COVERAGE ?= yes

# Coverage flags
ifeq ($(COVERAGE),yes)
	COVERAGE_CFLAGS = -fprofile-arcs -ftest-coverage --coverage
	COVERAGE_LDFLAGS = --coverage
	COVERAGE_OPT = -O0
else
	COVERAGE_CFLAGS =
	COVERAGE_LDFLAGS =
	COVERAGE_OPT = -O2
endif

# Compiler flags for tests
CFLAGS = \
	$(COVERAGE_OPT) \
	-g \
	-Wall \
	-Wextra \
	-Wno-pedantic \
	-std=c11 \
	$(COVERAGE_CFLAGS) \
	$(INCLUDES) \
	-DTEST \
	-DHOST_TEST \
	-DSKIP_STATIC_ASSERTS

# Linker flags
LDFLAGS = \
	$(COVERAGE_LDFLAGS) \
	-lm

# Object files
UNITY_OBJ = $(OBJ_DIR)/unity.o
KERNEL_OBJS = $(KERNEL_SRC:$(PROJECT_ROOT)/Core/Src/icarus/%.c=$(OBJ_DIR)/%.o)
BSP_OBJS = $(BSP_SRC:$(PROJECT_ROOT)/Core/Src/bsp/%.c=$(OBJ_DIR)/bsp_%.o)
APP_OBJS = $(KERNEL_OBJS) $(BSP_OBJS)
TEST_OBJS = $(TEST_SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
MOCK_OBJS = $(MOCK_SOURCES:$(MOCKS_DIR)/%.c=$(OBJ_DIR)/%.o)

# Test executable
TEST_EXE = $(BUILD_DIR)/test_runner

# Coverage directories
# Allow override from parent Makefile to output to build/reports/coverage/
COVERAGE_OUTPUT_DIR ?= $(BUILD_DIR)/coverage/html
COVERAGE_DIR = $(BUILD_DIR)/coverage
COVERAGE_INFO = $(COVERAGE_DIR)/coverage.info
COVERAGE_HTML = $(COVERAGE_OUTPUT_DIR)

.PHONY: all test clean coverage coverage-html coverage-summary

# Default target
all: test

# Build test executable
test: $(TEST_EXE)
	@echo "Running tests..."
	@LLVM_PROFILE_FILE="$(OBJ_DIR)/test_runner.profraw" $(TEST_EXE) || true

# Link test executable
$(TEST_EXE): $(UNITY_OBJ) $(APP_OBJS) $(TEST_OBJS) $(MOCK_OBJS) | $(BUILD_DIR) $(OBJ_DIR)
	@echo "Linking test executable..."
	@$(CC) $(UNITY_OBJ) $(APP_OBJS) $(TEST_OBJS) $(MOCK_OBJS) $(LDFLAGS) -o $(TEST_EXE)

# Compile Unity
$(UNITY_OBJ): $(UNITY_DIR)/unity.c | $(OBJ_DIR)
	@echo "Compiling Unity..."
	@mkdir -p $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile kernel source files (with test defines)
$(OBJ_DIR)/%.o: $(PROJECT_ROOT)/Core/Src/icarus/%.c | $(OBJ_DIR)
	@echo "Compiling kernel source: $<"
	@mkdir -p $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile BSP source files
$(OBJ_DIR)/bsp_%.o: $(PROJECT_ROOT)/Core/Src/bsp/%.c | $(OBJ_DIR)
	@echo "Compiling BSP source: $<"
	@mkdir -p $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@


# Compile test sources
$(OBJ_DIR)/test_%.o: $(SRC_DIR)/test_%.c | $(OBJ_DIR)
	@echo "Compiling test: $<"
	@mkdir -p $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile mock sources (both .c files and mock_*.c files)
$(OBJ_DIR)/mock_%.o: $(MOCKS_DIR)/mock_%.c | $(OBJ_DIR)
	@echo "Compiling mock: $<"
	@mkdir -p $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

# Create directories
$(BUILD_DIR) $(OBJ_DIR):
	@mkdir -p $@

# Coverage targets
coverage: $(COVERAGE_INFO)
	@echo "Coverage report generated: $(COVERAGE_INFO)"
	@if command -v lcov >/dev/null 2>&1; then \
		lcov --summary $(COVERAGE_INFO) 2>/dev/null || echo "No coverage data"; \
	else \
		echo "Install lcov to view summary: brew install lcov (macOS)"; \
	fi

coverage-html: coverage
	@echo "Generating HTML coverage report..."
	@if ! command -v lcov >/dev/null 2>&1; then \
		echo "ERROR: lcov not found. Install: brew install lcov (macOS)"; \
		exit 1; \
	fi
	@mkdir -p $(COVERAGE_HTML)
	@# Use --synthesize-missing to generate HTML even if source paths are wrong
	@genhtml $(COVERAGE_INFO) --output-directory $(COVERAGE_HTML) --title "ICARUS OS Test Coverage" --ignore-errors source --synthesize-missing || true
	@if [ -f $(COVERAGE_HTML)/index.html ]; then \
		echo ""; \
		echo "✅ HTML coverage report generated: $(COVERAGE_HTML)/index.html"; \
		echo "Open in browser: open $(COVERAGE_HTML)/index.html"; \
	else \
		echo "WARNING: HTML report generation had errors. Check coverage.info file."; \
	fi

$(COVERAGE_INFO): test
	@echo "Collecting coverage data..."
	@if ! command -v lcov >/dev/null 2>&1; then \
		echo "ERROR: lcov not found. Install: brew install lcov (macOS)"; \
		exit 1; \
	fi
	@mkdir -p $(COVERAGE_DIR)
	@# Capture all coverage data - use test directory as base to capture relative paths correctly
	@lcov --capture --directory $(OBJ_DIR) --output-file $(COVERAGE_INFO) --base-directory $(TEST_DIR) --ignore-errors source || true
	@# Remove system paths
	@lcov --remove $(COVERAGE_INFO) '/usr/*' '/Applications/*' '/Library/*' --output-file $(COVERAGE_INFO) || true
	@# Remove test files, mocks, and unity framework (keep only application source)
	@lcov --remove $(COVERAGE_INFO) '*/tests/src/*' '*/mocks/*' '*/unity/*' --output-file $(COVERAGE_INFO) || true
	@# Fix paths: replace relative paths with absolute paths for all source files
	@if [ -f $(COVERAGE_INFO) ]; then \
		PROJECT_ABS=$$(cd $(PROJECT_ROOT) && pwd); \
		perl -pi -e "s|SF:\.\./Core/Src/|SF:$$PROJECT_ABS/Core/Src/|g" $(COVERAGE_INFO) 2>/dev/null || \
		sed -i.bak "s|SF:\.\./Core/Src/|SF:$$PROJECT_ABS/Core/Src/|g" $(COVERAGE_INFO) 2>/dev/null || \
		sed -i '' "s|SF:\.\./Core/Src/|SF:$$PROJECT_ABS/Core/Src/|g" $(COVERAGE_INFO) 2>/dev/null || true; \
		rm -f $(COVERAGE_INFO).bak 2>/dev/null || true; \
	fi
	@# Extract only application source files (Core/Src/icarus/*, Core/Src/bsp/*)
	@if [ -f $(COVERAGE_INFO) ]; then \
		lcov --extract $(COVERAGE_INFO) '*/Core/Src/icarus/*' '*/Core/Src/bsp/*' --output-file $(COVERAGE_INFO) || true; \
	fi

coverage-summary: coverage
	@echo "=== Test Coverage Summary ==="
	@lcov --summary $(COVERAGE_INFO) 2>/dev/null || echo "No coverage data available"

# Clean
clean:
	@echo "Cleaning test build..."
	@rm -rf $(BUILD_DIR)
	@find . -name "*.gcda" -delete 2>/dev/null || true
	@find . -name "*.gcno" -delete 2>/dev/null || true
	@find . -name "*.gcov" -delete 2>/dev/null || true
	@echo "Test build cleaned"
