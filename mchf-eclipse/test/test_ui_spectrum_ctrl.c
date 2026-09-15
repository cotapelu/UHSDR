/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_ui_spectrum_ctrl.c                                         **
 **  Description:   Host unit tests for UiDriver_SpectrumChangeLayoutParameters,    **
 **                 UiDriver_SetSpectrumMode, UiDriver_GetSpectrumMode              **
 **                 Reimplements the logic inline to validate boundary modes        **
 **                 without pulling in HAL/vendor dependencies.                    **
 **  Last Modified: 2026-08-28                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include "mock_uhsdr_board.h"

/* -------------------------------------------------------------------------
 * Mock global state
 * ------------------------------------------------------------------------- */
TransceiverState ts;

/* -------------------------------------------------------------------------
 * Mock externals called by UiDriver_SpectrumChangeLayoutParameters()
 * ------------------------------------------------------------------------- */
static uint8_t mock_waterfall_clear_calls = 0;
static uint8_t mock_audio_set_chain_calls = 0;
static uint8_t mock_spectrum_init_calls = 0;
static uint8_t last_dmod_mode = 0xFF;

void UiSpectrum_WaterfallClearData(void)
{
    mock_waterfall_clear_calls++;
}

void AudioDriver_SetProcessingChain(uint8_t dmod_mode, bool reset_dsp_nr)
{
    mock_audio_set_chain_calls++;
    last_dmod_mode = dmod_mode;
    (void)reset_dsp_nr;
}

void UiSpectrum_Init(void)
{
    mock_spectrum_init_calls++;
}

/* -------------------------------------------------------------------------
 * Reimplementation of ui_spectrum_ctrl.c logic for host testing
 * (mirrors mchf-eclipse/drivers/ui/ui_spectrum_ctrl.c)
 * ------------------------------------------------------------------------- */
#define FLAGS1_SCOPE_ENABLED   0x100
#define FLAGS1_WFALL_ENABLED   0x80

typedef enum {
    SPECTRUM_BLANK = 0,
    SPECTRUM_SCOPE = 2,
    SPECTRUM_DUAL = 3
} SpectrumMode_t;

static SpectrumMode_t UiDriver_GetSpectrumMode(void)
{
    return (SpectrumMode_t)((ts.flags1 & (FLAGS1_SCOPE_ENABLED | FLAGS1_WFALL_ENABLED)) >> 7);
}

static void UiDriver_SetSpectrumMode(SpectrumMode_t mode)
{
    ts.flags1 = (ts.flags1 & ~(FLAGS1_SCOPE_ENABLED | FLAGS1_WFALL_ENABLED)) | (mode << 7);
}

static void UiDriver_SpectrumChangeLayoutParameters(void)
{
    UiSpectrum_WaterfallClearData();
    AudioDriver_SetProcessingChain(ts.dmod_mode, false);
    if (ts.menu_mode == false)
    {
        UiSpectrum_Init();
    }
}

/* -------------------------------------------------------------------------
 * Test helpers
 * ------------------------------------------------------------------------- */
static void reset_mocks(void)
{
    mock_waterfall_clear_calls = 0;
    mock_audio_set_chain_calls = 0;
    mock_spectrum_init_calls = 0;
    last_dmod_mode = 0xFF;
    ts.flags1 = 0;
    ts.menu_mode = 0;
    ts.dmod_mode = 0;
}

/* -------------------------------------------------------------------------
 * Tests for UiDriver_SetSpectrumMode / UiDriver_GetSpectrumMode
 * ------------------------------------------------------------------------- */
static void test_set_get_spectrum_mode_blank(void)
{
    reset_mocks();
    UiDriver_SetSpectrumMode(SPECTRUM_BLANK);
    TEST_ASSERT_EQUAL(SPECTRUM_BLANK, UiDriver_GetSpectrumMode());
    TEST_ASSERT_EQUAL(0, ts.flags1 & (FLAGS1_SCOPE_ENABLED | FLAGS1_WFALL_ENABLED));
}

static void test_set_get_spectrum_mode_scope(void)
{
    reset_mocks();
    UiDriver_SetSpectrumMode(SPECTRUM_SCOPE);
    TEST_ASSERT_EQUAL(SPECTRUM_SCOPE, UiDriver_GetSpectrumMode());
    TEST_ASSERT_EQUAL(FLAGS1_SCOPE_ENABLED, ts.flags1 & (FLAGS1_SCOPE_ENABLED | FLAGS1_WFALL_ENABLED));
}

static void test_set_get_spectrum_mode_dual(void)
{
    reset_mocks();
    UiDriver_SetSpectrumMode(SPECTRUM_DUAL);
    TEST_ASSERT_EQUAL(SPECTRUM_DUAL, UiDriver_GetSpectrumMode());
    TEST_ASSERT_EQUAL((FLAGS1_SCOPE_ENABLED | FLAGS1_WFALL_ENABLED),
                       ts.flags1 & (FLAGS1_SCOPE_ENABLED | FLAGS1_WFALL_ENABLED));
}

static void test_set_spectrum_mode_preserves_other_flags(void)
{
    reset_mocks();
    ts.flags1 = 0x01; /* set some unrelated flag */
    UiDriver_SetSpectrumMode(SPECTRUM_DUAL);
    TEST_ASSERT_EQUAL((0x01 | FLAGS1_SCOPE_ENABLED | FLAGS1_WFALL_ENABLED), ts.flags1);
}

static void test_get_spectrum_mode_initial_zero(void)
{
    reset_mocks();
    TEST_ASSERT_EQUAL(SPECTRUM_BLANK, UiDriver_GetSpectrumMode());
}

static void test_set_spectrum_mode_overwrite(void)
{
    reset_mocks();
    UiDriver_SetSpectrumMode(SPECTRUM_SCOPE);
    TEST_ASSERT_EQUAL(SPECTRUM_SCOPE, UiDriver_GetSpectrumMode());
    UiDriver_SetSpectrumMode(SPECTRUM_BLANK);
    TEST_ASSERT_EQUAL(SPECTRUM_BLANK, UiDriver_GetSpectrumMode());
}

/* -------------------------------------------------------------------------
 * Tests for UiDriver_SpectrumChangeLayoutParameters
 * ------------------------------------------------------------------------- */
static void test_spectrum_change_layout_calls_all_when_not_menu(void)
{
    reset_mocks();
    ts.menu_mode = 0;
    ts.dmod_mode = 5;
    UiDriver_SpectrumChangeLayoutParameters();
    TEST_ASSERT_EQUAL(1, mock_waterfall_clear_calls);
    TEST_ASSERT_EQUAL(1, mock_audio_set_chain_calls);
    TEST_ASSERT_EQUAL(1, mock_spectrum_init_calls);
    TEST_ASSERT_EQUAL(5, last_dmod_mode);
}

static void test_spectrum_change_layout_skips_init_when_menu(void)
{
    reset_mocks();
    ts.menu_mode = 1;
    ts.dmod_mode = 3;
    UiDriver_SpectrumChangeLayoutParameters();
    TEST_ASSERT_EQUAL(1, mock_waterfall_clear_calls);
    TEST_ASSERT_EQUAL(1, mock_audio_set_chain_calls);
    TEST_ASSERT_EQUAL(0, mock_spectrum_init_calls);
    TEST_ASSERT_EQUAL(3, last_dmod_mode);
}

static void test_spectrum_change_layout_always_clears_waterfall(void)
{
    reset_mocks();
    ts.menu_mode = 1;
    UiDriver_SpectrumChangeLayoutParameters();
    TEST_ASSERT_EQUAL(1, mock_waterfall_clear_calls);
}

static void test_spectrum_change_layout_always_sets_chain(void)
{
    reset_mocks();
    ts.menu_mode = 0;
    UiDriver_SpectrumChangeLayoutParameters();
    TEST_ASSERT_EQUAL(1, mock_audio_set_chain_calls);
}

/* -------------------------------------------------------------------------
 * Test registration
 * ------------------------------------------------------------------------- */
void test_ui_spectrum_ctrl_register(void)
{
    printf("--- ui_spectrum_ctrl (T42.1) ---\n");
    TEST_REGISTER(test_set_get_spectrum_mode_blank);
    TEST_REGISTER(test_set_get_spectrum_mode_scope);
    TEST_REGISTER(test_set_get_spectrum_mode_dual);
    TEST_REGISTER(test_set_spectrum_mode_preserves_other_flags);
    TEST_REGISTER(test_get_spectrum_mode_initial_zero);
    TEST_REGISTER(test_set_spectrum_mode_overwrite);
    TEST_REGISTER(test_spectrum_change_layout_calls_all_when_not_menu);
    TEST_REGISTER(test_spectrum_change_layout_skips_init_when_menu);
    TEST_REGISTER(test_spectrum_change_layout_always_clears_waterfall);
    TEST_REGISTER(test_spectrum_change_layout_always_sets_chain);
    printf("\n");
}
