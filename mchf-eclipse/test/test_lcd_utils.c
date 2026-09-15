/**
 * @file test_lcd_utils.c
 * @brief Unit tests for LCD string utility functions
 *
 * Tests UiLcdHy28_StringGetLine() by reimplementing the logic inline
 * to avoid hardware-dependent headers.
 */

#include "test.h"
#include <string.h>

/* -------------------------------------------------------------------------
 * Reimplementation of UiLcdHy28_StringGetLine for host testing
 * ------------------------------------------------------------------------- */
static const char* host_UiLcdHy28_StringGetLine(const char* str)
{
    const char* retval;
    for (retval = str; *retval != '\0' && *retval != '\n'; retval++);
    return retval;
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */
static void test_string_getline_empty_string(void)
{
    const char str[] = "";
    const char* result = host_UiLcdHy28_StringGetLine(str);
    /* Empty string: retval stays at str (points to '\0') */
    TEST_ASSERT(result == str);
    TEST_ASSERT_EQUAL('\0', *result);
}

static void test_string_getline_no_newline(void)
{
    const char str[] = "hello world";
    const char* result = host_UiLcdHy28_StringGetLine(str);
    /* No newline: retval points to null terminator at end */
    TEST_ASSERT(result == str + strlen(str));
    TEST_ASSERT_EQUAL('\0', *result);
}

static void test_string_getline_single_newline(void)
{
    const char str[] = "hello\nworld";
    const char* result = host_UiLcdHy28_StringGetLine(str);
    /* Should point to '\n' at position 5 */
    TEST_ASSERT(result == str + 5);
    TEST_ASSERT_EQUAL('\n', *result);
}

static void test_string_getline_multiple_newlines(void)
{
    const char str[] = "line1\nline2\nline3";
    const char* result = host_UiLcdHy28_StringGetLine(str);
    /* Should point to first '\n' at position 5 */
    TEST_ASSERT(result == str + 5);
    TEST_ASSERT_EQUAL('\n', *result);
}

static void test_string_getline_newline_at_start(void)
{
    const char str[] = "\nrest";
    const char* result = host_UiLcdHy28_StringGetLine(str);
    /* Should point to '\n' at position 0 */
    TEST_ASSERT(result == str);
    TEST_ASSERT_EQUAL('\n', *result);
}

static void test_string_getline_newline_at_end(void)
{
    const char str[] = "text\n";
    const char* result = host_UiLcdHy28_StringGetLine(str);
    /* Should point to '\n' at position 4 */
    TEST_ASSERT(result == str + 4);
    TEST_ASSERT_EQUAL('\n', *result);
}

static void test_string_getline_null_terminated_only(void)
{
    const char str[] = "abc\0def";
    const char* result = host_UiLcdHy28_StringGetLine(str);
    /* Should stop at first '\0' at position 3 */
    TEST_ASSERT(result == str + 3);
    TEST_ASSERT_EQUAL('\0', *result);
}

static void test_string_getline_single_char(void)
{
    const char str[] = "a";
    const char* result = host_UiLcdHy28_StringGetLine(str);
    /* Single char + null: retval points to null terminator at position 1 */
    TEST_ASSERT(result == str + 1);
    TEST_ASSERT_EQUAL('\0', *result);
}

static void test_string_getline_single_newline_char(void)
{
    const char str[] = "\n";
    const char* result = host_UiLcdHy28_StringGetLine(str);
    /* Just newline: retval points to '\n' at position 0 */
    TEST_ASSERT(result == str);
    TEST_ASSERT_EQUAL('\n', *result);
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */
void test_lcd_utils_register(void)
{
    printf("--- lcd_utils ---\n");
    TEST_REGISTER(test_string_getline_empty_string);
    TEST_REGISTER(test_string_getline_no_newline);
    TEST_REGISTER(test_string_getline_single_newline);
    TEST_REGISTER(test_string_getline_multiple_newlines);
    TEST_REGISTER(test_string_getline_newline_at_start);
    TEST_REGISTER(test_string_getline_newline_at_end);
    TEST_REGISTER(test_string_getline_null_terminated_only);
    TEST_REGISTER(test_string_getline_single_char);
    TEST_REGISTER(test_string_getline_single_newline_char);
    printf("\n");
}
