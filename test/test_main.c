#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "cmocka.h"

#ifdef _WIN32
/* Compatibility with the Windows standard C library. */
#define vsnprintf _vsnprintf
#endif /* _WIN32 */

#if defined(scanf) || defined(printf) || defined(fprintf) || defined(__isoc99_scanf)
#error "Not handled yet: redefinition of stdio functions"
#endif

#define array_length(x) (sizeof(x) / sizeof((x)[0]))

#define free_args(args)                            \
    for (int i = 0; i < array_length(args); ++i) { \
        free(args[i]);                             \
    }

int __real_main(int, char **);
int __real_printf(const char *format, ...);

int __wrap_printf(const char *format, ...) CMOCKA_PRINTF_ATTRIBUTE(1, 2);
int __wrap_fprintf(FILE *file, const char *format, ...) CMOCKA_PRINTF_ATTRIBUTE(2, 3);
int __wrap_scanf(const char *format, ...);
int __wrap___isoc99_scanf(const char *restrict format, ...);

#define TEST_BUFFER_SIZE 256
static char temporary_buffer[TEST_BUFFER_SIZE];
static char test_buffer_stdout[TEST_BUFFER_SIZE];
static char test_buffer_stderr[TEST_BUFFER_SIZE];

static FILE *test_stdin_file;

/**
 * Set bytes of stdout and stderr buffers to 0
 */
static void free_buffers() {
    bzero(test_buffer_stdout, TEST_BUFFER_SIZE);
    bzero(test_buffer_stderr, TEST_BUFFER_SIZE);
}

/**
 * Remove spaces in string
 * @param[in,out] s
 */
static void remove_spaces(char *s) {
    const char *d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while ((*s++ = *d++));
}

/**
 * A mock fprintf function that checks the value of strings printed to the
 * standard error stream or output stream.
 */
int __wrap_fprintf(FILE *file, const char *format, ...) {
    int return_value;
    va_list args;
    assert_true(file == stdout || file == stderr);
    va_start(args, format);
    return_value = vsnprintf(temporary_buffer, sizeof(temporary_buffer),
                             format, args);

    remove_spaces(temporary_buffer);

    if (file == stdout) {
        strcpy(test_buffer_stdout + strlen(test_buffer_stdout), temporary_buffer);
    } else {
        strcpy(test_buffer_stderr + strlen(test_buffer_stderr), temporary_buffer);
    }

    va_end(args);
    return return_value;
}

/**
 * A mock printf function that checks the value of strings printed to the
 * standard output stream.
 */
int __wrap_printf(const char *format, ...) {
    int return_value;
    va_list args;
    va_start(args, format);
    return_value = vsnprintf(temporary_buffer, sizeof(temporary_buffer),
                             format, args);
    remove_spaces(temporary_buffer);
    strcpy(test_buffer_stdout + strlen(test_buffer_stdout), temporary_buffer);
    va_end(args);
    return return_value;
}

/**
 *  A mock scanf function that redirects input from file
 */
int __wrap_scanf(const char *format, ...) {
    int return_value;
    va_list args;
    va_start(args, format);

    return_value = vfscanf(test_stdin_file, format, args);
    va_end(args);
    return return_value;
}

/**
 *  A mock __isoc99_scanf function that only call __wrap_scanf
 *  passing all its parameters
 */
int __wrap___isoc99_scanf(const char *restrict format, ...) {
    void *args = __builtin_apply_args();
    void *ret = __builtin_apply((void (*)()) __wrap_scanf, args, 1000);
    __builtin_return(ret);
}

static void test_example_main_no_args(void **state) {
    free_buffers();
    char *args[] = {
            strdup("test"),
    };
    char *main_args[] = {
            args[0],
    };
    assert_int_equal(__real_main(array_length(main_args), main_args), -1);
    free_args(args);
}

static void test_example_main_many_args(void **state) {
    free_buffers();
    char *args[] = {
            strdup("test"),
            strdup("--from=3"),
            strdup("--to=3"),
            strdup("something bad"),
    };
    char *main_args[] = {
            args[0],
            args[1],
            args[2],
            args[3],
    };
    assert_int_equal(__real_main(array_length(main_args), main_args), -2);
    free_args(args);
}

static void test_main_1(void **state) {
    free_buffers();
    test_stdin_file = NULL;
    if ((test_stdin_file = fopen("data/test1.txt", "r")) == NULL) {
        __real_printf("Cannot open file.\n");
        exit(1);
    }
    char *args[] = {
            strdup("test"),
            strdup("--from=3"),
    };
    char *main_args[] = {
            args[0],
            args[1],
    };
    assert_int_equal(__real_main(array_length(main_args), main_args), 3);
    assert_string_equal(test_buffer_stdout, "21");
    free_args(args);
    if (test_stdin_file != NULL) fclose(test_stdin_file);
}

static void test_main_2(void **state) {
    free_buffers();
    test_stdin_file = NULL;
    if ((test_stdin_file = fopen("data/test2.txt", "r")) == NULL) {
        __real_printf("Cannot open file.\n");
        exit(1);
    }
    char *args[] = {
            strdup("test"),
            strdup("--to=9"),
            strdup("--from=3"),
    };
    char *main_args[] = {
            args[0],
            args[1],
            args[2],
    };
    assert_int_equal(__real_main(array_length(main_args), main_args), 3);
    assert_string_equal(test_buffer_stdout, "1");
    assert_string_equal(test_buffer_stderr, "10");
    free_args(args);
    if (test_stdin_file != NULL) fclose(test_stdin_file);
}

int __wrap_main() {
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_example_main_no_args),
            cmocka_unit_test(test_example_main_many_args),
            cmocka_unit_test(test_main_1),
            cmocka_unit_test(test_main_2),
    };

    if (test_stdin_file != NULL) fclose(test_stdin_file);
    return cmocka_run_group_tests(tests, NULL, NULL);
}

