# ICARUS OS Unit Tests

Unit testing framework using Unity for DO-178C compliance and code coverage.

## Overview

This test suite provides host-based unit tests for the ICARUS OS kernel. Tests run on your development machine (not the embedded target) and can generate code coverage reports.

## Directory Structure

```
tests/
├── unity/              # Unity testing framework
├── cmock/              # CMock (for future use)
├── src/                # Test source files (test_*.c)
├── mocks/              # Mock implementations for hardware dependencies
├── runners/            # Test runners (auto-generated)
├── build/              # Build output and coverage reports
└── Makefile            # Test build system
```

## Prerequisites

**Required:**
- GCC compiler (for host machine)
- Make

**Optional (for coverage):**
- lcov: `brew install lcov` (macOS) or `apt-get install lcov` (Ubuntu)

## Quick Start

```bash
cd tests

# Run tests
make test

# Run tests with coverage
make COVERAGE=yes test

# Generate coverage report
make coverage

# Generate HTML coverage report
make coverage-html

# View coverage summary
make coverage-summary

# Open HTML report in browser (macOS)
open build/coverage/html/index.html
```

**Coverage Report Location:**
- HTML report: `tests/build/coverage/html/index.html`
- Coverage info: `tests/build/coverage/coverage.info`

**Note:** The HTML report is generated in `tests/build/coverage/html/index.html` (relative to the `tests/` directory).

## Writing Tests

### Test File Structure

Create test files in `tests/src/` with naming convention `test_*.c`:

```c
#include "unity.h"
#include "icarus/task.h"

void setUp(void) {
    // Initialize test fixtures
}

void tearDown(void) {
    // Cleanup after test
}

void test_function_name(void) {
    // Test implementation
    TEST_ASSERT_EQUAL(expected, actual);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_function_name);
    return UNITY_END();
}
```

### Unity Assertions

Common Unity assertions:
- `TEST_ASSERT_EQUAL(expected, actual)` - Compare integers
- `TEST_ASSERT_TRUE(condition)` - Assert true
- `TEST_ASSERT_FALSE(condition)` - Assert false
- `TEST_ASSERT_NULL(ptr)` - Assert NULL pointer
- `TEST_ASSERT_NOT_NULL(ptr)` - Assert non-NULL pointer
- `TEST_ASSERT_GREATER_THAN(threshold, value)` - Value > threshold
- `TEST_ASSERT_LESS_THAN(threshold, value)` - Value < threshold

See Unity documentation for full list.

## Mocks

Hardware dependencies are mocked in `tests/mocks/`:

- `mock_display.c/h` - Display system mocks
- `mock_hal.c/h` - HAL (Hardware Abstraction Layer) mocks
- `mock_usb.c/h` - USB CDC mocks

Add more mocks as needed for other dependencies.

## Coverage

### Generating Coverage Reports

```bash
# Build and run tests with coverage
make COVERAGE=yes test

# Generate coverage report
make coverage-html

# Open in browser
open build/coverage/html/index.html
```

### Coverage Metrics

Coverage reports show:
- **Statement Coverage**: Which lines were executed
- **Branch Coverage**: Which branches (if/else) were taken
- **Function Coverage**: Which functions were called

### DO-178C Coverage Requirements

- **Level C**: 100% statement coverage
- **Level B**: 100% statement + decision coverage
- **Level A**: 100% statement + MC/DC coverage

## Test Organization

### Current Tests

- `test_task_basic.c` - Basic task management tests
  - Print buffer operations
  - Critical sections
  - Tick counting
  - Busy wait

### Adding New Tests

1. Create `tests/src/test_<feature>.c`
2. Include Unity and kernel headers
3. Implement `setUp()` and `tearDown()` if needed
4. Write test functions with `test_` prefix
5. Add to `main()` with `RUN_TEST()`
6. Run: `make test`

## Integration with Main Build

The test system is separate from the embedded target build:

- **Target build**: `cd build && make` (ARM toolchain, for STM32)
- **Test build**: `cd tests && make` (Host GCC, for unit tests)

Coverage from tests can be integrated into DO-178C verification evidence.

## Troubleshooting

### "gcc: command not found"
- Install GCC: `xcode-select --install` (macOS) or `apt-get install gcc` (Ubuntu)

### "lcov: command not found"
- Install lcov: `brew install lcov` (macOS) or `apt-get install lcov` (Ubuntu)
- Coverage reports still work without lcov, but HTML reports won't generate

### Tests fail to compile
- Check that mocks exist for all hardware dependencies
- Verify include paths in Makefile
- Ensure kernel source compiles with `-DHOST_TEST` define

### No coverage data
- Ensure `COVERAGE=yes` when building: `make COVERAGE=yes test`
- Check that `.gcda` files are generated in `build/obj/`
- Run tests before generating coverage: `make test` then `make coverage`

## Next Steps

1. Add more test cases for kernel functions
2. Test task creation, scheduling, and state management
3. Add integration tests for task interactions
4. Achieve 100% statement coverage for kernel code
5. Add MC/DC coverage analysis for Level A compliance
