/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_decayavg.c                                                 **
 **  Description:   Host unit tests for decayavg() exponential moving average       **
 **                 (reimplemented inline to avoid DSP/float32_t dependencies)      **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include <math.h>

/* -------------------------------------------------------------------------
 * Reimplemented decayavg logic (avoids float32_t/arm_math dependencies)
 * ------------------------------------------------------------------------- */
static float decayavg(float average, float input, int weight)
{
    float retval;
    if (weight <= 1)
    {
        retval = input;
    }
    else
    {
        retval = ((input - average) / (float)weight) + average;
    }
    return retval;
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */
static void test_weight_zero_returns_input(void)
{
    TEST_ASSERT_FLOAT_EQ(5.0f, decayavg(10.0f, 5.0f, 0), 0.001f);
}

static void test_weight_one_returns_input(void)
{
    TEST_ASSERT_FLOAT_EQ(5.0f, decayavg(10.0f, 5.0f, 1), 0.001f);
}

static void test_weight_two_returns_midpoint(void)
{
    TEST_ASSERT_FLOAT_EQ(7.5f, decayavg(10.0f, 5.0f, 2), 0.001f);
}

static void test_weight_three_weighted_average(void)
{
    /* (5 - 10) / 3 + 10 = -5/3 + 10 = 8.333... */
    TEST_ASSERT_FLOAT_EQ(8.33333f, decayavg(10.0f, 5.0f, 3), 0.001f);
}

static void test_large_weight_approaches_average(void)
{
    /* (5 - 100) / 100 + 100 = -0.95 + 100 = 99.05 */
    TEST_ASSERT_FLOAT_EQ(99.05f, decayavg(100.0f, 5.0f, 100), 0.001f);
}

static void test_negative_values(void)
{
    TEST_ASSERT_FLOAT_EQ(-7.5f, decayavg(-10.0f, -5.0f, 2), 0.001f);
}

static void test_mixed_sign_values(void)
{
    /* (5 - (-10)) / 2 + (-10) = 15/2 - 10 = 7.5 - 10 = -2.5 */
    TEST_ASSERT_FLOAT_EQ(-2.5f, decayavg(-10.0f, 5.0f, 2), 0.001f);
}

static void test_zero_average(void)
{
    TEST_ASSERT_FLOAT_EQ(2.5f, decayavg(0.0f, 5.0f, 2), 0.001f);
}

static void test_zero_input(void)
{
    TEST_ASSERT_FLOAT_EQ(5.0f, decayavg(10.0f, 0.0f, 2), 0.001f);
}

static void test_equal_values_unchanged(void)
{
    TEST_ASSERT_FLOAT_EQ(7.0f, decayavg(7.0f, 7.0f, 10), 0.001f);
}

static void test_convergence_sequence(void)
{
    float avg = 100.0f;
    avg = decayavg(avg, 0.0f, 4);
    avg = decayavg(avg, 0.0f, 4);
    avg = decayavg(avg, 0.0f, 4);
    /* After 3 steps with weight 4: 100 -> 75 -> 56.25 -> 42.1875 */
    TEST_ASSERT_FLOAT_EQ(42.1875f, avg, 0.001f);
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */
void test_decayavg_register(void)
{
    printf("--- decayavg ---\n");
    TEST_REGISTER(test_weight_zero_returns_input);
    TEST_REGISTER(test_weight_one_returns_input);
    TEST_REGISTER(test_weight_two_returns_midpoint);
    TEST_REGISTER(test_weight_three_weighted_average);
    TEST_REGISTER(test_large_weight_approaches_average);
    TEST_REGISTER(test_negative_values);
    TEST_REGISTER(test_mixed_sign_values);
    TEST_REGISTER(test_zero_average);
    TEST_REGISTER(test_zero_input);
    TEST_REGISTER(test_equal_values_unchanged);
    TEST_REGISTER(test_convergence_sequence);
    printf("\n");
}
