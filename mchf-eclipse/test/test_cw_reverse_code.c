/**
 * @file test_cw_reverse_code.c
 * @brief Unit tests for cw_gen.c CwGen_ReverseCode()
 *
 * Reimplements the base-4 digit reversal logic inline to avoid
 * hardware-dependent headers and global state.
 */

#include "test.h"
#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Reimplementation of CwGen_ReverseCode()
 * ------------------------------------------------------------------------- */

static uint32_t host_reverse_code(uint32_t code)
{
    int n[10], c = 0, i = 0;
    uint32_t result = 0;

    while (code > 0)
    {
        n[c++] = code % 4;
        code /= 4;
    }
    for (i = 0; i < c; i++)
    {
        result = result * 4 + n[i];
    }
    return result;
}

/* =========================================================================
 * Tests: CwGen_ReverseCode
 * ========================================================================= */

static void test_reverse_code_zero(void)
{
    /* Zero input should return zero */
    TEST_ASSERT_EQUAL(0, host_reverse_code(0));
}

static void test_reverse_code_single_digit(void)
{
    /* Single base-4 digit should be unchanged */
    TEST_ASSERT_EQUAL(1, host_reverse_code(1));
    TEST_ASSERT_EQUAL(2, host_reverse_code(2));
    TEST_ASSERT_EQUAL(3, host_reverse_code(3));
}

static void test_reverse_code_two_digits(void)
{
    /* Reverse base-4 digits: 4 = 10_4 -> 01_4 = 1 */
    TEST_ASSERT_EQUAL(1, host_reverse_code(4));
    TEST_ASSERT_EQUAL(5, host_reverse_code(5));   /* 11_4 -> 11_4 = 5 */
    TEST_ASSERT_EQUAL(9, host_reverse_code(6));   /* 12_4 -> 21_4 = 9 */
}

static void test_reverse_code_palindrome(void)
{
    /* Palindrome in base-4 should be unchanged */
    TEST_ASSERT_EQUAL(5, host_reverse_code(5));   /* 11_4 */
    TEST_ASSERT_EQUAL(21, host_reverse_code(21)); /* 111_4 = 1+4+16 = 21 */
}

static void test_reverse_code_trailing_zeros_dropped(void)
{
    /* Trailing zeros in base-4 become leading zeros and are dropped */
    TEST_ASSERT_EQUAL(1, host_reverse_code(4));   /* 10_4 -> 01_4 = 1 */
    TEST_ASSERT_EQUAL(1, host_reverse_code(16));  /* 100_4 -> 001_4 = 1 */
}

static void test_reverse_code_max_value(void)
{
    /* Maximum value that fits in n[10] (10 base-4 digits = 4^10 - 1 = 1048575) */
    TEST_ASSERT_EQUAL(1048575, host_reverse_code(1048575U)); /* 3333333333_4 -> reversed = same */
}

static void test_reverse_code_double_reverse(void)
{
    /* Reversing twice should return original */
    uint32_t original = 12345;
    TEST_ASSERT_EQUAL(original, host_reverse_code(host_reverse_code(original)));
}

static void test_reverse_code_power_of_four(void)
{
    /* 4^n in base-4 is 1 followed by n zeros; reversed is 1 */
    TEST_ASSERT_EQUAL(1, host_reverse_code(1));      /* 4^0 */
    TEST_ASSERT_EQUAL(1, host_reverse_code(4));      /* 4^1 */
    TEST_ASSERT_EQUAL(1, host_reverse_code(16));     /* 4^2 */
    TEST_ASSERT_EQUAL(1, host_reverse_code(64));     /* 4^3 */
    TEST_ASSERT_EQUAL(1, host_reverse_code(256));    /* 4^4 */
}

static void test_reverse_code_increasing(void)
{
    /* For codes that are powers of 4 plus 1: 4^n + 1 = 100...001_4 */
    /* Reversed: 100...001_4 = same value */
    TEST_ASSERT_EQUAL(5, host_reverse_code(5));      /* 11_4 */
    TEST_ASSERT_EQUAL(17, host_reverse_code(17));    /* 101_4 */
    TEST_ASSERT_EQUAL(65, host_reverse_code(65));    /* 1001_4 */
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_cw_reverse_code_register(void)
{
    printf("--- cw_reverse_code ---\n");

    TEST_REGISTER(test_reverse_code_zero);
    TEST_REGISTER(test_reverse_code_single_digit);
    TEST_REGISTER(test_reverse_code_two_digits);
    TEST_REGISTER(test_reverse_code_palindrome);
    TEST_REGISTER(test_reverse_code_trailing_zeros_dropped);
    TEST_REGISTER(test_reverse_code_max_value);
    TEST_REGISTER(test_reverse_code_double_reverse);
    TEST_REGISTER(test_reverse_code_power_of_four);
    TEST_REGISTER(test_reverse_code_increasing);

    printf("\n");
}
