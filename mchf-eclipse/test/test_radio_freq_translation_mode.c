/**
 * @file test_radio_freq_translation_mode.c
 * @brief Unit tests for radio_management.c RadioManagement_GetRealFreqTranslationMode()
 *
 * Reimplements the frequency translation mode logic inline to avoid
 * hardware-dependent headers and global state.
 */

#include "test.h"
#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Replicated constants from hardware/uhsdr_board.h and drivers/audio/audio_driver.h
 * ------------------------------------------------------------------------- */

#define TRX_MODE_TX              1
#define DEMOD_CW                 2
#define FREQ_IQ_CONV_MODE_OFF    0
#define FREQ_IQ_CONV_P6KHZ       1
#define FREQ_IQ_CONV_M6KHZ       2
#define FREQ_IQ_CONV_P12KHZ      3
#define FREQ_IQ_CONV_M12KHZ      4

/* -------------------------------------------------------------------------
 * Reimplementation of RadioManagement_GetRealFreqTranslationMode()
 * ------------------------------------------------------------------------- */

static uint32_t host_get_real_freq_translation_mode(uint32_t txrx_mode, uint32_t dmod_mode, uint32_t iq_freq_mode)
{
    uint32_t retval = iq_freq_mode;
    if (dmod_mode == DEMOD_CW && txrx_mode == TRX_MODE_TX)
    {
        retval = FREQ_IQ_CONV_MODE_OFF;
    }
    return retval;
}

/* =========================================================================
 * Tests: RadioManagement_GetRealFreqTranslationMode
 * ========================================================================= */

static void test_cw_tx_forces_off(void)
{
    /* CW in TX mode should always return OFF regardless of configured mode */
    TEST_ASSERT_EQUAL(FREQ_IQ_CONV_MODE_OFF, host_get_real_freq_translation_mode(TRX_MODE_TX, DEMOD_CW, FREQ_IQ_CONV_P6KHZ));
    TEST_ASSERT_EQUAL(FREQ_IQ_CONV_MODE_OFF, host_get_real_freq_translation_mode(TRX_MODE_TX, DEMOD_CW, FREQ_IQ_CONV_M6KHZ));
    TEST_ASSERT_EQUAL(FREQ_IQ_CONV_MODE_OFF, host_get_real_freq_translation_mode(TRX_MODE_TX, DEMOD_CW, FREQ_IQ_CONV_P12KHZ));
    TEST_ASSERT_EQUAL(FREQ_IQ_CONV_MODE_OFF, host_get_real_freq_translation_mode(TRX_MODE_TX, DEMOD_CW, FREQ_IQ_CONV_M12KHZ));
    TEST_ASSERT_EQUAL(FREQ_IQ_CONV_MODE_OFF, host_get_real_freq_translation_mode(TRX_MODE_TX, DEMOD_CW, FREQ_IQ_CONV_MODE_OFF));
}

static void test_cw_rx_preserves_mode(void)
{
    /* CW in RX mode should preserve the configured mode */
    TEST_ASSERT_EQUAL(FREQ_IQ_CONV_P6KHZ, host_get_real_freq_translation_mode(0, DEMOD_CW, FREQ_IQ_CONV_P6KHZ));
    TEST_ASSERT_EQUAL(FREQ_IQ_CONV_M12KHZ, host_get_real_freq_translation_mode(0, DEMOD_CW, FREQ_IQ_CONV_M12KHZ));
}

static void test_non_cw_tx_preserves_mode(void)
{
    /* Non-CW modes in TX should preserve the configured mode */
    TEST_ASSERT_EQUAL(FREQ_IQ_CONV_P6KHZ, host_get_real_freq_translation_mode(TRX_MODE_TX, 0, FREQ_IQ_CONV_P6KHZ));
    TEST_ASSERT_EQUAL(FREQ_IQ_CONV_M12KHZ, host_get_real_freq_translation_mode(TRX_MODE_TX, 3, FREQ_IQ_CONV_M12KHZ));
}

static void test_non_cw_rx_preserves_mode(void)
{
    /* Non-CW modes in RX should preserve the configured mode */
    TEST_ASSERT_EQUAL(FREQ_IQ_CONV_P12KHZ, host_get_real_freq_translation_mode(0, 1, FREQ_IQ_CONV_P12KHZ));
    TEST_ASSERT_EQUAL(FREQ_IQ_CONV_M6KHZ, host_get_real_freq_translation_mode(0, 3, FREQ_IQ_CONV_M6KHZ));
}

static void test_cw_tx_with_off_mode(void)
{
    /* Even if mode is already OFF, CW TX should return OFF */
    TEST_ASSERT_EQUAL(FREQ_IQ_CONV_MODE_OFF, host_get_real_freq_translation_mode(TRX_MODE_TX, DEMOD_CW, FREQ_IQ_CONV_MODE_OFF));
}

static void test_all_modes_preserved_in_rx(void)
{
    /* All modes should be preserved in RX */
    for (uint32_t mode = 0; mode <= 4; mode++)
    {
        TEST_ASSERT_EQUAL(mode, host_get_real_freq_translation_mode(0, DEMOD_CW, mode));
    }
}

static void test_cw_tx_overrides_all_modes(void)
{
    /* All modes should be overridden to OFF in CW TX */
    for (uint32_t mode = 0; mode <= 4; mode++)
    {
        TEST_ASSERT_EQUAL(FREQ_IQ_CONV_MODE_OFF, host_get_real_freq_translation_mode(TRX_MODE_TX, DEMOD_CW, mode));
    }
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_radio_freq_translation_mode_register(void)
{
    printf("--- radio_freq_translation_mode ---\n");

    TEST_REGISTER(test_cw_tx_forces_off);
    TEST_REGISTER(test_cw_rx_preserves_mode);
    TEST_REGISTER(test_non_cw_tx_preserves_mode);
    TEST_REGISTER(test_non_cw_rx_preserves_mode);
    TEST_REGISTER(test_cw_tx_with_off_mode);
    TEST_REGISTER(test_all_modes_preserved_in_rx);
    TEST_REGISTER(test_cw_tx_overrides_all_modes);

    printf("\n");
}
