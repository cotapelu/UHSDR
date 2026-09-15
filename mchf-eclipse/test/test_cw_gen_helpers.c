#include <stdint.h>
#include <stdbool.h>
#include "test.h"

/* -------------------------------------------------------------------------
 * Replicated helpers from cw_gen.c for host-based pure-logic testing
 * ------------------------------------------------------------------------- */

/**
 * Convert lowercase ASCII to uppercase by clearing bit 5 (subtracting 32).
 * Non-lowercase characters are returned unchanged.
 */
static uint8_t CwGen_TranslateToUperCase(uint8_t c)
{
    return (c >= 'a' && c <= 'z') ? (c - 32) : c;
}

/**
 * Replicated paddle logic: returns true if DIT is logically requested.
 * Paddle reversal swaps DIT and DAH line readings.
 */
static bool CwGen_DitRequested(bool paddle_reverse, bool dit_line, bool dah_line)
{
    if (paddle_reverse)
    {
        return dah_line;
    }
    else
    {
        return dit_line;
    }
}

/**
 * Replicated paddle logic: returns true if DAH is logically requested.
 */
static bool CwGen_DahRequested(bool paddle_reverse, bool dit_line, bool dah_line)
{
    if (!paddle_reverse)
    {
        return dah_line;
    }
    else
    {
        return dit_line;
    }
}

/* -------------------------------------------------------------------------
 * Tests: CwGen_TranslateToUperCase
 * ------------------------------------------------------------------------- */
static void test_uppercase_lowercase_a(void)
{
    TEST_ASSERT_EQUAL('A', CwGen_TranslateToUperCase('a'));
}

static void test_uppercase_lowercase_z(void)
{
    TEST_ASSERT_EQUAL('Z', CwGen_TranslateToUperCase('z'));
}

static void test_uppercase_already_upper(void)
{
    TEST_ASSERT_EQUAL('A', CwGen_TranslateToUperCase('A'));
    TEST_ASSERT_EQUAL('Z', CwGen_TranslateToUperCase('Z'));
}

static void test_uppercase_non_alpha_passthrough(void)
{
    TEST_ASSERT_EQUAL('0', CwGen_TranslateToUperCase('0'));
    TEST_ASSERT_EQUAL(' ', CwGen_TranslateToUperCase(' '));
    TEST_ASSERT_EQUAL('!', CwGen_TranslateToUperCase('!'));
}

static void test_uppercase_boundary_values(void)
{
    TEST_ASSERT_EQUAL('A', CwGen_TranslateToUperCase('a'));
    TEST_ASSERT_EQUAL('Z', CwGen_TranslateToUperCase('z'));
    TEST_ASSERT_EQUAL('@', CwGen_TranslateToUperCase('@'));
    TEST_ASSERT_EQUAL('[', CwGen_TranslateToUperCase('['));
}

/* -------------------------------------------------------------------------
 * Tests: CwGen_DitRequested / CwGen_DahRequested
 * ------------------------------------------------------------------------- */
static void test_dah_normal_paddle(void)
{
    /* Normal: DAH requested when dah_line is pressed */
    TEST_ASSERT(CwGen_DahRequested(false, false, true));
    TEST_ASSERT(!CwGen_DahRequested(false, true, false));
}

static void test_dit_normal_paddle(void)
{
    /* Normal: DIT requested when dit_line is pressed */
    TEST_ASSERT(CwGen_DitRequested(false, true, false));
    TEST_ASSERT(!CwGen_DitRequested(false, false, true));
}

static void test_dit_reversed_paddle(void)
{
    /* Reversed: DIT reads DAH line */
    TEST_ASSERT(CwGen_DitRequested(true, false, true));
    TEST_ASSERT(!CwGen_DitRequested(true, true, false));
}

static void test_dah_reversed_paddle(void)
{
    /* Reversed: DAH reads DIT line */
    TEST_ASSERT(CwGen_DahRequested(true, true, false));
    TEST_ASSERT(!CwGen_DahRequested(true, false, true));
}

static void test_both_pressed_normal(void)
{
    TEST_ASSERT(CwGen_DitRequested(false, true, true));
    TEST_ASSERT(CwGen_DahRequested(false, true, true));
}

static void test_both_pressed_reversed(void)
{
    TEST_ASSERT(CwGen_DitRequested(true, true, true));
    TEST_ASSERT(CwGen_DahRequested(true, true, true));
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */
void test_cw_gen_helpers_register(void)
{
    printf("--- cw_gen helpers ---\n");
    TEST_REGISTER(test_uppercase_lowercase_a);
    TEST_REGISTER(test_uppercase_lowercase_z);
    TEST_REGISTER(test_uppercase_already_upper);
    TEST_REGISTER(test_uppercase_non_alpha_passthrough);
    TEST_REGISTER(test_uppercase_boundary_values);
    TEST_REGISTER(test_dit_normal_paddle);
    TEST_REGISTER(test_dah_normal_paddle);
    TEST_REGISTER(test_dit_reversed_paddle);
    TEST_REGISTER(test_dah_reversed_paddle);
    TEST_REGISTER(test_both_pressed_normal);
    TEST_REGISTER(test_both_pressed_reversed);
    printf("\n");
}
