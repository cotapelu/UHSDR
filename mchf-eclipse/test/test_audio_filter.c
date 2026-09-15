/**
 * @file test_audio_filter.c
 * @brief Unit tests for audio_filter.c pure logic: AudioFilter_GetFilterModeFromDemodMode()
 *
 * Reimplements AudioFilter_GetFilterModeFromDemodMode() logic inline to avoid
 * hardware-dependent headers.
 */

#include "test.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Reimplementation of AudioFilter_GetFilterModeFromDemodMode() for host testing
 * ------------------------------------------------------------------------- */

/* Demod mode constants (from uhsdr_board.h) */
#define DEMOD_USB      0
#define DEMOD_LSB      1
#define DEMOD_CW       2
#define DEMOD_AM       3
#define DEMOD_SAM      4
#define DEMOD_FM       5
#define DEMOD_DIGI     6

/* Filter mode constants (from audio_filter.h) */
#define FILTER_MODE_CW     0
#define FILTER_MODE_SSB    1
#define FILTER_MODE_AM     2
#define FILTER_MODE_FM     3
#define FILTER_MODE_SAM    4
#define FILTER_MODE_MAX    5

static uint16_t host_AudioFilter_GetFilterModeFromDemodMode(const uint8_t dmod_mode)
{
    uint16_t filter_mode;
    switch (dmod_mode)
    {
    case DEMOD_AM:
        filter_mode = FILTER_MODE_AM;
        break;
    case DEMOD_FM:
        filter_mode = FILTER_MODE_FM;
        break;
    case DEMOD_CW:
        filter_mode = FILTER_MODE_CW;
        break;
    case DEMOD_SAM:
        /* from 2017 on, the new SAM mode uses the AM filters! */
        filter_mode = FILTER_MODE_AM;
        break;
    default:
        filter_mode = FILTER_MODE_SSB;
    }
    return filter_mode;
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */
static void test_filter_mode_from_demod_am(void)
{
    TEST_ASSERT_EQUAL(FILTER_MODE_AM, host_AudioFilter_GetFilterModeFromDemodMode(DEMOD_AM));
}

static void test_filter_mode_from_demod_fm(void)
{
    TEST_ASSERT_EQUAL(FILTER_MODE_FM, host_AudioFilter_GetFilterModeFromDemodMode(DEMOD_FM));
}

static void test_filter_mode_from_demod_cw(void)
{
    TEST_ASSERT_EQUAL(FILTER_MODE_CW, host_AudioFilter_GetFilterModeFromDemodMode(DEMOD_CW));
}

static void test_filter_mode_from_demod_sam(void)
{
    /* SAM maps to AM filter mode (per comment: "from 2017 on, the new SAM mode uses the AM filters!") */
    TEST_ASSERT_EQUAL(FILTER_MODE_AM, host_AudioFilter_GetFilterModeFromDemodMode(DEMOD_SAM));
}

static void test_filter_mode_from_demod_usb(void)
{
    TEST_ASSERT_EQUAL(FILTER_MODE_SSB, host_AudioFilter_GetFilterModeFromDemodMode(DEMOD_USB));
}

static void test_filter_mode_from_demod_lsb(void)
{
    TEST_ASSERT_EQUAL(FILTER_MODE_SSB, host_AudioFilter_GetFilterModeFromDemodMode(DEMOD_LSB));
}

static void test_filter_mode_from_demod_digi(void)
{
    TEST_ASSERT_EQUAL(FILTER_MODE_SSB, host_AudioFilter_GetFilterModeFromDemodMode(DEMOD_DIGI));
}

static void test_filter_mode_from_demod_unknown(void)
{
    /* Unknown demod mode falls through to SSB */
    TEST_ASSERT_EQUAL(FILTER_MODE_SSB, host_AudioFilter_GetFilterModeFromDemodMode(255));
}

static void test_filter_mode_from_demod_zero(void)
{
    /* DEMOD_USB is 0, should map to SSB */
    TEST_ASSERT_EQUAL(FILTER_MODE_SSB, host_AudioFilter_GetFilterModeFromDemodMode(0));
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */
void test_audio_filter_register(void)
{
    printf("--- audio_filter mode mapping ---\n");
    TEST_REGISTER(test_filter_mode_from_demod_am);
    TEST_REGISTER(test_filter_mode_from_demod_fm);
    TEST_REGISTER(test_filter_mode_from_demod_cw);
    TEST_REGISTER(test_filter_mode_from_demod_sam);
    TEST_REGISTER(test_filter_mode_from_demod_usb);
    TEST_REGISTER(test_filter_mode_from_demod_lsb);
    TEST_REGISTER(test_filter_mode_from_demod_digi);
    TEST_REGISTER(test_filter_mode_from_demod_unknown);
    TEST_REGISTER(test_filter_mode_from_demod_zero);
    printf("\n");
}
