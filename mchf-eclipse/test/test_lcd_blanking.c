/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                             Host unit tests: LCD blanking timer                 **
 **                                                                                 **
 **--------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_lcd_blanking.c                                             **
 **  Description:   Tests for UiDriver_LcdBlankingStartTimer()                      **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"

/* -------------------------------------------------------------------------
 * Reimplemented logic under test
 * ------------------------------------------------------------------------- */

#define LCD_BLANKING_ENABLE   0x80
#define LCD_BLANKING_TIMEMASK 0x0F

typedef struct
{
    unsigned int lcd_backlight_blanking;
    unsigned int sysclock;
    unsigned int lcd_blanking_time;
    bool lcd_blanking_flag;
} UiDriverState;

static void UiDriver_LcdBlankingStartTimer_under_test(UiDriverState *ts)
{
    if (ts->lcd_backlight_blanking & LCD_BLANKING_ENABLE)
    {
        unsigned int ltemp = ts->lcd_backlight_blanking & LCD_BLANKING_TIMEMASK;
        ltemp *= 100;
        ts->lcd_blanking_time = ltemp + ts->sysclock;
        ts->lcd_blanking_flag = false;
    }
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

static void test_blanking_disabled_does_nothing(void)
{
    UiDriverState ts;
    memset(&ts, 0, sizeof(ts));
    ts.sysclock = 1000;
    ts.lcd_blanking_flag = true;

    UiDriver_LcdBlankingStartTimer_under_test(&ts);

    TEST_ASSERT_EQUAL(0, ts.lcd_backlight_blanking);
    TEST_ASSERT_EQUAL(1000, ts.sysclock);
    TEST_ASSERT_EQUAL(0, ts.lcd_blanking_time);
    TEST_ASSERT_EQUAL(1, ts.lcd_blanking_flag); /* unchanged */
}

static void test_blanking_enabled_sets_flag_false(void)
{
    UiDriverState ts;
    memset(&ts, 0, sizeof(ts));
    ts.lcd_backlight_blanking = LCD_BLANKING_ENABLE | 5; /* 5 seconds */
    ts.sysclock = 2000;
    ts.lcd_blanking_flag = true;

    UiDriver_LcdBlankingStartTimer_under_test(&ts);

    TEST_ASSERT_EQUAL(0, ts.lcd_blanking_flag);
}

static void test_blanking_time_calculation(void)
{
    UiDriverState ts;
    memset(&ts, 0, sizeof(ts));
    ts.lcd_backlight_blanking = LCD_BLANKING_ENABLE | 3; /* 3 seconds */
    ts.sysclock = 1000;

    UiDriver_LcdBlankingStartTimer_under_test(&ts);

    TEST_ASSERT_EQUAL(1300, ts.lcd_blanking_time); /* 3 * 100 + 1000 */
}

static void test_blanking_max_timing(void)
{
    UiDriverState ts;
    memset(&ts, 0, sizeof(ts));
    ts.lcd_backlight_blanking = LCD_BLANKING_ENABLE | 0x0F; /* max 15 seconds */
    ts.sysclock = 500;

    UiDriver_LcdBlankingStartTimer_under_test(&ts);

    TEST_ASSERT_EQUAL(2000, ts.lcd_blanking_time); /* 15 * 100 + 500 */
}

static void test_blanking_zero_timing_when_enabled(void)
{
    UiDriverState ts;
    memset(&ts, 0, sizeof(ts));
    ts.lcd_backlight_blanking = LCD_BLANKING_ENABLE | 0; /* 0 seconds but enabled */
    ts.sysclock = 777;

    UiDriver_LcdBlankingStartTimer_under_test(&ts);

    TEST_ASSERT_EQUAL(777, ts.lcd_blanking_time); /* 0 * 100 + 777 */
    TEST_ASSERT_EQUAL(0, ts.lcd_blanking_flag);
}

static void test_blanking_preserves_other_bits_in_config(void)
{
    UiDriverState ts;
    memset(&ts, 0, sizeof(ts));
    ts.lcd_backlight_blanking = 0x40 | LCD_BLANKING_ENABLE | 2; /* bit 6 set (reserved) */
    ts.sysclock = 100;

    UiDriver_LcdBlankingStartTimer_under_test(&ts);

    TEST_ASSERT_EQUAL(0xC0 | LCD_BLANKING_ENABLE | 2, ts.lcd_backlight_blanking);
    TEST_ASSERT_EQUAL(300, ts.lcd_blanking_time); /* 2 * 100 + 100 */
}

static void test_blanking_sysclock_unchanged(void)
{
    UiDriverState ts;
    memset(&ts, 0, sizeof(ts));
    ts.lcd_backlight_blanking = LCD_BLANKING_ENABLE | 7;
    ts.sysclock = 12345;

    UiDriver_LcdBlankingStartTimer_under_test(&ts);

    TEST_ASSERT_EQUAL(12345, ts.sysclock);
}

static void test_blanking_multiple_calls_same_result(void)
{
    UiDriverState ts;
    memset(&ts, 0, sizeof(ts));
    ts.lcd_backlight_blanking = LCD_BLANKING_ENABLE | 4;
    ts.sysclock = 500;

    UiDriver_LcdBlankingStartTimer_under_test(&ts);
    unsigned int first_time = ts.lcd_blanking_time;

    UiDriver_LcdBlankingStartTimer_under_test(&ts);
    unsigned int second_time = ts.lcd_blanking_time;

    TEST_ASSERT_EQUAL(first_time, second_time);
    TEST_ASSERT_EQUAL(900, first_time); /* 4 * 100 + 500 */
}

static void test_blanking_disable_then_enable(void)
{
    UiDriverState ts;
    memset(&ts, 0, sizeof(ts));
    ts.sysclock = 100;

    /* First call: disabled */
    ts.lcd_backlight_blanking = 0;
    UiDriver_LcdBlankingStartTimer_under_test(&ts);
    TEST_ASSERT_EQUAL(0, ts.lcd_blanking_time);
    TEST_ASSERT_EQUAL(0, ts.lcd_blanking_flag);

    /* Second call: enabled */
    ts.lcd_backlight_blanking = LCD_BLANKING_ENABLE | 2;
    UiDriver_LcdBlankingStartTimer_under_test(&ts);
    TEST_ASSERT_EQUAL(300, ts.lcd_blanking_time);
    TEST_ASSERT_EQUAL(0, ts.lcd_blanking_flag);
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */

void test_lcd_blanking_register(void)
{
    printf("--- LCD blanking timer (UiDriver_LcdBlankingStartTimer) ---\n");
    TEST_REGISTER(test_blanking_disabled_does_nothing);
    TEST_REGISTER(test_blanking_enabled_sets_flag_false);
    TEST_REGISTER(test_blanking_time_calculation);
    TEST_REGISTER(test_blanking_max_timing);
    TEST_REGISTER(test_blanking_zero_timing_when_enabled);
    TEST_REGISTER(test_blanking_preserves_other_bits_in_config);
    TEST_REGISTER(test_blanking_sysclock_unchanged);
    TEST_REGISTER(test_blanking_multiple_calls_same_result);
    TEST_REGISTER(test_blanking_disable_then_enable);
    printf("\n");
}
