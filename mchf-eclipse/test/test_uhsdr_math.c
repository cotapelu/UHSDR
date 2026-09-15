/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_uhsdr_math.c                                               **
 **  Description:   Host unit tests for uhsdr_math pure math helpers                **
 **  Last Modified: 2026-08-28                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include <stdint.h>
#include <math.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Reimplementation of uhsdr_math.c pure helpers for host testing
 * (avoids pulling in CMSIS DSP dependency)
 * ------------------------------------------------------------------------- */
typedef float float32_t;

static float32_t Math_log10f_fast(float32_t X)
{
    float Y, F;
    int E;
    F = frexpf(fabsf(X), &E);
    Y = 1.23149591368684f;
    Y *= F;
    Y += -4.11852516267426f;
    Y *= F;
    Y += 6.02197014179219f;
    Y *= F;
    Y += -3.13396450166353f;
    Y += E;
    return (Y * 0.3010299956639812f);
}

static float32_t Math_absmax(float32_t* buffer, int size)
{
    float32_t min, max;

    /* Simplified: use standard library min/max for host testing */
    if (size <= 0) return 0.0f;
    max = buffer[0];
    min = buffer[0];
    for (int i = 1; i < size; i++)
    {
        if (buffer[i] > max) max = buffer[i];
        if (buffer[i] < min) min = buffer[i];
    }
    return -min > max ? -min : max;
}

static float32_t Math_sign_new(float32_t x)
{
    return (x < 0.0f) ? -1.0f : ((x > 0.0f) ? 1.0f : 0.0f);
}

/* -------------------------------------------------------------------------
 * Tests: Math_sign_new
 * ------------------------------------------------------------------------- */
static void test_sign_new_positive(void)
{
    TEST_ASSERT_FLOAT_EQ(1.0f, Math_sign_new(5.0f), 1e-6f);
}

static void test_sign_new_negative(void)
{
    TEST_ASSERT_FLOAT_EQ(-1.0f, Math_sign_new(-3.5f), 1e-6f);
}

static void test_sign_new_zero(void)
{
    TEST_ASSERT_FLOAT_EQ(0.0f, Math_sign_new(0.0f), 1e-6f);
}

static void test_sign_new_negative_zero(void)
{
    TEST_ASSERT_FLOAT_EQ(0.0f, Math_sign_new(-0.0f), 1e-6f);
}

/* -------------------------------------------------------------------------
 * Tests: Math_absmax
 * ------------------------------------------------------------------------- */
static void test_absmax_all_positive(void)
{
    float32_t buf[] = {1.0f, 2.0f, 3.0f, 4.0f};
    TEST_ASSERT_FLOAT_EQ(4.0f, Math_absmax(buf, 4), 1e-6f);
}

static void test_absmax_all_negative(void)
{
    float32_t buf[] = {-1.0f, -5.0f, -3.0f};
    TEST_ASSERT_FLOAT_EQ(5.0f, Math_absmax(buf, 3), 1e-6f);
}

static void test_absmax_mixed(void)
{
    float32_t buf[] = {-2.0f, 1.0f, -5.0f, 3.0f};
    TEST_ASSERT_FLOAT_EQ(5.0f, Math_absmax(buf, 4), 1e-6f);
}

static void test_absmax_single_element(void)
{
    float32_t buf[] = {-7.5f};
    TEST_ASSERT_FLOAT_EQ(7.5f, Math_absmax(buf, 1), 1e-6f);
}

static void test_absmax_zero_size(void)
{
    float32_t buf[] = {1.0f, 2.0f};
    TEST_ASSERT_FLOAT_EQ(0.0f, Math_absmax(buf, 0), 1e-6f);
}

static void test_absmax_all_zeros(void)
{
    float32_t buf[] = {0.0f, 0.0f, 0.0f};
    TEST_ASSERT_FLOAT_EQ(0.0f, Math_absmax(buf, 3), 1e-6f);
}

/* -------------------------------------------------------------------------
 * Tests: Math_log10f_fast
 * ------------------------------------------------------------------------- */
static void test_log10_fast_one(void)
{
    TEST_ASSERT_FLOAT_EQ(0.0f, Math_log10f_fast(1.0f), 0.1f);
}

static void test_log10_fast_ten(void)
{
    TEST_ASSERT_FLOAT_EQ(1.0f, Math_log10f_fast(10.0f), 0.1f);
}

static void test_log10_fast_hundred(void)
{
    TEST_ASSERT_FLOAT_EQ(2.0f, Math_log10f_fast(100.0f), 0.1f);
}

static void test_log10_fast_thousand(void)
{
    TEST_ASSERT_FLOAT_EQ(3.0f, Math_log10f_fast(1000.0f), 0.1f);
}

static void test_log10_fast_fraction(void)
{
    TEST_ASSERT_FLOAT_EQ(-1.0f, Math_log10f_fast(0.1f), 0.1f);
}

static void test_log10_fast_negative_input(void)
{
    /* log10 of negative should be handled by fabsf inside */
    TEST_ASSERT_FLOAT_EQ(1.0f, Math_log10f_fast(-10.0f), 0.1f);
}

static void test_log10_fast_zero(void)
{
    /* Note: frexpf(0) returns F=0, E=0; polynomial gives ~-0.94 (not -inf) */
    /* This is a known limitation of the fast approximation */
    float32_t result = Math_log10f_fast(0.0f);
    TEST_ASSERT(result < 0.0f);
    TEST_ASSERT(result > -2.0f);
}

static void test_log10_fast_large(void)
{
    TEST_ASSERT_FLOAT_EQ(6.0f, Math_log10f_fast(1000000.0f), 0.1f);
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */
void test_uhsdr_math_register(void)
{
    printf("--- uhsdr_math ---\n");
    TEST_REGISTER(test_sign_new_positive);
    TEST_REGISTER(test_sign_new_negative);
    TEST_REGISTER(test_sign_new_zero);
    TEST_REGISTER(test_sign_new_negative_zero);
    TEST_REGISTER(test_absmax_all_positive);
    TEST_REGISTER(test_absmax_all_negative);
    TEST_REGISTER(test_absmax_mixed);
    TEST_REGISTER(test_absmax_single_element);
    TEST_REGISTER(test_absmax_zero_size);
    TEST_REGISTER(test_absmax_all_zeros);
    TEST_REGISTER(test_log10_fast_one);
    TEST_REGISTER(test_log10_fast_ten);
    TEST_REGISTER(test_log10_fast_hundred);
    TEST_REGISTER(test_log10_fast_thousand);
    TEST_REGISTER(test_log10_fast_fraction);
    TEST_REGISTER(test_log10_fast_negative_input);
    TEST_REGISTER(test_log10_fast_zero);
    TEST_REGISTER(test_log10_fast_large);
    printf("\n");
}
