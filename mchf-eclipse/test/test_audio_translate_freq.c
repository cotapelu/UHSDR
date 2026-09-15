/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_audio_translate_freq.c                                     **
 **  Description:   Host unit tests for AudioDriver_GetTranslateFreq()              **
 **                 (IQ frequency translation mode to Hz delta)                    **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Constants matching production code (audio_driver.h)
 * ------------------------------------------------------------------------- */
#define FREQ_IQ_CONV_OFF        0
#define FREQ_IQ_CONV_P6KHZ      1
#define FREQ_IQ_CONV_M6KHZ      2
#define FREQ_IQ_CONV_P12KHZ     3
#define FREQ_IQ_CONV_M12KHZ     4
#define FREQ_IQ_CONV_MODE_DEFAULT FREQ_IQ_CONV_M12KHZ

/* -------------------------------------------------------------------------
 * Minimal transceiver state for testing
 * ------------------------------------------------------------------------- */
typedef struct {
    uint8_t iq_freq_mode;
} test_transceiver_state_t;

static test_transceiver_state_t ts;

/* -------------------------------------------------------------------------
 * Reimplemented AudioDriver_GetTranslateFreq (from audio_driver.c)
 * ------------------------------------------------------------------------- */
static int32_t AudioDriver_GetTranslateFreq(void)
{
    int32_t fdelta = 0;
    switch (ts.iq_freq_mode)
    {
    case FREQ_IQ_CONV_P6KHZ:
        fdelta = 6000;
        break;
    case FREQ_IQ_CONV_M6KHZ:
        fdelta = -6000;
        break;
    case FREQ_IQ_CONV_P12KHZ:
        fdelta = 12000;
        break;
    case FREQ_IQ_CONV_M12KHZ:
        fdelta = -12000;
        break;
    }
    return fdelta;
}

/* =========================================================================
 * AudioDriver_GetTranslateFreq tests
 * ========================================================================= */

static void test_off_returns_zero(void)
{
    ts.iq_freq_mode = FREQ_IQ_CONV_OFF;
    TEST_ASSERT_EQUAL(0, AudioDriver_GetTranslateFreq());
}

static void test_p6khz_returns_positive(void)
{
    ts.iq_freq_mode = FREQ_IQ_CONV_P6KHZ;
    TEST_ASSERT_EQUAL(6000, AudioDriver_GetTranslateFreq());
}

static void test_m6khz_returns_negative(void)
{
    ts.iq_freq_mode = FREQ_IQ_CONV_M6KHZ;
    TEST_ASSERT_EQUAL(-6000, AudioDriver_GetTranslateFreq());
}

static void test_p12khz_returns_positive(void)
{
    ts.iq_freq_mode = FREQ_IQ_CONV_P12KHZ;
    TEST_ASSERT_EQUAL(12000, AudioDriver_GetTranslateFreq());
}

static void test_m12khz_returns_negative(void)
{
    ts.iq_freq_mode = FREQ_IQ_CONV_M12KHZ;
    TEST_ASSERT_EQUAL(-12000, AudioDriver_GetTranslateFreq());
}

static void test_unknown_mode_returns_zero(void)
{
    ts.iq_freq_mode = 255;
    TEST_ASSERT_EQUAL(0, AudioDriver_GetTranslateFreq());
}

static void test_default_mode_returns_m12khz(void)
{
    ts.iq_freq_mode = FREQ_IQ_CONV_MODE_DEFAULT;
    TEST_ASSERT_EQUAL(-12000, AudioDriver_GetTranslateFreq());
}

static void test_switch_between_modes(void)
{
    ts.iq_freq_mode = FREQ_IQ_CONV_P6KHZ;
    TEST_ASSERT_EQUAL(6000, AudioDriver_GetTranslateFreq());
    ts.iq_freq_mode = FREQ_IQ_CONV_M12KHZ;
    TEST_ASSERT_EQUAL(-12000, AudioDriver_GetTranslateFreq());
    ts.iq_freq_mode = FREQ_IQ_CONV_OFF;
    TEST_ASSERT_EQUAL(0, AudioDriver_GetTranslateFreq());
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_audio_translate_freq_register(void)
{
    printf("--- audio_translate_freq ---\n");
    TEST_REGISTER(test_off_returns_zero);
    TEST_REGISTER(test_p6khz_returns_positive);
    TEST_REGISTER(test_m6khz_returns_negative);
    TEST_REGISTER(test_p12khz_returns_positive);
    TEST_REGISTER(test_m12khz_returns_negative);
    TEST_REGISTER(test_unknown_mode_returns_zero);
    TEST_REGISTER(test_default_mode_returns_m12khz);
    TEST_REGISTER(test_switch_between_modes);
    printf("\n");
}
