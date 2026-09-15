/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_radio_management.c                                         **
 **  Description:   Host unit tests for radio_management pure helper functions      **
 **  Last Modified: 2026-08-28                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Reimplementation of radio_management.c pure helpers for host testing
 * ------------------------------------------------------------------------- */
typedef struct BandInfo
{
    uint32_t tune;
    uint32_t size;
    const char* name;
    uint32_t band_mode;
    bool rx_only;
} BandInfo;

#define DEMOD_CW        2
#define TRX_MODE_TX     1
#define FREQ_IQ_CONV_MODE_OFF 0

static inline bool RadioManagement_IsGenericBand(const BandInfo* band)
{
    return band->size == 0;
}

static bool RadioManagement_FreqIsInBand(const BandInfo* bandinfo, const uint32_t freq)
{
    return (freq >= bandinfo->tune) && (freq <= (bandinfo->tune + bandinfo->size));
}

static uint32_t RadioManagement_GetRealFreqTranslationMode(uint32_t txrx_mode, uint32_t dmod_mode, uint32_t iq_freq_mode)
{
    uint32_t retval = iq_freq_mode;
    if (dmod_mode == DEMOD_CW && txrx_mode == TRX_MODE_TX)
    {
        retval = FREQ_IQ_CONV_MODE_OFF;
    }
    return retval;
}

/* -------------------------------------------------------------------------
 * Tests: RadioManagement_IsGenericBand
 * ------------------------------------------------------------------------- */
static void test_is_generic_band_size_zero(void)
{
    BandInfo band = { .tune = 0, .size = 0, .name = "Gen", .band_mode = 0, .rx_only = false };
    TEST_ASSERT(RadioManagement_IsGenericBand(&band));
}

static void test_is_generic_band_size_nonzero(void)
{
    BandInfo band = { .tune = 14000000, .size = 350000, .name = "20m", .band_mode = 0, .rx_only = false };
    TEST_ASSERT(!RadioManagement_IsGenericBand(&band));
}

static void test_is_generic_band_large_size(void)
{
    BandInfo band = { .tune = 430000000, .size = 20000000, .name = "70cm", .band_mode = 0, .rx_only = false };
    TEST_ASSERT(!RadioManagement_IsGenericBand(&band));
}

/* -------------------------------------------------------------------------
 * Tests: RadioManagement_FreqIsInBand
 * ------------------------------------------------------------------------- */
static void test_freq_in_band_at_tune(void)
{
    BandInfo band = { .tune = 14000000, .size = 350000, .name = "20m", .band_mode = 0, .rx_only = false };
    TEST_ASSERT(RadioManagement_FreqIsInBand(&band, 14000000));
}

static void test_freq_in_band_at_upper_edge(void)
{
    BandInfo band = { .tune = 14000000, .size = 350000, .name = "20m", .band_mode = 0, .rx_only = false };
    TEST_ASSERT(RadioManagement_FreqIsInBand(&band, 14350000));
}

static void test_freq_in_band_middle(void)
{
    BandInfo band = { .tune = 7000000, .size = 200000, .name = "40m", .band_mode = 0, .rx_only = false };
    TEST_ASSERT(RadioManagement_FreqIsInBand(&band, 7100000));
}

static void test_freq_below_band(void)
{
    BandInfo band = { .tune = 14000000, .size = 350000, .name = "20m", .band_mode = 0, .rx_only = false };
    TEST_ASSERT(!RadioManagement_FreqIsInBand(&band, 13999999));
}

static void test_freq_above_band(void)
{
    BandInfo band = { .tune = 14000000, .size = 350000, .name = "20m", .band_mode = 0, .rx_only = false };
    TEST_ASSERT(!RadioManagement_FreqIsInBand(&band, 14350001));
}

static void test_freq_in_zero_size_band(void)
{
    BandInfo band = { .tune = 0, .size = 0, .name = "Gen", .band_mode = 0, .rx_only = false };
    TEST_ASSERT(RadioManagement_FreqIsInBand(&band, 0));
    TEST_ASSERT(!RadioManagement_FreqIsInBand(&band, 1));
}

/* -------------------------------------------------------------------------
 * Tests: RadioManagement_GetRealFreqTranslationMode
 * ------------------------------------------------------------------------- */
static void test_translation_cw_tx_forces_off(void)
{
    /* CW + TX should always return FREQ_IQ_CONV_MODE_OFF regardless of iq_freq_mode */
    TEST_ASSERT_EQUAL(FREQ_IQ_CONV_MODE_OFF, RadioManagement_GetRealFreqTranslationMode(TRX_MODE_TX, DEMOD_CW, 1));
    TEST_ASSERT_EQUAL(FREQ_IQ_CONV_MODE_OFF, RadioManagement_GetRealFreqTranslationMode(TRX_MODE_TX, DEMOD_CW, 2));
    TEST_ASSERT_EQUAL(FREQ_IQ_CONV_MODE_OFF, RadioManagement_GetRealFreqTranslationMode(TRX_MODE_TX, DEMOD_CW, 3));
}

static void test_translation_non_cw_passthrough(void)
{
    /* Non-CW modes should pass through iq_freq_mode unchanged */
    TEST_ASSERT_EQUAL(0, RadioManagement_GetRealFreqTranslationMode(0, 0, 0));
    TEST_ASSERT_EQUAL(1, RadioManagement_GetRealFreqTranslationMode(0, 1, 1));
    TEST_ASSERT_EQUAL(2, RadioManagement_GetRealFreqTranslationMode(1, 0, 2));
    TEST_ASSERT_EQUAL(3, RadioManagement_GetRealFreqTranslationMode(1, 1, 3));
}

static void test_translation_cw_rx_passthrough(void)
{
    /* CW + RX should pass through iq_freq_mode unchanged */
    TEST_ASSERT_EQUAL(1, RadioManagement_GetRealFreqTranslationMode(0, DEMOD_CW, 1));
    TEST_ASSERT_EQUAL(2, RadioManagement_GetRealFreqTranslationMode(0, DEMOD_CW, 2));
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */
void test_radio_management_register(void)
{
    printf("--- radio_management ---\n");
    TEST_REGISTER(test_is_generic_band_size_zero);
    TEST_REGISTER(test_is_generic_band_size_nonzero);
    TEST_REGISTER(test_is_generic_band_large_size);
    TEST_REGISTER(test_freq_in_band_at_tune);
    TEST_REGISTER(test_freq_in_band_at_upper_edge);
    TEST_REGISTER(test_freq_in_band_middle);
    TEST_REGISTER(test_freq_below_band);
    TEST_REGISTER(test_freq_above_band);
    TEST_REGISTER(test_freq_in_zero_size_band);
    TEST_REGISTER(test_translation_cw_tx_forces_off);
    TEST_REGISTER(test_translation_non_cw_passthrough);
    TEST_REGISTER(test_translation_cw_rx_passthrough);
    printf("\n");
}
