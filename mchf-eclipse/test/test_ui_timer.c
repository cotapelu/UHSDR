/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_ui_timer.c                                                 **
 **  Description:   Host unit tests for UiDriver timer utilities                    **
 **                 (reimplemented inline to avoid ui_driver.c dependencies)         **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Reimplemented timer logic (avoids ui_driver.c / SysClockTimers deps)
 * ------------------------------------------------------------------------- */
#define SCTimer_NUM 8

static uint32_t last_sysclock_seen[SCTimer_NUM];

static void timer_reset(void)
{
    for (int i = 0; i < SCTimer_NUM; i++) {
        last_sysclock_seen[i] = 0;
    }
}

static bool UiDriver_TimerIsExpired(int sct, uint32_t now, uint32_t divider)
{
    return (last_sysclock_seen[sct] != now / divider);
}

static void UiDriver_TimerRewind(int sct, uint32_t now, uint32_t divider)
{
    last_sysclock_seen[sct] = now / divider;
}

static bool UiDriver_TimerExpireAndRewind(int sct, uint32_t now, uint32_t divider)
{
    bool retval = UiDriver_TimerIsExpired(sct, now, divider);
    if (retval) {
        UiDriver_TimerRewind(sct, now, divider);
    }
    return retval;
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */
static void test_timer_not_expired_after_rewind(void)
{
    timer_reset();
    UiDriver_TimerRewind(0, 100, 4);
    TEST_ASSERT_EQUAL(false, UiDriver_TimerIsExpired(0, 100, 4));
}

static void test_timer_expired_when_now_changes(void)
{
    timer_reset();
    UiDriver_TimerRewind(0, 100, 4);
    TEST_ASSERT_EQUAL(false, UiDriver_TimerIsExpired(0, 100, 4));
    /* now=103, 103/4 = 25, stored=25 -> not expired yet */
    TEST_ASSERT_EQUAL(false, UiDriver_TimerIsExpired(0, 103, 4));
    /* now=104, 104/4 = 26, stored=25 -> expired */
    TEST_ASSERT_EQUAL(true, UiDriver_TimerIsExpired(0, 104, 4));
}

static void test_timer_not_expired_within_divider(void)
{
    timer_reset();
    UiDriver_TimerRewind(0, 100, 4);
    /* now=101, 101/4 = 25, stored=100/4=25 -> not expired */
    TEST_ASSERT_EQUAL(false, UiDriver_TimerIsExpired(0, 101, 4));
    TEST_ASSERT_EQUAL(false, UiDriver_TimerIsExpired(0, 103, 4));
    /* now=104, 104/4 = 26, stored=25 -> expired */
    TEST_ASSERT_EQUAL(true, UiDriver_TimerIsExpired(0, 104, 4));
}

static void test_timer_expire_and_rewind_resets(void)
{
    timer_reset();
    UiDriver_TimerRewind(0, 100, 4);
    bool expired = UiDriver_TimerExpireAndRewind(0, 104, 4);
    TEST_ASSERT_EQUAL(true, expired);
    /* After rewind, should not be expired again immediately */
    TEST_ASSERT_EQUAL(false, UiDriver_TimerIsExpired(0, 104, 4));
}

static void test_timer_expire_and_rewind_not_expired_no_change(void)
{
    timer_reset();
    UiDriver_TimerRewind(0, 100, 4);
    bool expired = UiDriver_TimerExpireAndRewind(0, 100, 4);
    TEST_ASSERT_EQUAL(false, expired);
    TEST_ASSERT_EQUAL(false, UiDriver_TimerIsExpired(0, 100, 4));
}

static void test_timer_multiple_timers_independent(void)
{
    timer_reset();
    UiDriver_TimerRewind(0, 100, 4);
    UiDriver_TimerRewind(1, 200, 8);
    TEST_ASSERT_EQUAL(false, UiDriver_TimerIsExpired(0, 103, 4));
    TEST_ASSERT_EQUAL(false, UiDriver_TimerIsExpired(1, 203, 8));
    TEST_ASSERT_EQUAL(true, UiDriver_TimerIsExpired(0, 104, 4));
    TEST_ASSERT_EQUAL(false, UiDriver_TimerIsExpired(1, 204, 8));
}

static void test_timer_divider_one_checks_every_tick(void)
{
    timer_reset();
    UiDriver_TimerRewind(0, 100, 1);
    TEST_ASSERT_EQUAL(false, UiDriver_TimerIsExpired(0, 100, 1));
    TEST_ASSERT_EQUAL(true, UiDriver_TimerIsExpired(0, 101, 1));
}

static void test_timer_large_divider(void)
{
    timer_reset();
    UiDriver_TimerRewind(0, 1000, 100);
    TEST_ASSERT_EQUAL(false, UiDriver_TimerIsExpired(0, 1099, 100));
    TEST_ASSERT_EQUAL(true, UiDriver_TimerIsExpired(0, 1100, 100));
}

static void test_timer_zero_now_after_rewind(void)
{
    timer_reset();
    UiDriver_TimerRewind(0, 0, 4);
    TEST_ASSERT_EQUAL(false, UiDriver_TimerIsExpired(0, 0, 4));
    TEST_ASSERT_EQUAL(true, UiDriver_TimerIsExpired(0, 4, 4));
}

static void test_timer_expire_and_rewind_multiple_cycles(void)
{
    timer_reset();
    UiDriver_TimerRewind(0, 0, 4);
    /* Cycle 1: 0-3 not expired, 4 expired */
    TEST_ASSERT_EQUAL(true, UiDriver_TimerExpireAndRewind(0, 4, 4));
    TEST_ASSERT_EQUAL(false, UiDriver_TimerIsExpired(0, 4, 4));
    /* Cycle 2: 4-7 not expired, 8 expired */
    TEST_ASSERT_EQUAL(true, UiDriver_TimerExpireAndRewind(0, 8, 4));
    TEST_ASSERT_EQUAL(false, UiDriver_TimerIsExpired(0, 8, 4));
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */
void test_ui_timer_register(void)
{
    printf("--- ui_timer ---\n");
    TEST_REGISTER(test_timer_not_expired_after_rewind);
    TEST_REGISTER(test_timer_expired_when_now_changes);
    TEST_REGISTER(test_timer_not_expired_within_divider);
    TEST_REGISTER(test_timer_expire_and_rewind_resets);
    TEST_REGISTER(test_timer_expire_and_rewind_not_expired_no_change);
    TEST_REGISTER(test_timer_multiple_timers_independent);
    TEST_REGISTER(test_timer_divider_one_checks_every_tick);
    TEST_REGISTER(test_timer_large_divider);
    TEST_REGISTER(test_timer_zero_now_after_rewind);
    TEST_REGISTER(test_timer_expire_and_rewind_multiple_cycles);
    printf("\n");
}
