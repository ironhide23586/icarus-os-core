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

# Source code to test (kernel only for now)
KERNEL_SRC = $(PROJECT_ROOT)/Core/Src/kernel/task.c
KERNEL_INC = $(PROJECT_ROOT)/Core/Inc/kernel

# Test sources
TEST_SOURCES = $(wildcard $(SRC_DIR)/test_*.c)
MOCK_SOURCES = $(wildcard $(MOCKS_DIR)/*.c)

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
KERNEL_OBJ = $(OBJ_DIR)/task.o
TEST_OBJS = $(TEST_SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
MOCK_OBJS = $(MOCK_SOURCES:$(MOCKS_DIR)/%.c=$(OBJ_DIR)/%.o)

# Test executable
TEST_EXE = $(BUILD_DIR)/test_runner

# Coverage directories
COVERAGE_DIR = $(BUILD_DIR)/coverage
COVERAGE_INFO = $(COVERAGE_DIR)/coverage.info
COVERAGE_HTML = $(COVERAGE_DIR)/html

.PHONY: all test clean coverage coverage-html coverage-summary

# Default target
all: test

# Build test executable
test: $(TEST_EXE)
	@echo "Running tests..."
	@$(TEST_EXE) || true

# Link test executable
$(TEST_EXE): $(UNITY_OBJ) $(KERNEL_OBJ) $(TEST_OBJS) $(MOCK_OBJS) | $(BUILD_DIR) $(OBJ_DIR)
	@echo "Linking test executable..."
	@$(CC) $(UNITY_OBJ) $(KERNEL_OBJ) $(TEST_OBJS) $(MOCK_OBJS) $(LDFLAGS) -o $(TEST_EXE)

# Compile Unity
$(UNITY_OBJ): $(UNITY_DIR)/unity.c | $(OBJ_DIR)
	@echo "Compiling Unity..."
	@mkdir -p $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile kernel source (with test defines)
$(KERNEL_OBJ): $(KERNEL_SRC) | $(OBJ_DIR)
	@echo "Compiling kernel source for testing..."
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
	@lcov --capture --directory $(OBJ_DIR) --output-file $(COVERAGE_INFO) --base-directory $(PROJECT_ROOT) --no-external --ignore-errors source || true
	@lcov --remove $(COVERAGE_INFO) '/usr/*' '/Applications/*' '/Library/*' --output-file $(COVERAGE_INFO) || true
	@lcov --remove $(COVERAGE_INFO) '*/mocks/*' '*/unity/*' --output-file $(COVERAGE_INFO) || true
	@# Fix source file paths: replace incorrect paths with correct test paths
	@if [ -f $(COVERAGE_INFO) ]; then \
		TEST_ABS_DIR=$$(cd $(TEST_DIR) && pwd); \
		perl -pi -e "s|$(PROJECT_ROOT)/tes?\.?/src/|$$TEST_ABS_DIR/src/|g" $(COVERAGE_INFO) 2>/dev/null || \
		sed -i.bak "s|$(PROJECT_ROOT)/tes\./src/|$$TEST_ABS_DIR/src/|g" $(COVERAGE_INFO) 2>/dev/null || \
		sed -i '' "s|$(PROJECT_ROOT)/tes\./src/|$$TEST_ABS_DIR/src/|g" $(COVERAGE_INFO) 2>/dev/null || true; \
		rm -f $(COVERAGE_INFO).bak 2>/dev/null || true; \
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
