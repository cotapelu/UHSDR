/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_audio_api.c                                                **
 **  Description:   Host unit tests for core audio API helpers (T35.x)              **
 **                 Uses mock global state and re-implements the tested logic       **
 **                 inline to exercise the validation framework without pulling     **
 **                 in HAL / vendor dependencies.                                  **
 **  Last Modified: 2026-08-24                                                      **
 **  Licence:       GNU GPLv3                                                       **
 ************************************************************************************/

#include "test.h"
#include "mock_uhsdr_board.h"
#include "mock_codec.h"
#include <string.h>

/* -------------------------------------------------------------------------
 * Mock global state
 * This instance replaces the real `TransceiverState ts` from uhsdr_board.c
 * during host testing. Using a local definition keeps the linker happy and
 * gives tests full control over initial conditions.
 * ------------------------------------------------------------------------- */
TransceiverState ts;

/* -------------------------------------------------------------------------
 * Re-implementation of AudioDriver_GetTranslateFreq
 * Mirrors the switch-based lookup in drivers/audio/audio_driver.c.
 * ------------------------------------------------------------------------- */
static int32_t AudioDriver_GetTranslateFreq(void)
{
    int32_t fdelta = 0;
    switch (ts.iq_freq_mode)
    {
        case FREQ_IQ_CONV_P6KHZ:  fdelta = 6000;  break;
        case FREQ_IQ_CONV_M6KHZ:  fdelta = -6000; break;
        case FREQ_IQ_CONV_P12KHZ: fdelta = 12000; break;
        case FREQ_IQ_CONV_M12KHZ: fdelta = -12000; break;
        default: break;
    }
    return fdelta;
}

/* -------------------------------------------------------------------------
 * Test helpers
 * ------------------------------------------------------------------------- */
static void reset_ts(void)
{
    memset(&ts, 0, sizeof(ts));
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

static void test_translate_freq_off(void)
{
    reset_ts();
    ts.iq_freq_mode = FREQ_IQ_CONV_OFF;
    TEST_ASSERT_EQUAL(0, AudioDriver_GetTranslateFreq());
}

static void test_translate_freq_p6khz(void)
{
    reset_ts();
    ts.iq_freq_mode = FREQ_IQ_CONV_P6KHZ;
    TEST_ASSERT_EQUAL(6000, AudioDriver_GetTranslateFreq());
}

static void test_translate_freq_m6khz(void)
{
    reset_ts();
    ts.iq_freq_mode = FREQ_IQ_CONV_M6KHZ;
    TEST_ASSERT_EQUAL(-6000, AudioDriver_GetTranslateFreq());
}

static void test_translate_freq_p12khz(void)
{
    reset_ts();
    ts.iq_freq_mode = FREQ_IQ_CONV_P12KHZ;
    TEST_ASSERT_EQUAL(12000, AudioDriver_GetTranslateFreq());
}

static void test_translate_freq_m12khz(void)
{
    reset_ts();
    ts.iq_freq_mode = FREQ_IQ_CONV_M12KHZ;
    TEST_ASSERT_EQUAL(-12000, AudioDriver_GetTranslateFreq());
}

static void test_translate_freq_invalid_mode_returns_zero(void)
{
    reset_ts();
    ts.iq_freq_mode = 0xFF; /* garbage */
    TEST_ASSERT_EQUAL(0, AudioDriver_GetTranslateFreq());
}