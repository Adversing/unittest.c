#include "unittest.h"

#define INITIAL_RESULT_CAPACITY 8

static const char* get_status_color(test_status_t status) {
    switch (status) {
        case STATUS_SUCCESS:
            return ANSI_GREEN;
        case STATUS_UNEXPECTED_OUTPUT:
            return ANSI_YELLOW;
        case STATUS_EXPECTED_BUILD_ERROR:
        case STATUS_EXPECTED_RUNTIME_ERROR:
            return ANSI_GRAY;
        case STATUS_BUILD_ERROR:
        case STATUS_RUNTIME_ERROR:
            return ANSI_RED;
        default:
            return ANSI_RESET;
    }
}

static char get_status_char(test_status_t status) {
    switch (status) {
        case STATUS_SUCCESS:
        case STATUS_UNEXPECTED_OUTPUT:
            return 'K';
        case STATUS_EXPECTED_BUILD_ERROR:
        case STATUS_BUILD_ERROR:
            return 'B';
        case STATUS_EXPECTED_RUNTIME_ERROR:
        case STATUS_RUNTIME_ERROR:
            return 'R';
        default:
            return '?'; // this should be handled better smhw
    }
}

test_runner_t* test_runner_create(void) {
    test_runner_t* runner = malloc(sizeof(test_runner_t));
    if (!runner) return NULL;
    
    runner->root_suite = NULL;
    memset(&runner->global_stats, 0, sizeof(test_stats_t));
    return runner;
}

void test_runner_destroy(test_runner_t* runner) {
    if (!runner) return;
    
    if (runner->root_suite) {
        test_suite_destroy_siblings(runner->root_suite);
    }
    free(runner);
}

test_suite_t* test_suite_create(const char* name) {
    test_suite_t* suite = malloc(sizeof(test_suite_t));
    if (!suite) return NULL;
    
    suite->name = strdup(name);
    if (!suite->name) {
        free(suite);
        return NULL;
    }
    
    suite->test_cases = NULL;
    suite->child_suites = NULL;
    suite->next = NULL;
    memset(&suite->stats, 0, sizeof(test_stats_t));
    return suite;
}

void test_suite_destroy(test_suite_t* suite) {
    if (!suite) return;
    
    if (suite->test_cases) {
        test_case_destroy_siblings(suite->test_cases);
    }
    
    if (suite->child_suites) {
        test_suite_destroy_siblings(suite->child_suites);
    }
    
    free(suite->name);
    free(suite);
}

void test_suite_destroy_siblings(test_suite_t* suite) {
    while (suite) {
        test_suite_t* next = suite->next;
        suite->next = NULL;
        test_suite_destroy(suite);
        suite = next;
    }
}

void test_suite_add_child(test_suite_t* parent, test_suite_t* child) {
    if (!parent || !child) return;
    
    if (!parent->child_suites) {
        parent->child_suites = child;
    } else {
        test_suite_t* current = parent->child_suites;
        while (current->next) {
            current = current->next;
        }
        current->next = child;
    }
}

void test_suite_add_test_case(test_suite_t* suite, test_case_t* test_case) {
    if (!suite || !test_case) return;
    
    if (!suite->test_cases) {
        suite->test_cases = test_case;
    } else {
        test_case_t* current = suite->test_cases;
        while (current->next) {
            current = current->next;
        }
        current->next = test_case;
    }
}

test_case_t* test_case_create(const char* name, test_func_t test_func) {
    test_case_t* test_case = malloc(sizeof(test_case_t));
    if (!test_case) return NULL;
    
    test_case->name = strdup(name);
    if (!test_case->name) {
        free(test_case);
        return NULL;
    }
    
    test_case->test_func = test_func;
    test_case->results = NULL;
    test_case->result_count = 0;
    test_case->result_capacity = 0;
    test_case->next = NULL;
    return test_case;
}

void test_case_destroy(test_case_t* test_case) {
    if (!test_case) return;
        
    free(test_case->name);
    free(test_case->results);
    free(test_case);
}

void test_case_destroy_siblings(test_case_t* test_case) {
    while (test_case) {
        test_case_t* next = test_case->next;
        test_case->next = NULL; 
        test_case_destroy(test_case);
        test_case = next;
    }
}

int test_case_add_result(test_case_t* test_case, test_status_t status) {
    if (!test_case) return -1;
    
    // array resize
    if (test_case->result_count >= test_case->result_capacity) {
        int new_capacity = test_case->result_capacity == 0 ? 
                          INITIAL_RESULT_CAPACITY : 
                          test_case->result_capacity * 2;
        
        test_status_t* new_results = realloc(test_case->results, 
                                           new_capacity * sizeof(test_status_t));
        if (!new_results) {
            return -1;  // realloc error
        }
        
        test_case->results = new_results;
        test_case->result_capacity = new_capacity;
    }
    
    test_case->results[test_case->result_count] = status;
    test_case->result_count++;
    return 0;
}

int test_case_add_results_va(test_case_t* test_case, int count, ...) {
    if (!test_case || count <= 0) return -1;
    
    va_list args;
    va_start(args, count);
    
    for (int i = 0; i < count; i++) {
        test_status_t status = va_arg(args, test_status_t);
        if (test_case_add_result(test_case, status) != 0) {
            va_end(args);
            return -1;  // add result failed
        }
    }
    
    va_end(args);
    return 0;
}

void test_runner_add_suite(test_runner_t* runner, test_suite_t* suite) {
    if (!runner || !suite) return;
    
    if (!runner->root_suite) {
        runner->root_suite = suite;
    } else {
        test_suite_t* current = runner->root_suite;
        while (current->next) {
            current = current->next;
        }
        current->next = suite;
    }
}

static void calculate_suite_stats(test_suite_t* suite) {
    if (!suite) return;
    
    memset(&suite->stats, 0, sizeof(test_stats_t));
    
    // stats calc from test cases
    test_case_t* current_case = suite->test_cases;
    while (current_case) {
        for (int i = 0; i < current_case->result_count; i++) {
            switch (current_case->results[i]) {
                case STATUS_SUCCESS:
                    suite->stats.success_count++;
                    break;
                case STATUS_UNEXPECTED_OUTPUT:
                    suite->stats.unexpected_output_count++;
                    break;
                case STATUS_EXPECTED_BUILD_ERROR:
                    suite->stats.expected_build_error_count++;
                    break;
                case STATUS_BUILD_ERROR:
                    suite->stats.build_error_count++;
                    break;
                case STATUS_EXPECTED_RUNTIME_ERROR:
                    suite->stats.expected_runtime_error_count++;
                    break;
                case STATUS_RUNTIME_ERROR:
                    suite->stats.runtime_error_count++;
                    break;
            }
        }
        current_case = current_case->next;
    }
    
    test_suite_t* current_suite = suite->child_suites;
    while (current_suite) {
        calculate_suite_stats(current_suite);
        suite->stats.success_count += current_suite->stats.success_count;
        suite->stats.unexpected_output_count += current_suite->stats.unexpected_output_count;
        suite->stats.expected_build_error_count += current_suite->stats.expected_build_error_count;
        suite->stats.build_error_count += current_suite->stats.build_error_count;
        suite->stats.expected_runtime_error_count += current_suite->stats.expected_runtime_error_count;
        suite->stats.runtime_error_count += current_suite->stats.runtime_error_count;
        current_suite = current_suite->next;
    }
}

void print_legend(void) {
    printf("%sK%s - success                %sK%s - unexpected output\n",
           ANSI_GREEN, ANSI_RESET, ANSI_YELLOW, ANSI_RESET);
    printf("%sB%s - expected build error   %sB%s - build error\n",
           ANSI_GRAY, ANSI_RESET, ANSI_RED, ANSI_RESET);
    printf("%sR%s - expected runtime error %sR%s - runtime error\n",
           ANSI_GRAY, ANSI_RESET, ANSI_RED, ANSI_RESET);
    printf("\n");
}

static void print_stats(const test_stats_t* stats) {
    printf("K: %s%2d%s/%s%d%s  B: %s%2d%s/%s%d%s  R: %s%2d%s/%s%d%s",
           ANSI_GREEN, stats->success_count, ANSI_RESET,
           ANSI_YELLOW, stats->unexpected_output_count, ANSI_RESET,
           ANSI_GRAY, stats->expected_build_error_count, ANSI_RESET,
           ANSI_RED, stats->build_error_count, ANSI_RESET,
           ANSI_GRAY, stats->expected_runtime_error_count, ANSI_RESET,
           ANSI_RED, stats->runtime_error_count, ANSI_RESET);
}

static void print_tree_node(test_suite_t* suite, const char* prefix, bool is_last, int depth) {
    if (!suite) return;
    
    // current suite
    printf("%s%s─%s", prefix, is_last ? "└" : "├", suite->name);
    
    // padding and stats
    int name_len = strlen(suite->name);
    int padding = 50 - strlen(prefix) - name_len - 2; // 2 for tree chars
    if (padding < 1) padding = 1;
    
    for (int i = 0; i < padding; i++) {
        printf(" ");
    }
    
    print_stats(&suite->stats);
    printf("\n");
    
    // this is clunky, but it works
    char new_prefix[256];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s ", prefix, is_last ? " " : "│");
    
    // child suites
    test_suite_t* current_suite = suite->child_suites;
    while (current_suite) {
        bool is_last_suite = (current_suite->next == NULL);
        print_tree_node(current_suite, new_prefix, is_last_suite, depth + 1);
        current_suite = current_suite->next;
    }
    
    test_case_t* current_case = suite->test_cases;
    while (current_case) {
        bool is_last_case = (current_case->next == NULL && suite->child_suites == NULL);
        
        printf("%s%s─%s: ", new_prefix, is_last_case ? "└" : "├", current_case->name);
        
        // print results
        for (int i = 0; i < current_case->result_count; i++) {
            const char* color = get_status_color(current_case->results[i]);
            char status_char = get_status_char(current_case->results[i]);
            printf("%s%c%s ", color, status_char, ANSI_RESET);
        }
        printf("\n");
        
        current_case = current_case->next;
    }
}

void print_test_results(test_runner_t* runner) {
    if (!runner || !runner->root_suite) return;
    
    print_legend();
    
    // stats calc
    test_suite_t* current = runner->root_suite;
    while (current) {
        calculate_suite_stats(current);
        current = current->next;
    }
    
    current = runner->root_suite;
    while (current) {
        bool is_last = (current->next == NULL);
        print_tree_node(current, "", is_last, 0);
        current = current->next;
    }
}

void test_runner_run(test_runner_t* runner) {
    if (!runner) return;
    
    test_suite_t* current_suite = runner->root_suite;
    while (current_suite) {
        test_case_t* current_case = current_suite->test_cases;
        while (current_case) {
            if (current_case->test_func && current_case->result_count == 0) {
                
                // no manual results were added
                test_status_t result = current_case->test_func();
                if (test_case_add_result(current_case, result) != 0) {
                    fprintf(stderr, "Warning: Failed to add test result for %s\n", 
                           current_case->name);
                }
            }
            current_case = current_case->next;
        }
        current_suite = current_suite->next;
    }
    
    print_test_results(runner);
}