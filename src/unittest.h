#ifndef UNITTEST_H
#define UNITTEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#define ANSI_RESET "\033[0m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_RED "\033[31m"
#define ANSI_GRAY "\033[90m"

typedef enum {
    STATUS_SUCCESS,                     // K (green)
    STATUS_UNEXPECTED_OUTPUT,           // K (yellow) 
    STATUS_EXPECTED_BUILD_ERROR,        // B (gray)
    STATUS_BUILD_ERROR,                 // B (red)
    STATUS_EXPECTED_RUNTIME_ERROR,      // R (gray)
    STATUS_RUNTIME_ERROR                // R (red)
} test_status_t;

typedef struct {
    int success_count;
    int unexpected_output_count;
    int expected_build_error_count;
    int build_error_count;
    int expected_runtime_error_count;
    int runtime_error_count;
} test_stats_t;

typedef struct test_case test_case_t;
typedef struct test_suite test_suite_t;
typedef struct test_runner test_runner_t;

typedef test_status_t (*test_func_t)(void);

struct test_case {
    char* name;
    test_func_t test_func;
    test_status_t* results;
    int result_count;
    int result_capacity;
    test_case_t* next;
};

struct test_suite {
    char* name;
    test_case_t* test_cases;
    test_suite_t* child_suites;
    test_suite_t* next;
    test_stats_t stats;
};

struct test_runner {
    test_suite_t* root_suite;
    test_stats_t global_stats;
};

test_runner_t* test_runner_create(void);
void test_runner_destroy(test_runner_t* runner);

test_suite_t* test_suite_create(const char* name);
void test_suite_destroy(test_suite_t* suite);
void test_suite_destroy_siblings(test_suite_t* suite);
void test_suite_add_child(test_suite_t* parent, test_suite_t* child);
void test_suite_add_test_case(test_suite_t* suite, test_case_t* test_case);

test_case_t* test_case_create(const char* name, test_func_t test_func);
void test_case_destroy(test_case_t* test_case);
void test_case_destroy_siblings(test_case_t* test_case);
int test_case_add_result(test_case_t* test_case, test_status_t status);
int test_case_add_results_va(test_case_t* test_case, int count, ...);

void test_runner_add_suite(test_runner_t* runner, test_suite_t* suite);
void test_runner_run(test_runner_t* runner);

void print_legend(void);
void print_test_results(test_runner_t* runner);

#define UNITTEST_SUITE(name) test_suite_create(name)
#define UNITTEST_CASE(name, func) test_case_create(name, func)
#define UNITTEST_RUN(runner) test_runner_run(runner)

#define RESULTS(test_case, ...) test_case_add_results_va(test_case, \
    sizeof((test_status_t[]){__VA_ARGS__})/sizeof(test_status_t), __VA_ARGS__)

#define RESULTS_ARRAY(...) (test_status_t[]){__VA_ARGS__}, \
    sizeof((test_status_t[]){__VA_ARGS__})/sizeof(test_status_t)

#endif // UNITTEST_H