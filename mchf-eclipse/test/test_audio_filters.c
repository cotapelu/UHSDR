/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_audio_filters.c                                            **
 **  Description:   Unit tests for audio_driver_filters module                      **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include <math.h>
#include "audio_driver_filters_standalone.h"

/* Test: SetBiquadCoeffs copies all 5 coefficients correctly */
static void test_set_biquad_coeffs(void)
{
    float32_t src[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    float32_t dst[5] = {0.0f};

    AudioDriver_SetBiquadCoeffs(dst, src);

    TEST_ASSERT_FLOAT_EQ(1.0f, dst[0], 1e-6f);
    TEST_ASSERT_FLOAT_EQ(2.0f, dst[1], 1e-6f);
    TEST_ASSERT_FLOAT_EQ(3.0f, dst[2], 1e-6f);
    TEST_ASSERT_FLOAT_EQ(4.0f, dst[3], 1e-6f);
    TEST_ASSERT_FLOAT_EQ(5.0f, dst[4], 1e-6f);
}

/* Test: ScaleBiquadCoeffs divides A1/A2 by scalingA and B0/B1/B2 by scalingB */
static void test_scale_biquad_coeffs(void)
{
    float32_t coeffs[5] = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f};

    AudioDriver_ScaleBiquadCoeffs(coeffs, 2.0f, 5.0f);

    TEST_ASSERT_FLOAT_EQ(20.0f, coeffs[3], 1e-6f);  /* A1 = 40/2 */
    TEST_ASSERT_FLOAT_EQ(25.0f, coeffs[4], 1e-6f);  /* A2 = 50/2 */
    TEST_ASSERT_FLOAT_EQ( 2.0f, coeffs[0], 1e-6f);  /* B0 = 10/5 */
    TEST_ASSERT_FLOAT_EQ( 4.0f, coeffs[1], 1e-6f);  /* B1 = 20/5 */
    TEST_ASSERT_FLOAT_EQ( 6.0f, coeffs[2], 1e-6f);  /* B2 = 30/5 */
}

/* Test: CalcBandstop produces stable coefficients */
static void test_calc_bandstop(void)
{
    float32_t coeffs[5];

    AudioDriver_CalcBandstop(coeffs, 1000.0f, 48000.0f);

    for (int i = 0; i < 5; i++)
    {
        TEST_ASSERT(!isnan(coeffs[i]));
        TEST_ASSERT(!isinf(coeffs[i]));
    }
}

/* Test: CalcBandpass produces stable coefficients (no NaN/Inf) */
static void test_calc_bandpass(void)
{
    float32_t coeffs[5];

    AudioDriver_CalcBandpass(coeffs, 2000.0f, 48000.0f);

    TEST_ASSERT(!isnan(coeffs[0]));
    TEST_ASSERT(!isnan(coeffs[1]));
    TEST_ASSERT(!isnan(coeffs[2]));
    TEST_ASSERT(!isnan(coeffs[3]));
    TEST_ASSERT(!isnan(coeffs[4]));
    TEST_ASSERT(!isinf(coeffs[0]));
    TEST_ASSERT(!isinf(coeffs[1]));
    TEST_ASSERT(!isinf(coeffs[2]));
    TEST_ASSERT(!isinf(coeffs[3]));
    TEST_ASSERT(!isinf(coeffs[4]));
}

/* Test: CalcHighShelf and CalcLowShelf produce finite coefficients */
static void test_calc_shelf_filters(void)
{
    float32_t high[5], low[5];

    AudioDriver_CalcHighShelf(high, 3000.0f, 1.0f, 3.0f, 48000.0f);
    AudioDriver_CalcLowShelf(low, 500.0f, 1.0f, -3.0f, 48000.0f);

    for (int i = 0; i < 5; i++)
    {
        TEST_ASSERT(!isnan(high[i]));
        TEST_ASSERT(!isnan(low[i]));
        TEST_ASSERT(!isinf(high[i]));
        TEST_ASSERT(!isinf(low[i]));
    }
}

/* Test: SetBiquadCoeffsAllInstances copies coefficients into biquad array */
static void test_set_biquad_coeffs_all_instances(void)
{
    typedef struct {
        uint32_t numStages;
        float32_t *pCoeffs;
        float32_t *pState;
    } test_biquad_t;

    test_biquad_t instance;
    float32_t coeffs[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    float32_t pCoeffs[5] = {0.0f};
    float32_t pState[10] = {0.0f};

    instance.pCoeffs = pCoeffs;
    instance.pState = pState;

    AudioDriver_SetBiquadCoeffsAllInstances(&instance, 0, coeffs);

    TEST_ASSERT_FLOAT_EQ(1.0f, pCoeffs[0], 1e-6f);
    TEST_ASSERT_FLOAT_EQ(2.0f, pCoeffs[1], 1e-6f);
    TEST_ASSERT_FLOAT_EQ(3.0f, pCoeffs[2], 1e-6f);
    TEST_ASSERT_FLOAT_EQ(4.0f, pCoeffs[3], 1e-6f);
    TEST_ASSERT_FLOAT_EQ(5.0f, pCoeffs[4], 1e-6f);
}

/* Test: Bandstop coefficients remain stable across sample rates */
static void test_calc_bandstop_multiple_sample_rates(void)
{
    float32_t coeffs_48k[5];
    float32_t coeffs_96k[5];

    AudioDriver_CalcBandstop(coeffs_48k, 1000.0f, 48000.0f);
    AudioDriver_CalcBandstop(coeffs_96k, 1000.0f, 96000.0f);

    for (int i = 0; i < 5; i++)
    {
        TEST_ASSERT(!isnan(coeffs_48k[i]));
        TEST_ASSERT(!isnan(coeffs_96k[i]));
        TEST_ASSERT(!isinf(coeffs_48k[i]));
        TEST_ASSERT(!isinf(coeffs_96k[i]));
    }
}

/* Test: Bandpass coefficients remain stable across sample rates */
static void test_calc_bandpass_multiple_sample_rates(void)
{
    float32_t coeffs_48k[5];
    float32_t coeffs_96k[5];

    AudioDriver_CalcBandpass(coeffs_48k, 2000.0f, 48000.0f);
    AudioDriver_CalcBandpass(coeffs_96k, 2000.0f, 96000.0f);

    for (int i = 0; i < 5; i++)
    {
        TEST_ASSERT(!isnan(coeffs_48k[i]));
        TEST_ASSERT(!isnan(coeffs_96k[i]));
        TEST_ASSERT(!isinf(coeffs_48k[i]));
        TEST_ASSERT(!isinf(coeffs_96k[i]));
    }
}

/* Test: Bandstop at very low frequency produces stable coefficients */
static void test_calc_bandstop_low_freq(void)
{
    float32_t coeffs[5];

    AudioDriver_CalcBandstop(coeffs, 10.0f, 48000.0f);

    for (int i = 0; i < 5; i++)
    {
        TEST_ASSERT(!isnan(coeffs[i]));
        TEST_ASSERT(!isinf(coeffs[i]));
    }
}

/* Test: Bandstop near Nyquist produces stable coefficients */
static void test_calc_bandstop_near_nyquist(void)
{
    float32_t coeffs[5];

    AudioDriver_CalcBandstop(coeffs, 20000.0f, 48000.0f);

    for (int i = 0; i < 5; i++)
    {
        TEST_ASSERT(!isnan(coeffs[i]));
        TEST_ASSERT(!isinf(coeffs[i]));
    }
}

/* Test: Bandpass at very low frequency produces stable coefficients */
static void test_calc_bandpass_low_freq(void)
{
    float32_t coeffs[5];

    AudioDriver_CalcBandpass(coeffs, 10.0f, 48000.0f);

    for (int i = 0; i < 5; i++)
    {
        TEST_ASSERT(!isnan(coeffs[i]));
        TEST_ASSERT(!isinf(coeffs[i]));
    }
}

/* Test: Bandpass at high frequency (below Nyquist) produces stable coefficients */
static void test_calc_bandpass_high_freq(void)
{
    float32_t coeffs[5];

    AudioDriver_CalcBandpass(coeffs, 20000.0f, 48000.0f);

    for (int i = 0; i < 5; i++)
    {
        TEST_ASSERT(!isnan(coeffs[i]));
        TEST_ASSERT(!isinf(coeffs[i]));
    }
}

/* Test: High shelf with extreme positive gain produces stable coefficients */
static void test_calc_high_shelf_extreme_gain(void)
{
    float32_t coeffs[5];

    AudioDriver_CalcHighShelf(coeffs, 1000.0f, 1.0f, 10.0f, 48000.0f);

    for (int i = 0; i < 5; i++)
    {
        TEST_ASSERT(!isnan(coeffs[i]));
        TEST_ASSERT(!isinf(coeffs[i]));
    }
}

/* Test: Low shelf with extreme negative gain produces stable coefficients */
static void test_calc_low_shelf_extreme_gain(void)
{
    float32_t coeffs[5];

    AudioDriver_CalcLowShelf(coeffs, 1000.0f, 1.0f, -10.0f, 48000.0f);

    for (int i = 0; i < 5; i++)
    {
        TEST_ASSERT(!isnan(coeffs[i]));
        TEST_ASSERT(!isinf(coeffs[i]));
    }
}

/* Test runner entry point */
/* Tests are registered in test.c to avoid multiple test_main definitions */
