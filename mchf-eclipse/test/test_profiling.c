/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_profiling.c                                                **
 **  Description:   Host unit tests for profiling WCET calculation logic            **
 **  Last Modified: 2026-08-28                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include <stdint.h>

/* -------------------------------------------------------------------------
 * Standalone WCET calculation helpers (mirrors misc/profiling.c logic)
 * ------------------------------------------------------------------------- */
typedef enum {
    ProfileAudioISR_WCET = 12,
    ProfilePendSV_WCET = 13,
    ProfileMainLoop_WCET = 14,
    ProfileSpectrum_WCET = 15,
    ProfileMenu_WCET = 16,
    ProfileOther = 99
} ProfiledEventNames;

typedef struct {
    uint32_t count;
    uint64_t duration;
    uint32_t wcet;
} ProfilingTimedEvent;

static uint32_t wcet_calc_avg_us(uint64_t duration, uint32_t count, uint32_t cpu_hz)
{
    if (count == 0) return 0;
    return (uint32_t)((duration / count) / (cpu_hz / 1000000));
}

static uint32_t wcet_calc_wcet_us(uint32_t wcet_cycles, uint32_t cpu_hz)
{
    return wcet_cycles / (cpu_hz / 1000000);
}

static uint32_t wcet_calc_budget_us(ProfiledEventNames event)
{
    switch (event) {
        case ProfileAudioISR_WCET:    return 100;
        case ProfilePendSV_WCET:      return 500;
        case ProfileMainLoop_WCET:    return 10000;
        case ProfileSpectrum_WCET:    return 8000;
        case ProfileMenu_WCET:        return 5000;
        default:                      return 0;
    }
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/* Test: avg_us = 0 when count is 0 (avoid divide-by-zero) */
static void test_wcet_avg_zero_count(void)
{
    TEST_ASSERT_EQUAL(0, wcet_calc_avg_us(1000, 0, 168000000));
}

/* Test: avg_us calculation at 168 MHz */
static void test_wcet_avg_168mhz(void)
{
    /* duration=168000 cycles, count=100, cpu_hz=168000000 */
    /* avg = (168000/100) / (168000000/1000000) = 1680 / 168 = 10 us */
    TEST_ASSERT_EQUAL(10, wcet_calc_avg_us(168000, 100, 168000000));
}

/* Test: wcet_us calculation at 168 MHz */
static void test_wcet_wcet_168mhz(void)
{
    /* wcet_cycles=168000, cpu_hz=168000000 */
    /* wcet_us = 168000 / 168 = 1000 us */
    TEST_ASSERT_EQUAL(1000, wcet_calc_wcet_us(168000, 168000000));
}

/* Test: wcet_us = 0 when wcet_cycles = 0 */
static void test_wcet_wcet_zero_cycles(void)
{
    TEST_ASSERT_EQUAL(0, wcet_calc_wcet_us(0, 168000000));
}

/* Test: budget assignment for known events */
static void test_wcet_budget_audio_isr(void)
{
    TEST_ASSERT_EQUAL(100, wcet_calc_budget_us(ProfileAudioISR_WCET));
}

static void test_wcet_budget_pendsv(void)
{
    TEST_ASSERT_EQUAL(500, wcet_calc_budget_us(ProfilePendSV_WCET));
}

static void test_wcet_budget_main_loop(void)
{
    TEST_ASSERT_EQUAL(10000, wcet_calc_budget_us(ProfileMainLoop_WCET));
}

static void test_wcet_budget_spectrum(void)
{
    TEST_ASSERT_EQUAL(8000, wcet_calc_budget_us(ProfileSpectrum_WCET));
}

static void test_wcet_budget_menu(void)
{
    TEST_ASSERT_EQUAL(5000, wcet_calc_budget_us(ProfileMenu_WCET));
}

static void test_wcet_budget_unknown_event(void)
{
    TEST_ASSERT_EQUAL(0, wcet_calc_budget_us(ProfileOther));
}

/* Test: avg_us rounds down correctly (integer division) */
static void test_wcet_avg_rounds_down(void)
{
    /* 169000 cycles / 100 = 1690; 1690 / 168 = 10 (truncated) */
    TEST_ASSERT_EQUAL(10, wcet_calc_avg_us(169000, 100, 168000000));
}

/* Test: wcet_us rounds down correctly (integer division) */
static void test_wcet_wcet_rounds_down(void)
{
    /* 169000 / 168 = 1005 (truncated from 1005.95...) */
    TEST_ASSERT_EQUAL(1005, wcet_calc_wcet_us(169000, 168000000));
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */
void test_profiling_register(void)
{
    printf("--- Profiling WCET Logic ---\n");
    TEST_REGISTER(test_wcet_avg_zero_count);
    TEST_REGISTER(test_wcet_avg_168mhz);
    TEST_REGISTER(test_wcet_wcet_168mhz);
    TEST_REGISTER(test_wcet_wcet_zero_cycles);
    TEST_REGISTER(test_wcet_budget_audio_isr);
    TEST_REGISTER(test_wcet_budget_pendsv);
    TEST_REGISTER(test_wcet_budget_main_loop);
    TEST_REGISTER(test_wcet_budget_spectrum);
    TEST_REGISTER(test_wcet_budget_menu);
    TEST_REGISTER(test_wcet_budget_unknown_event);
    TEST_REGISTER(test_wcet_avg_rounds_down);
    TEST_REGISTER(test_wcet_wcet_rounds_down);
    printf("\n");
}
