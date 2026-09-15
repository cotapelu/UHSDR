/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                     Host unit tests: TX sidetone usage detection                **
 **                                                                                 **
 **--------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_tx_sidetone.c                                              **
 **  Description:   Tests for RadioManagement_UsesTxSidetone()                      **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"

/* -------------------------------------------------------------------------
 * Reimplemented logic under test
 * ------------------------------------------------------------------------- */

#define DEMOD_USB   0
#define DEMOD_LSB   1
#define DEMOD_CW    2
#define DEMOD_AM    3
#define DEMOD_SAM   4
#define DEMOD_FM    5
#define DEMOD_DIGI  6

#define DigitalMode_None    0
#define DigitalMode_FreeDV  1
#define DigitalMode_RTTY    2
#define DigitalMode_BPSK    3
#define DigitalMode_Num_Modes 4

typedef struct
{
    uint8_t dmod_mode;
    uint8_t digital_mode;
    bool tune;
    bool iq_freq_mode;
} TxSidetoneState;

static bool RadioManagement_UsesTxSidetone_under_test(const TxSidetoneState *ts)
{
    return ts->dmod_mode == DEMOD_CW ||
           (ts->dmod_mode == DEMOD_DIGI && ts->digital_mode == DigitalMode_RTTY) ||
           (ts->dmod_mode == DEMOD_DIGI && ts->digital_mode == DigitalMode_BPSK) ||
           (ts->tune && !ts->iq_freq_mode);
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

static void test_cw_uses_sidetone(void)
{
    TxSidetoneState ts;
    memset(&ts, 0, sizeof(ts));
    ts.dmod_mode = DEMOD_CW;

    bool result = RadioManagement_UsesTxSidetone_under_test(&ts);

    TEST_ASSERT_EQUAL(1, result);
}

static void test_usb_no_sidetone(void)
{
    TxSidetoneState ts;
    memset(&ts, 0, sizeof(ts));
    ts.dmod_mode = DEMOD_USB;

    bool result = RadioManagement_UsesTxSidetone_under_test(&ts);

    TEST_ASSERT_EQUAL(0, result);
}

static void test_lsb_no_sidetone(void)
{
    TxSidetoneState ts;
    memset(&ts, 0, sizeof(ts));
    ts.dmod_mode = DEMOD_LSB;

    bool result = RadioManagement_UsesTxSidetone_under_test(&ts);

    TEST_ASSERT_EQUAL(0, result);
}

static void test_fm_no_sidetone(void)
{
    TxSidetoneState ts;
    memset(&ts, 0, sizeof(ts));
    ts.dmod_mode = DEMOD_FM;

    bool result = RadioManagement_UsesTxSidetone_under_test(&ts);

    TEST_ASSERT_EQUAL(0, result);
}

static void test_am_no_sidetone(void)
{
    TxSidetoneState ts;
    memset(&ts, 0, sizeof(ts));
    ts.dmod_mode = DEMOD_AM;

    bool result = RadioManagement_UsesTxSidetone_under_test(&ts);

    TEST_ASSERT_EQUAL(0, result);
}

static void test_sam_no_sidetone(void)
{
    TxSidetoneState ts;
    memset(&ts, 0, sizeof(ts));
    ts.dmod_mode = DEMOD_SAM;

    bool result = RadioManagement_UsesTxSidetone_under_test(&ts);

    TEST_ASSERT_EQUAL(0, result);
}

static void test_digi_rtty_uses_sidetone(void)
{
    TxSidetoneState ts;
    memset(&ts, 0, sizeof(ts));
    ts.dmod_mode = DEMOD_DIGI;
    ts.digital_mode = DigitalMode_RTTY;

    bool result = RadioManagement_UsesTxSidetone_under_test(&ts);

    TEST_ASSERT_EQUAL(1, result);
}

static void test_digi_bpsk_uses_sidetone(void)
{
    TxSidetoneState ts;
    memset(&ts, 0, sizeof(ts));
    ts.dmod_mode = DEMOD_DIGI;
    ts.digital_mode = DigitalMode_BPSK;

    bool result = RadioManagement_UsesTxSidetone_under_test(&ts);

    TEST_ASSERT_EQUAL(1, result);
}

static void test_digi_freedv_no_sidetone(void)
{
    TxSidetoneState ts;
    memset(&ts, 0, sizeof(ts));
    ts.dmod_mode = DEMOD_DIGI;
    ts.digital_mode = DigitalMode_FreeDV;

    bool result = RadioManagement_UsesTxSidetone_under_test(&ts);

    TEST_ASSERT_EQUAL(0, result);
}

static void test_digi_none_no_sidetone(void)
{
    TxSidetoneState ts;
    memset(&ts, 0, sizeof(ts));
    ts.dmod_mode = DEMOD_DIGI;
    ts.digital_mode = DigitalMode_None;

    bool result = RadioManagement_UsesTxSidetone_under_test(&ts);

    TEST_ASSERT_EQUAL(0, result);
}

static void test_tune_mode_uses_sidetone_any_mode(void)
{
    TxSidetoneState ts;
    memset(&ts, 0, sizeof(ts));
    ts.dmod_mode = DEMOD_USB;
    ts.tune = true;
    ts.iq_freq_mode = false;

    bool result = RadioManagement_UsesTxSidetone_under_test(&ts);

    TEST_ASSERT_EQUAL(1, result);
}

static void test_tune_mode_iq_freq_mode_no_sidetone(void)
{
    TxSidetoneState ts;
    memset(&ts, 0, sizeof(ts));
    ts.dmod_mode = DEMOD_USB;
    ts.tune = true;
    ts.iq_freq_mode = true;

    bool result = RadioManagement_UsesTxSidetone_under_test(&ts);

    TEST_ASSERT_EQUAL(0, result);
}

static void test_not_tune_no_sidetone_any_mode(void)
{
    TxSidetoneState ts;
    memset(&ts, 0, sizeof(ts));
    ts.dmod_mode = DEMOD_FM;
    ts.tune = false;
    ts.iq_freq_mode = true;

    bool result = RadioManagement_UsesTxSidetone_under_test(&ts);

    TEST_ASSERT_EQUAL(0, result);
}

static void test_cw_with_tune_and_iq_freq_mode(void)
{
    TxSidetoneState ts;
    memset(&ts, 0, sizeof(ts));
    ts.dmod_mode = DEMOD_CW;
    ts.tune = true;
    ts.iq_freq_mode = true;

    bool result = RadioManagement_UsesTxSidetone_under_test(&ts);

    TEST_ASSERT_EQUAL(1, result); /* CW always uses sidetone */
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */

void test_tx_sidetone_register(void)
{
    printf("--- TX sidetone usage (RadioManagement_UsesTxSidetone) ---\n");
    TEST_REGISTER(test_cw_uses_sidetone);
    TEST_REGISTER(test_usb_no_sidetone);
    TEST_REGISTER(test_lsb_no_sidetone);
    TEST_REGISTER(test_fm_no_sidetone);
    TEST_REGISTER(test_am_no_sidetone);
    TEST_REGISTER(test_sam_no_sidetone);
    TEST_REGISTER(test_digi_rtty_uses_sidetone);
    TEST_REGISTER(test_digi_bpsk_uses_sidetone);
    TEST_REGISTER(test_digi_freedv_no_sidetone);
    TEST_REGISTER(test_digi_none_no_sidetone);
    TEST_REGISTER(test_tune_mode_uses_sidetone_any_mode);
    TEST_REGISTER(test_tune_mode_iq_freq_mode_no_sidetone);
    TEST_REGISTER(test_not_tune_no_sidetone_any_mode);
    TEST_REGISTER(test_cw_with_tune_and_iq_freq_mode);
    printf("\n");
}
