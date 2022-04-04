/*
 * Acutest -- Another C/C++ Unit Test facility
 * <http://github.com/mity/acutest>
 *
 * Copyright (c) 2013-2019 Martin Mitas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef ACUTEST_H__
#define ACUTEST_H__


/************************
 *** Public interface ***
 ************************/

/* By default, "acutest.h" provides the main program entry point (function
 * main()). However, if the test suite is composed of multiple source files
 * which include "acutest.h", then this causes a problem of multiple main()
 * definitions. To avoid this problem, #define macro TEST_NO_MAIN in all
 * compilation units but one.
 */

/* Macro to specify list of unit tests in the suite.
 * The unit test implementation MUST provide list of unit tests it implements
 * with this macro:
 *
 *   TEST_LIST = {
 *       { "test1_name", test1_func_ptr },
 *       { "test2_name", test2_func_ptr },
 *       ...
 *       { 0 }
 *   };
 *
 * The list specifies names of each test (must be unique) and pointer to
 * a function implementing it. The function does not take any arguments
 * and has no return values, i.e. every test function has to be compatible
 * with this prototype:
 *
 *   void test_func(void);
 */
#define TEST_LIST               const struct test__ test_list__[]


/* Macros for testing whether an unit test succeeds or fails. These macros
 * can be used arbitrarily in functions implementing the unit tests.
 *
 * If any condition fails throughout execution of a test, the test fails.
 *
 * TEST_CHECK takes only one argument (the condition), TEST_CHECK_ allows
 * also to specify an error message to print out if the condition fails.
 * (It expects printf-like format string and its parameters). The macros
 * return non-zero (condition passes) or 0 (condition fails).
 *
 * That can be useful when more conditions should be checked only if some
 * preceding condition passes, as illustrated in this code snippet:
 *
 *   SomeStruct* ptr = allocate_some_struct();
 *   if(TEST_CHECK(ptr != NULL)) {
 *       TEST_CHECK(ptr->member1 < 100);
 *       TEST_CHECK(ptr->member2 > 200);
 *   }
 */
#define TEST_CHECK_(cond,...)   test_check__((cond), __FILE__, __LINE__, __VA_ARGS__)
#define TEST_CHECK(cond)        test_check__((cond), __FILE__, __LINE__, "%s", #cond)

#ifdef __cplusplus
/* Macros to verify that the code (the 1st argument) throws exception of given
 * type (the 2nd argument). (Note these macros are only available in C++.)
 *
 * TEST_EXCEPTION_ is like TEST_EXCEPTION but accepts custom printf-like
 * message.
 *
 * For example:
 *
 *   TEST_EXCEPTION(function_that_throw(), ExpectedExceptionType);
 *
 * If the function_that_throw() throws ExpectedExceptionType, the check passes.
 * If the function throws anything incompatible with ExpectedExceptionType
 * (or if it does not thrown an exception at all), the check fails.
 */
#define TEST_EXCEPTION(code, exctype)                                          \
    do {                                                                       \
        bool exc_ok__ = false;                                                 \
        const char *msg__ = NULL;                                              \
        try {                                                                  \
            code;                                                              \
            msg__ = "No exception thrown.";                                    \
        } catch(exctype const&) {                                              \
            exc_ok__= true;                                                    \
        } catch(...) {                                                         \
            msg__ = "Unexpected exception thrown.";                            \
        }                                                                      \
        test_check__(exc_ok__, __FILE__, __LINE__, #code " throws " #exctype); \
        if(msg__ != NULL)                                                      \
            test_message__("%s", msg__);                                       \
    } while(0)
#define TEST_EXCEPTION_(code, exctype, ...)                                    \
    do {                                                                       \
        bool exc_ok__ = false;                                                 \
        const char *msg__ = NULL;                                              \
        try {                                                                  \
            code;                                                              \
            msg__ = "No exception thrown.";                                    \
        } catch(exctype const&) {                                              \
            exc_ok__= true;                                                    \
        } catch(...) {                                                         \
            msg__ = "Unexpected exception thrown.";                            \
        }                                                                      \
        test_check__(exc_ok__, __FILE__, __LINE__, __VA_ARGS__);               \
        if(msg__ != NULL)                                                      \
            test_message__("%s", msg__);                                       \
    } while(0)
#endif  /* #ifdef __cplusplus */


/* Sometimes it is useful to split execution of more complex unit tests to some
 * smaller parts and associate those parts with some names.
 *
 * This is especially handy if the given unit test is implemented as a loop
 * over some vector of multiple testing inputs. Using these macros allow to use
 * sort of subtitle for each iteration of the loop (e.g. outputting the input
 * itself or a name associated to it), so that if any TEST_CHECK condition
 * fails in the loop, it can be easily seen which iteration triggers the
 * failure, without the need to manually output the iteration-specific data in
 * every single TEST_CHECK inside the loop body.
 *
 * TEST_CASE allows to specify only single string as the name of the case,
 * TEST_CASE_ provides all the power of printf-like string formatting.
 *
 * Note that the test cases cannot be nested. Starting a new test case ends
 * implicitly the previous one. To end the test case explicitly (e.g. to end
 * the last test case after exiting the loop), you may use TEST_CASE(NULL).
 */
#define TEST_CASE_(...)         test_case__(__VA_ARGS__)
#define TEST_CASE(name)         test_case__("%s", name);


/* printf-like macro for outputting an extra information about a failure.
 *
 * Intended use is to output some computed output versus the expected value,
 * e.g. like this:
 *
 *   if(!TEST_CHECK(produced == expected)) {
 *       TEST_MSG("Expected: %d", expected);
 *       TEST_MSG("Produced: %d", produced);
 *   }
 *
 * Note the message is only written down if the most recent use of any checking
 * macro (like e.g. TEST_CHECK or TEST_EXCEPTION) in the current test failed.
 * This means the above is equivalent to just this:
 *
 *   TEST_CHECK(produced == expected);
 *   TEST_MSG("Expected: %d", expected);
 *   TEST_MSG("Produced: %d", produced);
 *
 * The macro can deal with multi-line output fairly well. It also automatically
 * adds a final new-line if there is none present.
 */
#define TEST_MSG(...)           test_message__(__VA_ARGS__)


/* Maximal output per TEST_MSG call. Longer messages are cut.
 * You may define another limit prior including "acutest.h"
 */
#ifndef TEST_MSG_MAXSIZE
    #define TEST_MSG_MAXSIZE    1024
#endif


/* Macro for dumping a block of memory.
 *
 * Its inteded use is very similar to what TEST_MSG is for, but instead of
 * generating any printf-like message, this is for dumping raw block of a
 * memory in a hexadecimal form:
 *
 * TEST_CHECK(size_produced == size_expected && memcmp(addr_produced, addr_expected, size_produced) == 0);
 * TEST_DUMP("Expected:", addr_expected, size_expected);
 * TEST_DUMP("Produced:", addr_produced, size_produced);
 */
#define TEST_DUMP(title, addr, size)    test_dump__(title, addr, size)

/* Maximal output per TEST_DUMP call (in bytes to dump). Longer blocks are cut.
 * You may define another limit prior including "acutest.h"
 */
#ifndef TEST_DUMP_MAXSIZE
    #define TEST_DUMP_MAXSIZE   1024
#endif


/**********************
 *** Implementation ***
 **********************/

/* The unit test files should not rely on anything below. */

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__)
    #define ACUTEST_UNIX__      1
    #include <errno.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <signal.h>
    #include <time.h>

    #if defined CLOCK_PROCESS_CPUTIME_ID  &&  defined CLOCK_MONOTONIC
        #define ACUTEST_HAS_POSIX_TIMER__       1
    #endif
#endif

#if defined(__gnu_linux__)
    #define ACUTEST_LINUX__     1
    #include <fcntl.h>
    #include <sys/stat.h>
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
    #define ACUTEST_WIN__       1
    #include <windows.h>
    #include <io.h>
#endif

#ifdef __cplusplus
    #include <exception>
#endif


/* Note our global private identifiers end with '__' to mitigate risk of clash
 * with the unit tests implementation. */


#ifdef __cplusplus
    extern "C" {
#endif


struct test__ {
    const char* name;
    void (*func)(void);
};

extern const struct test__ test_list__[];

int test_check__(int cond, const char* file, int line, const char* fmt, ...);
void test_case__(const char* fmt, ...);
void test_message__(const char* fmt, ...);
void test_dump__(const char* title, const void* addr, size_t size);


#ifndef TEST_NO_MAIN

static char* test_argv0__ = NULL;
static size_t test_list_size__ = 0;
static const struct test__** tests__ = NULL;
static char* test_flags__ = NULL;
static size_t test_count__ = 0;
static int test_no_exec__ = -1;
static int test_no_summary__ = 0;
static int test_tap__ = 0;
static int test_skip_mode__ = 0;
static int test_worker__ = 0;
static int test_worker_index__ = 0;
static int test_cond_failed__ = 0;

static int test_stat_failed_units__ = 0;
static int test_stat_run_units__ = 0;

static const struct test__* test_current_unit__ = NULL;
static int test_current_index__ = 0;
static char test_case_name__[64] = "";
static int test_current_already_logged__ = 0;
static int test_case_current_already_logged__ = 0;
static int test_verbose_level__ = 2;
static int test_current_failures__ = 0;
static int test_colorize__ = 0;
static int test_timer__ = 0;

#if defined ACUTEST_WIN__
    static LARGE_INTEGER test_timer_freq__;
    static LARGE_INTEGER test_timer_start__;
    static LARGE_INTEGER test_timer_end__;

    static void
    test_timer_init__(void)
    {
        QueryPerformanceFrequency(&test_timer_freq__);
    }

    static void
    test_timer_get_time__(LARGE_INTEGER* ts)
    {
        QueryPerformanceCounter(ts);
    }

    static void
    test_timer_print_diff__(void)
    {
        double duration = test_timer_end__.QuadPart - test_timer_start__.QuadPart;
        duration /= test_timer_freq__.QuadPart;
        printf("%.6lf secs", duration);
    }
#elif defined ACUTEST_HAS_POSIX_TIMER__
    static clockid_t test_timer_id__;
    struct timespec test_timer_start__;
    struct timespec test_timer_end__;

    static void
    test_timer_init__(void)
    {
        if(test_timer__ == 1)
    #ifdef CLOCK_MONOTONIC_RAW
            /* linux specific; not subject of NTP adjustements or adjtime() */
            test_timer_id__ = CLOCK_MONOTONIC_RAW;
    #else
            test_timer_id__ = CLOCK_MONOTONIC;
    #endif
        else if(test_timer__ == 2)
            test_timer_id__ = CLOCK_PROCESS_CPUTIME_ID;
    }

    static void
    test_timer_get_time__(struct timespec* ts)
    {
        clock_gettime(test_timer_id__, ts);
    }

    static void
    test_timer_print_diff__(void)
    {
        double duration = ((double) test_timer_end__.tv_sec +
                           (double) test_timer_end__.tv_nsec * 10e-9)
                          -
                          ((double) test_timer_start__.tv_sec +
                           (double) test_timer_start__.tv_nsec * 10e-9);
        printf("%.6lf secs", duration);
    }
#else
    static int test_timer_start__;
    static int test_timer_end__;

    void
    test_timer_init__(void)
    {}

    static void
    test_timer_get_time__(int* ts)
    {
        (void) ts;
    }

    static void
    test_timer_print_diff__(void)
    {}
#endif

#define TEST_COLOR_DEFAULT__            0
#define TEST_COLOR_GREEN__              1
#define TEST_COLOR_RED__                2
#define TEST_COLOR_DEFAULT_INTENSIVE__  3
#define TEST_COLOR_GREEN_INTENSIVE__    4
#define TEST_COLOR_RED_INTENSIVE__      5

static int
test_print_in_color__(int color, const char* fmt, ...)
{
    va_list args;
    char buffer[256];
    int n;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    buffer[sizeof(buffer)-1] = '\0';

    if(!test_colorize__) {
        return printf("%s", buffer);
    }

#if defined ACUTEST_UNIX__
    {
        const char* col_str;
        switch(color) {
            case TEST_COLOR_GREEN__:             col_str = "\033[0;32m"; break;
            case TEST_COLOR_RED__:               col_str = "\033[0;31m"; break;
            case TEST_COLOR_GREEN_INTENSIVE__:   col_str = "\033[1;32m"; break;
            case TEST_COLOR_RED_INTENSIVE__:     col_str = "\033[1;31m"; break;
            case TEST_COLOR_DEFAULT_INTENSIVE__: col_str = "\033[1m"; break;
            default:                             col_str = "\033[0m"; break;
        }
        printf("%s", col_str);
        n = printf("%s", buffer);
        printf("\033[0m");
        return n;
    }
#elif defined ACUTEST_WIN__
    {
        HANDLE h;
        CONSOLE_SCREEN_BUFFER_INFO info;
        WORD attr;

        h = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(h, &info);

        switch(color) {
            case TEST_COLOR_GREEN__:             attr = FOREGROUND_GREEN; break;
            case TEST_COLOR_RED__:               attr = FOREGROUND_RED; break;
            case TEST_COLOR_GREEN_INTENSIVE__:   attr = FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
            case TEST_COLOR_RED_INTENSIVE__:     attr = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
            case TEST_COLOR_DEFAULT_INTENSIVE__: attr = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY; break;
            default:                             attr = 0; break;
        }
        if(attr != 0)
            SetConsoleTextAttribute(h, attr);
        n = printf("%s", buffer);
        SetConsoleTextAttribute(h, info.wAttributes);
        return n;
    }
#else
    n = printf("%s", buffer);
    return n;
#endif
}

static void
test_begin_test_line__(const struct test__* test)
{
    if(!test_tap__) {
        if(test_verbose_level__ >= 3) {
            test_print_in_color__(TEST_COLOR_DEFAULT_INTENSIVE__, "Test %s:\n", test->name);
            test_current_already_logged__++;
        } else if(test_verbose_level__ >= 1) {
            int n;
            char spaces[48];

            n = test_print_in_color__(TEST_COLOR_DEFAULT_INTENSIVE__, "Test %s... ", test->name);
            memset(spaces, ' ', sizeof(spaces));
            if(n < (int) sizeof(spaces))
                printf("%.*s", (int) sizeof(spaces) - n, spaces);
        } else {
            test_current_already_logged__ = 1;
        }
    }
}

static void
test_finish_test_line__(int result)
{
    if(test_tap__) {
        const char* str = (result == 0) ? "ok" : "not ok";

        printf("%s %u - %s\n", str, test_current_index__ + 1, test_current_unit__->name);

        if(result == 0  &&  test_timer__) {
            printf("# Duration: ");
            test_timer_print_diff__();
            printf("\n");
        }
    } else {
        int color = (result == 0) ? TEST_COLOR_GREEN_INTENSIVE__ : TEST_COLOR_RED_INTENSIVE__;
        const char* str = (result == 0) ? "OK" : "FAILED";
        printf("[ ");
        test_print_in_color__(color, str);
        printf(" ]");

        if(result == 0  &&  test_timer__) {
            printf("  ");
            test_timer_print_diff__();
        }

        printf("\n");
    }
}

static void
test_line_indent__(int level)
{
    static const char spaces[] = "                ";
    int n = level * 2;

    if(test_tap__  &&  n > 0) {
        n--;
        printf("#");
    }

    while(n > 16) {
        printf("%s", spaces);
        n -= 16;
    }
    printf("%.*s", n, spaces);
}

int
test_check__(int cond, const char* file, int line, const char* fmt, ...)
{
    const char *result_str;
    int result_color;
    int verbose_level;

    if(cond) {
        result_str = "ok";
        result_color = TEST_COLOR_GREEN__;
        verbose_level = 3;
    } else {
        if(!test_current_already_logged__  &&  test_current_unit__ != NULL)
            test_finish_test_line__(-1);

        result_str = "failed";
        result_color = TEST_COLOR_RED__;
        verbose_level = 2;
        test_current_failures__++;
        test_current_already_logged__++;
    }

    if(test_verbose_level__ >= verbose_level) {
        va_list args;

        if(!test_case_current_already_logged__  &&  test_case_name__[0]) {
            test_line_indent__(1);
            test_print_in_color__(TEST_COLOR_DEFAULT_INTENSIVE__, "Case %s:\n", test_case_name__);
            test_current_already_logged__++;
            test_case_current_already_logged__++;
        }

        test_line_indent__(test_case_name__[0] ? 2 : 1);
        if(file != NULL) {
            if(test_verbose_level__ < 3) {
#ifdef ACUTEST_WIN__
                const char* lastsep1 = strrchr(file, '\\');
                const char* lastsep2 = strrchr(file, '/');
                if(lastsep1 == NULL)
                    lastsep1 = file-1;
                if(lastsep2 == NULL)
                    lastsep2 = file-1;
                file = (lastsep1 > lastsep2 ? lastsep1 : lastsep2) + 1;
#else
                const char* lastsep = strrchr(file, '/');
                if(lastsep != NULL)
                    file = lastsep+1;
#endif
            }
            printf("%s:%d: Check ", file, line);
        }

        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);

        printf("... ");
        test_print_in_color__(result_color, result_str);
        printf("\n");
        test_current_already_logged__++;
    }

    test_cond_failed__ = (cond == 0);
    return !test_cond_failed__;
}

void
test_case__(const char* fmt, ...)
{
    va_list args;

    if(test_verbose_level__ < 2)
        return;

    if(test_case_name__[0]) {
        test_case_current_already_logged__ = 0;
        test_case_name__[0] = '\0';
    }

    if(fmt == NULL)
        return;

    va_start(args, fmt);
    vsnprintf(test_case_name__, sizeof(test_case_name__) - 1, fmt, args);
    va_end(args);
    test_case_name__[sizeof(test_case_name__) - 1] = '\0';

    if(test_verbose_level__ >= 3) {
        test_line_indent__(1);
        test_print_in_color__(TEST_COLOR_DEFAULT_INTENSIVE__, "Case %s:\n", test_case_name__);
        test_current_already_logged__++;
        test_case_current_already_logged__++;
    }
}

void
test_message__(const char* fmt, ...)
{
    char buffer[TEST_MSG_MAXSIZE];
    char* line_beg;
    char* line_end;
    va_list args;

    if(test_verbose_level__ < 2)
        return;

    /* We allow extra message only when something is already wrong in the
     * current test. */
    if(test_current_unit__ == NULL  ||  !test_cond_failed__)
        return;

    va_start(args, fmt);
    vsnprintf(buffer, TEST_MSG_MAXSIZE, fmt, args);
    va_end(args);
    buffer[TEST_MSG_MAXSIZE-1] = '\0';

    line_beg = buffer;
    while(1) {
        line_end = strchr(line_beg, '\n');
        if(line_end == NULL)
            break;
        test_line_indent__(test_case_name__[0] ? 3 : 2);
        printf("%.*s\n", (int)(line_end - line_beg), line_beg);
        line_beg = line_end + 1;
    }
    if(line_beg[0] != '\0') {
        test_line_indent__(test_case_name__[0] ? 3 : 2);
        printf("%s\n", line_beg);
    }
}

void
test_dump__(const char* title, const void* addr, size_t size)
{
    static const size_t BYTES_PER_LINE = 16;
    size_t line_beg;
    size_t truncate = 0;

    if(test_verbose_level__ < 2)
        return;

    /* We allow extra message only when something is already wrong in the
     * current test. */
    if(test_current_unit__ == NULL  ||  !test_cond_failed__)
        return;

    if(size > TEST_DUMP_MAXSIZE) {
        truncate = size - TEST_DUMP_MAXSIZE;
        size = TEST_DUMP_MAXSIZE;
    }

    test_line_indent__(test_case_name__[0] ? 3 : 2);
    printf((title[strlen(title)-1] == ':') ? "%s\n" : "%s:\n", title);

    for(line_beg = 0; line_beg < size; line_beg += BYTES_PER_LINE) {
        size_t line_end = line_beg + BYTES_PER_LINE;
        size_t off;

        test_line_indent__(test_case_name__[0] ? 4 : 3);
        printf("%08x: ", line_beg);
        for(off = line_beg; off < line_end; off++) {
            if(off < size)
                printf(" %02x", ((unsigned char*)addr)[off]);
            else
                printf("   ");
        }

        printf("  ");
        for(off = line_beg; off < line_end; off++) {
            unsigned char byte = ((unsigned char*)addr)[off];
            if(off < size)
                printf("%c", (iscntrl(byte) ? '.' : byte));
            else
                break;
        }

        printf("\n");
    }

    if(truncate > 0) {
        test_line_indent__(test_case_name__[0] ? 4 : 3);
        printf("           ... (and more %u bytes)\n", (unsigned) truncate);
    }
}

static void
test_list_names__(void)
{
    const struct test__* test;

    printf("Unit tests:\n");
    for(test = &test_list__[0]; test->func != NULL; test++)
        printf("  %s\n", test->name);
}

static void
test_remember__(int i)
{
    if(test_flags__[i])
        return;
    else
        test_flags__[i] = 1;

    tests__[test_count__] = &test_list__[i];
    test_count__++;
}

static int
test_name_contains_word__(const char* name, const char* pattern)
{
    static const char word_delim[] = " \t-_.";
    const char* substr;
    size_t pattern_len;
    int starts_on_word_boundary;
    int ends_on_word_boundary;

    pattern_len = strlen(pattern);

    substr = strstr(name, pattern);
    while(substr != NULL) {
        starts_on_word_boundary = (substr == name || strchr(word_delim, substr[-1]) != NULL);
        ends_on_word_boundary = (substr[pattern_len] == '\0' || strchr(word_delim, substr[pattern_len]) != NULL);

        if(starts_on_word_boundary && ends_on_word_boundary)
            return 1;

        substr = strstr(substr+1, pattern);
    }

    return 0;
}

static int
test_lookup__(const char* pattern)
{
    int i;
    int n = 0;

    /* Try exact match. */
    for(i = 0; i < (int) test_list_size__; i++) {
        if(strcmp(test_list__[i].name, pattern) == 0) {
            test_remember__(i);
            n++;
            break;
        }
    }
    if(n > 0)
        return n;

    /* Try word match. */
    for(i = 0; i < (int) test_list_size__; i++) {
        if(test_name_contains_word__(test_list__[i].name, pattern)) {
            test_remember__(i);
            n++;
        }
    }
    if(n > 0)
        return n;

    /* Try relaxed match. */
    for(i = 0; i < (int) test_list_size__; i++) {
        if(strstr(test_list__[i].name, pattern) != NULL) {
            test_remember__(i);
            n++;
        }
    }

    return n;
}


/* Called if anything goes bad in Acutest, or if the unit test ends in other
 * way then by normal returning from its function (e.g. exception or some
 * abnormal child process termination). */
static void
test_error__(const char* fmt, ...)
{
    va_list args;

    if(test_verbose_level__ == 0)
        return;

    if(test_verbose_level__ <= 2  &&  !test_current_already_logged__  &&  test_current_unit__ != NULL) {
        if(test_tap__) {
            test_finish_test_line__(-1);
        } else {
            printf("[ ");
            test_print_in_color__(TEST_COLOR_RED_INTENSIVE__, "FAILED");
            printf(" ]\n");
        }
    }

    if(test_verbose_level__ >= 2) {
        test_line_indent__(1);
        if(test_verbose_level__ >= 3)
            test_print_in_color__(TEST_COLOR_RED_INTENSIVE__, "ERROR: ");
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        printf("\n");
    }

    if(test_verbose_level__ >= 3) {
        printf("\n");
    }
}

/* Call directly the given test unit function. */
static int
test_do_run__(const struct test__* test, int index)
{
    test_current_unit__ = test;
    test_current_index__ = index;
    test_current_failures__ = 0;
    test_current_already_logged__ = 0;
    test_cond_failed__ = 0;

    test_timer_init__();

    test_begin_test_line__(test);

#ifdef __cplusplus
    try {
#endif

        /* This is good to do for case the test unit e.g. crashes. */
        fflush(stdout);
        fflush(stderr);

        test_timer_get_time__(&test_timer_start__);
        test->func();
        test_timer_get_time__(&test_timer_end__);

        if(test_verbose_level__ >= 3) {
            test_line_indent__(1);
            if(test_current_failures__ == 0) {
                test_print_in_color__(TEST_COLOR_GREEN_INTENSIVE__, "SUCCESS: ");
                printf("All conditions have passed.\n");

                if(test_timer__) {
                    test_line_indent__(1);
                    printf("Duration: ");
                    test_timer_print_diff__();
                    printf("\n");
                }
            } else {
                test_print_in_color__(TEST_COLOR_RED_INTENSIVE__, "FAILED: ");
                printf("%d condition%s %s failed.\n",
                        test_current_failures__,
                        (test_current_failures__ == 1) ? "" : "s",
                        (test_current_failures__ == 1) ? "has" : "have");
            }
            printf("\n");
        } else if(test_verbose_level__ >= 1 && test_current_failures__ == 0) {
            test_finish_test_line__(0);
        }

        test_case__(NULL);
        test_current_unit__ = NULL;
        return (test_current_failures__ == 0) ? 0 : -1;

#ifdef __cplusplus
    } catch(std::exception& e) {
        const char* what = e.what();
        if(what != NULL)
            test_error__("Threw std::exception: %s", what);
        else
            test_error__("Threw std::exception");
        return -1;
    } catch(...) {
        test_error__("Threw an exception");
        return -1;
    }
#endif
}

/* Trigger the unit test. If possible (and not suppressed) it starts a child
 * process who calls test_do_run__(), otherwise it calls test_do_run__()
 * directly. */
static void
test_run__(const struct test__* test, int index)
{
    int failed = 1;

    test_current_unit__ = test;
    test_current_already_logged__ = 0;

    if(!test_no_exec__) {

#if defined(ACUTEST_UNIX__)

        pid_t pid;
        int exit_code;

        /* Make sure the child starts with empty I/O buffers. */
        fflush(stdout);
        fflush(stderr);

        pid = fork();
        if(pid == (pid_t)-1) {
            test_error__("Cannot fork. %s [%d]", strerror(errno), errno);
            failed = 1;
        } else if(pid == 0) {
            /* Child: Do the test. */
            failed = (test_do_run__(test, index) != 0);
            exit(failed ? 1 : 0);
        } else {
            /* Parent: Wait until child terminates and analyze its exit code. */
            waitpid(pid, &exit_code, 0);
            if(WIFEXITED(exit_code)) {
                switch(WEXITSTATUS(exit_code)) {
                    case 0:   failed = 0; break;   /* test has passed. */
                    case 1:   /* noop */ break;    /* "normal" failure. */
                    default:  test_error__("Unexpected exit code [%d]", WEXITSTATUS(exit_code));
                }
            } else if(WIFSIGNALED(exit_code)) {
                char tmp[32];
                const char* signame;
                switch(WTERMSIG(exit_code)) {
                    case SIGINT:  signame = "SIGINT"; break;
                    case SIGHUP:  signame = "SIGHUP"; break;
                    case SIGQUIT: signame = "SIGQUIT"; break;
                    case SIGABRT: signame = "SIGABRT"; break;
                    case SIGKILL: signame = "SIGKILL"; break;
                    case SIGSEGV: signame = "SIGSEGV"; break;
                    case SIGILL:  signame = "SIGILL"; break;
                    case SIGTERM: signame = "SIGTERM"; break;
                    default:      sprintf(tmp, "signal %d", WTERMSIG(exit_code)); signame = tmp; break;
                }
                test_error__("Test interrupted by %s", signame);
            } else {
                test_error__("Test ended in an unexpected way [%d]", exit_code);
            }
        }

#elif defined(ACUTEST_WIN__)

        char buffer[512] = {0};
        STARTUPINFOA startupInfo;
        PROCESS_INFORMATION processInfo;
        DWORD exitCode;

        /* Windows has no fork(). So we propagate all info into the child
         * through a command line arguments. */
        _snprintf(buffer, sizeof(buffer)-1,
                 "%s --worker=%d %s --no-exec --no-summary %s --verbose=%d --color=%s -- \"%s\"",
                 test_argv0__, index, test_timer__ ? "--timer" : "",
                 test_tap__ ? "--tap" : "", test_verbose_level__,
                 test_colorize__ ? "always" : "never",
                 test->name);
        memset(&startupInfo, 0, sizeof(startupInfo));
        startupInfo.cb = sizeof(STARTUPINFO);
        if(CreateProcessA(NULL, buffer, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo)) {
            WaitForSingleObject(processInfo.hProcess, INFINITE);
            GetExitCodeProcess(processInfo.hProcess, &exitCode);
            CloseHandle(processInfo.hThread);
            CloseHandle(processInfo.hProcess);
            failed = (exitCode != 0);
        } else {
            test_error__("Cannot create unit test subprocess [%ld].", GetLastError());
            failed = 1;
        }

#else

        /* A platform where we don't know how to run child process. */
        failed = (test_do_run__(test, index) != 0);

#endif

    } else {
        /* Child processes suppressed through --no-exec. */
        failed = (test_do_run__(test, index) != 0);
    }

    test_current_unit__ = NULL;

    test_stat_run_units__++;
    if(failed)
        test_stat_failed_units__++;
}

#if defined(ACUTEST_WIN__)
/* Callback for SEH events. */
static LONG CALLBACK
test_exception_filter__(EXCEPTION_POINTERS *ptrs)
{
    test_error__("Unhandled SEH exception %08lx at %p.",
                 ptrs->ExceptionRecord->ExceptionCode,
                 ptrs->ExceptionRecord->ExceptionAddress);
    fflush(stdout);
    fflush(stderr);
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif


#define TEST_CMDLINE_OPTFLAG_OPTIONALARG__      0x0001
#define TEST_CMDLINE_OPTFLAG_REQUIREDARG__      0x0002

#define TEST_CMDLINE_OPTID_NONE__               0
#define TEST_CMDLINE_OPTID_UNKNOWN__            (-0x7fffffff + 0)
#define TEST_CMDLINE_OPTID_MISSINGARG__         (-0x7fffffff + 1)
#define TEST_CMDLINE_OPTID_BOGUSARG__           (-0x7fffffff + 2)

typedef struct TEST_CMDLINE_OPTION__ {
    char shortname;
    const char* longname;
    int id;
    unsigned flags;
} TEST_CMDLINE_OPTION__;

static int
test_cmdline_handle_short_opt_group__(const TEST_CMDLINE_OPTION__* options,
                    const char* arggroup,
                    int (*callback)(int /*optval*/, const char* /*arg*/))
{
    const TEST_CMDLINE_OPTION__* opt;
    int i;
    int ret = 0;

    for(i = 0; arggroup[i] != '\0'; i++) {
        for(opt = options; opt->id != 0; opt++) {
            if(arggroup[i] == opt->shortname)
                break;
        }

        if(opt->id != 0  &&  !(opt->flags & TEST_CMDLINE_OPTFLAG_REQUIREDARG__)) {
            ret = callback(opt->id, NULL);
        } else {
            /* Unknown option. */
            char badoptname[3];
            badoptname[0] = '-';
            badoptname[1] = arggroup[i];
            badoptname[2] = '\0';
            ret = callback((opt->id != 0 ? TEST_CMDLINE_OPTID_MISSINGARG__ : TEST_CMDLINE_OPTID_UNKNOWN__),
                            badoptname);
        }

        if(ret != 0)
            break;
    }

    return ret;
}

#define TEST_CMDLINE_AUXBUF_SIZE__  32

static int
test_cmdline_read__(const TEST_CMDLINE_OPTION__* options, int argc, char** argv,
                    int (*callback)(int /*optval*/, const char* /*arg*/))
{

    const TEST_CMDLINE_OPTION__* opt;
    char auxbuf[TEST_CMDLINE_AUXBUF_SIZE__+1];
    int after_doubledash = 0;
    int i = 1;
    int ret = 0;

    auxbuf[TEST_CMDLINE_AUXBUF_SIZE__] = '\0';

    while(i < argc) {
        if(after_doubledash  ||  strcmp(argv[i], "-") == 0) {
            /* Non-option argument. */
            ret = callback(TEST_CMDLINE_OPTID_NONE__, argv[i]);
        } else if(strcmp(argv[i], "--") == 0) {
            /* End of options. All the remaining members are non-option arguments. */
            after_doubledash = 1;
        } else if(argv[i][0] != '-') {
            /* Non-option argument. */
            ret = callback(TEST_CMDLINE_OPTID_NONE__, argv[i]);
        } else {
            for(opt = options; opt->id != 0; opt++) {
                if(opt->longname != NULL  &&  strncmp(argv[i], "--", 2) == 0) {
                    size_t len = strlen(opt->longname);
                    if(strncmp(argv[i]+2, opt->longname, len) == 0) {
                        /* Regular long option. */
                        if(argv[i][2+len] == '\0') {
                            /* with no argument provided. */
                            if(!(opt->flags & TEST_CMDLINE_OPTFLAG_REQUIREDARG__))
                                ret = callback(opt->id, NULL);
                            else
                                ret = callback(TEST_CMDLINE_OPTID_MISSINGARG__, argv[i]);
                            break;
                        } else if(argv[i][2+len] == '=') {
                            /* with an argument provided. */
                            if(opt->flags & (TEST_CMDLINE_OPTFLAG_OPTIONALARG__ | TEST_CMDLINE_OPTFLAG_REQUIREDARG__)) {
                                ret = callback(opt->id, argv[i]+2+len+1);
                            } else {
                                sprintf(auxbuf, "--%s", opt->longname);
                                ret = callback(TEST_CMDLINE_OPTID_BOGUSARG__, auxbuf);
                            }
                            break;
                        } else {
                            continue;
                        }
                    }
                } else if(opt->shortname != '\0'  &&  argv[i][0] == '-') {
                    if(argv[i][1] == opt->shortname) {
                        /* Regular short option. */
                        if(opt->flags & TEST_CMDLINE_OPTFLAG_REQUIREDARG__) {
                            if(argv[i][2] != '\0')
                                ret = callback(opt->id, argv[i]+2);
                            else if(i+1 < argc)
                                ret = callback(opt->id, argv[++i]);
                            else
                                ret = callback(TEST_CMDLINE_OPTID_MISSINGARG__, argv[i]);
                            break;
                        } else {
                            ret = callback(opt->id, NULL);

                            /* There might be more (argument-less) short options
                             * grouped together. */
                            if(ret == 0  &&  argv[i][2] != '\0')
                                ret = test_cmdline_handle_short_opt_group__(options, argv[i]+2, callback);
                            break;
                        }
                    }
                }
            }

            if(opt->id == 0) {  /* still not handled? */
                if(argv[i][0] != '-') {
                    /* Non-option argument. */
                    ret = callback(TEST_CMDLINE_OPTID_NONE__, argv[i]);
                } else {
                    /* Unknown option. */
                    char* badoptname = argv[i];

                    if(strncmp(badoptname, "--", 2) == 0) {
                        /* Strip any argument from the long option. */
                        char* assignement = strchr(badoptname, '=');
                        if(assignement != NULL) {
                            size_t len = assignement - badoptname;
                            if(len > TEST_CMDLINE_AUXBUF_SIZE__)
                                len = TEST_CMDLINE_AUXBUF_SIZE__;
                            strncpy(auxbuf, badoptname, len);
                            auxbuf[len] = '\0';
                            badoptname = auxbuf;
                        }
                    }

                    ret = callback(TEST_CMDLINE_OPTID_UNKNOWN__, badoptname);
                }
            }
        }

        if(ret != 0)
            return ret;
        i++;
    }

    return ret;
}

static void
test_help__(void)
{
    printf("Usage: %s [options] [test...]\n", test_argv0__);
    printf("\n");
    printf("Run the specified unit tests; or if the option '--skip' is used, run all\n");
    printf("tests in the suite but those listed.  By default, if no tests are specified\n");
    printf("on the command line, all unit tests in the suite are run.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -s, --skip            Execute all unit tests but the listed ones\n");
    printf("      --exec[=WHEN]     If supported, execute unit tests as child processes\n");
    printf("                          (WHEN is one of 'auto', 'always', 'never')\n");
#if defined ACUTEST_WIN__
    printf("  -t, --timer           Measure test duration\n");
#elif defined ACUTEST_HAS_POSIX_TIMER__
    printf("  -t, --timer           Measure test duration (real time)\n");
    printf("      --timer=TIMER     Measure test duration, using given timer\n");
    printf("                          (TIMER is one of 'real', 'cpu')\n");
#endif
    printf("  -E, --no-exec         Same as --exec=never\n");
    printf("      --no-summary      Suppress printing of test results summary\n");
    printf("      --tap             Produce TAP-compliant output\n");
    printf("                          (See https://testanything.org/)\n");
    printf("  -l, --list            List unit tests in the suite and exit\n");
    printf("  -v, --verbose         Make output more verbose\n");
    printf("      --verbose=LEVEL   Set verbose level to LEVEL:\n");
    printf("                          0 ... Be silent\n");
    printf("                          1 ... Output one line per test (and summary)\n");
    printf("                          2 ... As 1 and failed conditions (this is default)\n");
    printf("                          3 ... As 1 and all conditions (and extended summary)\n");
    printf("      --color[=WHEN]    Enable colorized output\n");
    printf("                          (WHEN is one of 'auto', 'always', 'never')\n");
    printf("      --no-color        Same as --color=never\n");
    printf("  -h, --help            Display this help and exit\n");

    if(test_list_size__ < 16) {
        printf("\n");
        test_list_names__();
    }
}

static const TEST_CMDLINE_OPTION__ test_cmdline_options__[] = {
    { 's',  "skip",         's', 0 },
    {  0,   "exec",         'e', TEST_CMDLINE_OPTFLAG_OPTIONALARG__ },
    { 'E',  "no-exec",      'E', 0 },
#if defined ACUTEST_WIN__
    { 't',  "timer",        't', 0 },
#elif defined ACUTEST_HAS_POSIX_TIMER__
    { 't',  "timer",        't', TEST_CMDLINE_OPTFLAG_OPTIONALARG__ },
#endif
    {  0,   "no-summary",   'S', 0 },
    {  0,   "tap",          'T', 0 },
    { 'l',  "list",         'l', 0 },
    { 'v',  "verbose",      'v', TEST_CMDLINE_OPTFLAG_OPTIONALARG__ },
    {  0,   "color",        'c', TEST_CMDLINE_OPTFLAG_OPTIONALARG__ },
    {  0,   "no-color",     'C', 0 },
    { 'h',  "help",         'h', 0 },
    {  0,   "worker",       'w', TEST_CMDLINE_OPTFLAG_REQUIREDARG__ },  /* internal */
    {  0,   NULL,            0,  0 }
};

static int
test_cmdline_callback__(int id, const char* arg)
{
    switch(id) {
        case 's':
            test_skip_mode__ = 1;
            break;

        case 'e':
            if(arg == NULL || strcmp(arg, "always") == 0) {
                test_no_exec__ = 0;
            } else if(strcmp(arg, "never") == 0) {
                test_no_exec__ = 1;
            } else if(strcmp(arg, "auto") == 0) {
                /*noop*/
            } else {
                fprintf(stderr, "%s: Unrecognized argument '%s' for option --exec.\n", test_argv0__, arg);
                fprintf(stderr, "Try '%s --help' for more information.\n", test_argv0__);
                exit(2);
            }
            break;

        case 'E':
            test_no_exec__ = 1;
            break;

        case 't':
#if defined ACUTEST_WIN__  ||  defined ACUTEST_HAS_POSIX_TIMER__
            if(arg == NULL || strcmp(arg, "real") == 0) {
                test_timer__ = 1;
    #ifndef ACUTEST_WIN__
            } else if(strcmp(arg, "cpu") == 0) {
                test_timer__ = 2;
    #endif
            } else {
                fprintf(stderr, "%s: Unrecognized argument '%s' for option --timer.\n", test_argv0__, arg);
                fprintf(stderr, "Try '%s --help' for more information.\n", test_argv0__);
                exit(2);
            }
#endif
            break;

        case 'S':
            test_no_summary__ = 1;
            break;

        case 'T':
            test_tap__ = 1;
            break;

        case 'l':
            test_list_names__();
            exit(0);

        case 'v':
            test_verbose_level__ = (arg != NULL ? atoi(arg) : test_verbose_level__+1);
            break;

        case 'c':
            if(arg == NULL || strcmp(arg, "always") == 0) {
                test_colorize__ = 1;
            } else if(strcmp(arg, "never") == 0) {
                test_colorize__ = 0;
            } else if(strcmp(arg, "auto") == 0) {
                /*noop*/
            } else {
                fprintf(stderr, "%s: Unrecognized argument '%s' for option --color.\n", test_argv0__, arg);
                fprintf(stderr, "Try '%s --help' for more information.\n", test_argv0__);
                exit(2);
            }
            break;

        case 'C':
            test_colorize__ = 0;
            break;

        case 'h':
            test_help__();
            exit(0);

        case 'w':
            test_worker__ = 1;
            test_worker_index__ = atoi(arg);
            break;

        case 0:
            if(test_lookup__(arg) == 0) {
                fprintf(stderr, "%s: Unrecognized unit test '%s'\n", test_argv0__, arg);
                fprintf(stderr, "Try '%s --list' for list of unit tests.\n", test_argv0__);
                exit(2);
            }
            break;

        case TEST_CMDLINE_OPTID_UNKNOWN__:
            fprintf(stderr, "Unrecognized command line option '%s'.\n", arg);
            fprintf(stderr, "Try '%s --help' for more information.\n", test_argv0__);
            exit(2);

        case TEST_CMDLINE_OPTID_MISSINGARG__:
            fprintf(stderr, "The command line option '%s' requires an argument.\n", arg);
            fprintf(stderr, "Try '%s --help' for more information.\n", test_argv0__);
            exit(2);

        case TEST_CMDLINE_OPTID_BOGUSARG__:
            fprintf(stderr, "The command line option '%s' does not expect an argument.\n", arg);
            fprintf(stderr, "Try '%s --help' for more information.\n", test_argv0__);
            exit(2);
    }

    return 0;
}


#ifdef ACUTEST_LINUX__
static int
test_is_tracer_present__(void)
{
    char buf[256+32+1];
    int tracer_present = 0;
    int fd;
    ssize_t n_read;

    fd = open("/proc/self/status", O_RDONLY);
    if(fd == -1)
        return 0;

    n_read = read(fd, buf, sizeof(buf)-1);
    while(n_read > 0) {
        static const char pattern[] = "TracerPid:";
        const char* field;

        buf[n_read] = '\0';
        field = strstr(buf, pattern);
        if(field != NULL  &&  field < buf + sizeof(buf) - 32) {
            pid_t tracer_pid = (pid_t) atoi(field + sizeof(pattern) - 1);
            tracer_present = (tracer_pid != 0);
            break;
        }

        if(n_read == sizeof(buf)-1) {
            memmove(buf, buf + sizeof(buf)-1 - 32, 32);
            n_read = read(fd, buf+32, sizeof(buf)-1-32);
            if(n_read > 0)
                n_read += 32;
        }
    }

    close(fd);
    return tracer_present;
}
#endif

int
main(int argc, char** argv)
{
    int i;
    test_argv0__ = argv[0];

#if defined ACUTEST_UNIX__
    test_colorize__ = isatty(STDOUT_FILENO);
#elif defined ACUTEST_WIN__
 #if defined __BORLANDC__
    test_colorize__ = isatty(_fileno(stdout));
 #else
    test_colorize__ = _isatty(_fileno(stdout));
 #endif
#else
    test_colorize__ = 0;
#endif

    /* Count all test units */
    test_list_size__ = 0;
    for(i = 0; test_list__[i].func != NULL; i++)
        test_list_size__++;

    tests__ = (const struct test__**) malloc(sizeof(const struct test__*) * test_list_size__);
    test_flags__ = (char*) malloc(sizeof(char) * test_list_size__);
    if(tests__ == NULL || test_flags__ == NULL) {
        fprintf(stderr, "Out of memory.\n");
        exit(2);
    }
    memset((void*) test_flags__, 0, sizeof(char) * test_list_size__);

    /* Parse options */
    test_cmdline_read__(test_cmdline_options__, argc, argv, test_cmdline_callback__);

#if defined(ACUTEST_WIN__)
    SetUnhandledExceptionFilter(test_exception_filter__);
#endif

    /* By default, we want to run all tests. */
    if(test_count__ == 0) {
        for(i = 0; test_list__[i].func != NULL; i++)
            tests__[i] = &test_list__[i];
        test_count__ = test_list_size__;
    }

    /* Guess whether we want to run unit tests as child processes. */
    if(test_no_exec__ < 0) {
        test_no_exec__ = 0;

        if(test_count__ <= 1) {
            test_no_exec__ = 1;
        } else {
#ifdef ACUTEST_WIN__
            if(IsDebuggerPresent())
                test_no_exec__ = 1;
#endif
#ifdef ACUTEST_LINUX__
            if(test_is_tracer_present__())
                test_no_exec__ = 1;
#endif
        }
    }

    if(test_tap__) {
        /* TAP requires we know test result ("ok", "not ok") before we output
         * anything about the test, and this gets problematic for larger verbose
         * levels. */
        if(test_verbose_level__ > 2)
            test_verbose_level__ = 2;

        /* TAP harness should provide some summary. */
        test_no_summary__ = 1;

        if(!test_worker__)
            printf("1..%d\n", (int) test_count__);
    }

    /* Run the tests */
    if(!test_skip_mode__) {
        /* Run the listed tests. */
        for(i = 0; i < (int) test_count__; i++)
            test_run__(tests__[i], test_worker_index__ + i);
    } else {
        /* Run all tests except those listed. */
        int index = test_worker_index__;
        for(i = 0; test_list__[i].func != NULL; i++) {
            if(!test_flags__[i])
                test_run__(&test_list__[i], index++);
        }
    }

    /* Write a summary */
    if(!test_no_summary__ && test_verbose_level__ >= 1) {
        if(test_verbose_level__ >= 3) {
            test_print_in_color__(TEST_COLOR_DEFAULT_INTENSIVE__, "Summary:\n");

            printf("  Count of all unit tests:     %4d\n", (int) test_list_size__);
            printf("  Count of run unit tests:     %4d\n", test_stat_run_units__);
            printf("  Count of failed unit tests:  %4d\n", test_stat_failed_units__);
            printf("  Count of skipped unit tests: %4d\n", (int) test_list_size__ - test_stat_run_units__);
        }

        if(test_stat_failed_units__ == 0) {
            test_print_in_color__(TEST_COLOR_GREEN_INTENSIVE__, "SUCCESS:");
            printf(" All unit tests have passed.\n");
        } else {
            test_print_in_color__(TEST_COLOR_RED_INTENSIVE__, "FAILED:");
            printf(" %d of %d unit tests %s failed.\n",
                    test_stat_failed_units__, test_stat_run_units__,
                    (test_stat_failed_units__ == 1) ? "has" : "have");
        }

        if(test_verbose_level__ >= 3)
            printf("\n");
    }

    free((void*) tests__);
    free((void*) test_flags__);

    return (test_stat_failed_units__ == 0) ? 0 : 1;
}


#endif  /* #ifndef TEST_NO_MAIN */

#ifdef __cplusplus
    }  /* extern "C" */
#endif


#endif  /* #ifndef ACUTEST_H__ */
