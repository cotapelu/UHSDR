/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_ui_driver_utils.c                                          **
 **  Description:   Host unit tests for ui_driver_utils clamp/wrap helpers          **
 **  Last Modified: 2026-08-28                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include <stdint.h>

/* -------------------------------------------------------------------------
 * Reimplementation of ui_driver_utils.c helpers for host testing
 * ------------------------------------------------------------------------- */
int32_t change_and_limit_int(int32_t val, int32_t change, int32_t min, int32_t max)
{
    val += change;
    if (val < min)
    {
        val = min;
    }
    else if (val > max)
    {
        val = max;
    }
    return val;
}

uint32_t change_and_limit_uint(uint32_t val, int32_t change, uint32_t min, uint32_t max)
{
    if (change < 0 && (-change > (int32_t)(val - min)))
    {
        val = min;
    }
    else if (change > 0 && (uint32_t)change > max - val)
    {
        val = max;
    }
    else
    {
        val += change;
    }
    return val;
}

uint32_t change_and_wrap_uint(uint32_t val, int32_t change, uint32_t min, uint32_t max)
{
    if (change > ((int32_t)max - (int32_t)val))
    {
        val = min;
    }
    else if ((change + (int32_t)val) < (int32_t)min)
    {
        val = max;
    }
    else
    {
        val += change;
    }
    return val;
}

static void incr_wrap_uint8(volatile uint8_t* ptr, uint8_t min, uint8_t max)
{
    *ptr = (uint8_t)(change_and_wrap_uint(*ptr, +1, min, max));
}

static void decr_wrap_uint8(volatile uint8_t* ptr, uint8_t min, uint8_t max)
{
    *ptr = (uint8_t)(change_and_wrap_uint(*ptr, -1, min, max));
}

static void incr_wrap_uint16(volatile uint16_t* ptr, uint16_t min, uint16_t max)
{
    *ptr = (uint16_t)(change_and_wrap_uint(*ptr, +1, min, max));
}

static void decr_wrap_uint16(volatile uint16_t* ptr, uint16_t min, uint16_t max)
{
    *ptr = (uint16_t)(change_and_wrap_uint(*ptr, -1, min, max));
}

/* -------------------------------------------------------------------------
 * Tests: change_and_limit_int
 * ------------------------------------------------------------------------- */
static void test_limit_int_no_change(void)
{
    TEST_ASSERT_EQUAL(50, change_and_limit_int(50, 0, 0, 100));
}

static void test_limit_int_clamp_above_max(void)
{
    TEST_ASSERT_EQUAL(100, change_and_limit_int(50, 60, 0, 100));
}

static void test_limit_int_clamp_below_min(void)
{
    TEST_ASSERT_EQUAL(0, change_and_limit_int(50, -60, 0, 100));
}

static void test_limit_int_negative_range(void)
{
    TEST_ASSERT_EQUAL(-10, change_and_limit_int(-5, -5, -20, 0));
    TEST_ASSERT_EQUAL(0, change_and_limit_int(-5, 10, -20, 0));
}

static void test_limit_int_exact_boundary(void)
{
    TEST_ASSERT_EQUAL(100, change_and_limit_int(90, 10, 0, 100));
    TEST_ASSERT_EQUAL(0, change_and_limit_int(10, -10, 0, 100));
}

/* -------------------------------------------------------------------------
 * Tests: change_and_limit_uint
 * ------------------------------------------------------------------------- */
static void test_limit_uint_no_change(void)
{
    TEST_ASSERT_EQUAL(50, change_and_limit_uint(50, 0, 0, 100));
}

static void test_limit_uint_clamp_above_max(void)
{
    TEST_ASSERT_EQUAL(100, change_and_limit_uint(50, 60, 0, 100));
}

static void test_limit_uint_clamp_below_min(void)
{
    TEST_ASSERT_EQUAL(0, change_and_limit_uint(50, -60, 0, 100));
}

static void test_limit_uint_negative_change_underflow(void)
{
    TEST_ASSERT_EQUAL(0, change_and_limit_uint(30, -40, 0, 100));
}

static void test_limit_uint_exact_max(void)
{
    TEST_ASSERT_EQUAL(100, change_and_limit_uint(90, 10, 0, 100));
}

static void test_limit_uint_exact_min(void)
{
    TEST_ASSERT_EQUAL(20, change_and_limit_uint(10, 10, 0, 100));
}

/* -------------------------------------------------------------------------
 * Tests: change_and_wrap_uint
 * ------------------------------------------------------------------------- */
static void test_wrap_uint_no_change(void)
{
    TEST_ASSERT_EQUAL(50, change_and_wrap_uint(50, 0, 0, 100));
}

static void test_wrap_uint_overflow_wraps_to_min(void)
{
    TEST_ASSERT_EQUAL(0, change_and_wrap_uint(90, 20, 0, 100));
}

static void test_wrap_uint_underflow_wraps_to_max(void)
{
    TEST_ASSERT_EQUAL(100, change_and_wrap_uint(10, -20, 0, 100));
}

static void test_wrap_uint_exact_max_no_wrap(void)
{
    TEST_ASSERT_EQUAL(100, change_and_wrap_uint(90, 10, 0, 100));
}

static void test_wrap_uint_exact_min_no_wrap(void)
{
    TEST_ASSERT_EQUAL(0, change_and_wrap_uint(10, -10, 0, 100));
}

/* -------------------------------------------------------------------------
 * Tests: incr/decr wrap helpers
 * ------------------------------------------------------------------------- */
static void test_incr_wrap_uint8_middle(void)
{
    uint8_t val = 50;
    incr_wrap_uint8(&val, 0, 100);
    TEST_ASSERT_EQUAL(51, val);
}

static void test_incr_wrap_uint8_at_max(void)
{
    uint8_t val = 100;
    incr_wrap_uint8(&val, 0, 100);
    TEST_ASSERT_EQUAL(0, val);
}

static void test_decr_wrap_uint8_at_min(void)
{
    uint8_t val = 0;
    decr_wrap_uint8(&val, 0, 100);
    TEST_ASSERT_EQUAL(100, val);
}

static void test_incr_wrap_uint16_middle(void)
{
    uint16_t val = 1000;
    incr_wrap_uint16(&val, 0, 2000);
    TEST_ASSERT_EQUAL(1001, val);
}

static void test_incr_wrap_uint16_at_max(void)
{
    uint16_t val = 2000;
    incr_wrap_uint16(&val, 0, 2000);
    TEST_ASSERT_EQUAL(0, val);
}

static void test_decr_wrap_uint16_at_min(void)
{
    uint16_t val = 0;
    decr_wrap_uint16(&val, 0, 2000);
    TEST_ASSERT_EQUAL(2000, val);
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */
void test_ui_driver_utils_register(void)
{
    printf("--- ui_driver_utils ---\n");
    TEST_REGISTER(test_limit_int_no_change);
    TEST_REGISTER(test_limit_int_clamp_above_max);
    TEST_REGISTER(test_limit_int_clamp_below_min);
    TEST_REGISTER(test_limit_int_negative_range);
    TEST_REGISTER(test_limit_int_exact_boundary);
    TEST_REGISTER(test_limit_uint_no_change);
    TEST_REGISTER(test_limit_uint_clamp_above_max);
    TEST_REGISTER(test_limit_uint_clamp_below_min);
    TEST_REGISTER(test_limit_uint_negative_change_underflow);
    TEST_REGISTER(test_limit_uint_exact_max);
    TEST_REGISTER(test_limit_uint_exact_min);
    TEST_REGISTER(test_wrap_uint_no_change);
    TEST_REGISTER(test_wrap_uint_overflow_wraps_to_min);
    TEST_REGISTER(test_wrap_uint_underflow_wraps_to_max);
    TEST_REGISTER(test_wrap_uint_exact_max_no_wrap);
    TEST_REGISTER(test_wrap_uint_exact_min_no_wrap);
    TEST_REGISTER(test_incr_wrap_uint8_middle);
    TEST_REGISTER(test_incr_wrap_uint8_at_max);
    TEST_REGISTER(test_decr_wrap_uint8_at_min);
    TEST_REGISTER(test_incr_wrap_uint16_middle);
    TEST_REGISTER(test_incr_wrap_uint16_at_max);
    TEST_REGISTER(test_decr_wrap_uint16_at_min);
    printf("\n");
}
