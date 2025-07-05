# unittest.c

A simple yet powerful unit test library written in C.

![image](https://github.com/user-attachments/assets/1ae65c48-7ad5-4003-b1ad-df14bc1852a3)

## Features

- **Hierarchical Organization**: Organize tests in nested suites with unlimited depth
- **Colored Output**: ANSI color-coded status indicators
- **Tree Structure**: Unicode box-drawing characters for visual hierarchy
- **Multiple Status Types**: Success, errors, and expected failures
- **Statistics**: Automatic counting and display of test results
- **Memory Safe**: Robust memory management with error handling
- **Performance Optimized**: Efficient compilation and runtime performance
- **Modular Design**: Clean architecture following SOLID principles

## Status Types

| Symbol | Color | Meaning |
|--------|-------|---------|
| K | Green | Success |
| K | Yellow | Unexpected output |
| B | Gray | Expected build error |
| B | Red | Build error |
| R | Gray | Expected runtime error |
| R | Red | Runtime error |

## Quick Start

```c
#include "unittest.h"

// define a test function
test_status_t my_test(void) {
    // your test logic here
    return STATUS_SUCCESS;
}

int main(void) {
    // create test runner
    test_runner_t* runner = test_runner_create();
    if (!runner) {
        fprintf(stderr, "Failed to create test runner\n");
        return 1;
    }
    
    // create test suite
    test_suite_t* suite = test_suite_create("My Test Suite");
    if (!suite) {
        test_runner_destroy(runner);
        return 1;
    }
    
    // create and add test case
    test_case_t* test = test_case_create("my_test", my_test);
    if (test) {
        test_suite_add_test_case(suite, test);
    }
    
    // add suite to runner and execute
    test_runner_add_suite(runner, suite);
    test_runner_run(runner);
    
    // cleanup
    test_runner_destroy(runner);
    return 0;
}
```

## Building

```bash
make all
make run
```

To build as a library:

```bash
make libunittest.a
sudo make install
```

## API Reference

### Core Functions

#### Test Runner
- `test_runner_t* test_runner_create(void)` - Create a new test runner
- `void test_runner_destroy(test_runner_t* runner)` - Clean up test runner and all associated data
- `void test_runner_add_suite(test_runner_t* runner, test_suite_t* suite)` - Add suite to runner
- `void test_runner_run(test_runner_t* runner)` - Execute all tests and display results

#### Test Suite
- `test_suite_t* test_suite_create(const char* name)` - Create a new test suite
- `void test_suite_destroy(test_suite_t* suite)` - Clean up single test suite (children only)
- `void test_suite_destroy_siblings(test_suite_t* suite)` - Clean up entire sibling chain
- `void test_suite_add_child(test_suite_t* parent, test_suite_t* child)` - Add child suite
- `void test_suite_add_test_case(test_suite_t* suite, test_case_t* test_case)` - Add test case

#### Test Case
- `test_case_t* test_case_create(const char* name, test_func_t test_func)` - Create test case
- `void test_case_destroy(test_case_t* test_case)` - Clean up single test case
- `void test_case_destroy_siblings(test_case_t* test_case)` - Clean up entire sibling chain
- `int test_case_add_result(test_case_t* test_case, test_status_t status)` - Add single result (returns `0` on success)
- `int test_case_add_results_va(test_case_t* test_case, int count, ...)` - Add multiple results using variadic arguments

### Utility Macros

- `UNITTEST_SUITE(name)` - Quick suite creation
- `UNITTEST_CASE(name, func)` - Quick test case creation
- `UNITTEST_RUN(runner)` - Quick test execution
- `RESULTS(test_case, ...)` - Efficient variadic results addition
- `RESULTS_ARRAY(...)` - Legacy array-based results (for backwards compatibility)

## Advanced Usage

### Error Handling

The library provides comprehensive error handling with return codes:

```c
test_case_t* test = test_case_create("my_test", my_test_func);
if (!test) {
    fprintf(stderr, "Failed to create test case\n");
    return 1;
}

if (RESULTS(test, STATUS_SUCCESS, STATUS_BUILD_ERROR) != 0) {
    fprintf(stderr, "Failed to add test results\n");
    // handle error appropriately
}
```

### Nested Test Suites

Create complex hierarchical structures:

```c
test_suite_t* parent = test_suite_create("Parent Suite");
test_suite_t* child1 = test_suite_create("Child Suite 1");
test_suite_t* child2 = test_suite_create("Child Suite 2");
test_suite_t* grandchild = test_suite_create("Grandchild Suite");

test_suite_add_child(parent, child1);
test_suite_add_child(parent, child2);
test_suite_add_child(child1, grandchild);
```

### Multiple Test Results (Variadic API)

The `RESULTS` macro uses variadic functions for better performance:

```c
test_case_t* test = test_case_create("multi_result_test", NULL);
if (test) {
    RESULTS(test, 
           STATUS_SUCCESS, 
           STATUS_SUCCESS, 
           STATUS_EXPECTED_BUILD_ERROR,
           STATUS_BUILD_ERROR,
           STATUS_SUCCESS);
}
```

### Custom Test Functions with Error Checking

```c
test_status_t complex_test(void) {
    // setup
    void* resource = allocate_resource();
    if (!resource) {
        return STATUS_RUNTIME_ERROR;
    }
    
    // test logic
    int result = perform_operation(resource);
    
    // cleanup
    free_resource(resource);
    
    // assertion
    return (result == EXPECTED_VALUE) ? STATUS_SUCCESS : STATUS_RUNTIME_ERROR;
}
```

### Memory-Safe Operations

All memory allocations are checked and handled gracefully:

```c
// the library handles allocation failures gracefully
test_runner_t* runner = test_runner_create();
if (!runner) {
    // handle allocation failure
    return 1;
}

// all destroy functions handle NULL pointers safely
test_runner_destroy(runner);  // safe even if runner is NULL
```

### Memory Management Best Practices

```c
// good: check allocation success
test_runner_t* runner = test_runner_create();
if (!runner) {
    return 1;  // handle failure
}

// good: single cleanup call handles everything
test_runner_destroy(runner);

// The library automatically handles:
// - Sibling chains (next pointers)
// - Child hierarchies
// - Dynamic result arrays
// - String allocations
```

## Platform Support

- **Linux**: Fully tested with GCC
- **macOS**: Compatible (requires ANSI terminal support)
- **Windows**: Compatible with ANSI-enabled terminals or WSL

## Error Codes

Functions return the following error codes:

- `0`: Success
- `-1`: Memory allocation failure
- `NULL`: Object creation failure

## Thread Safety

The library is **NOT thread-safe**. If you need to run tests concurrently, create separate test runners for each thread. 
