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

static void test_calc_bandstop(void)
{
    float32_t coeffs[5];

    AudioDriver_CalcBandstop(coeffs, 1000.0f, 48000.0f);

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

/* Test runner entry point */
int test_main(void)
{
    printf("UHSDR Unit Tests: audio_driver_filters\n");
    printf("========================================\n\n");

    TEST_REGISTER(test_set_biquad_coeffs);
    TEST_REGISTER(test_scale_biquad_coeffs);
    TEST_REGISTER(test_calc_bandstop);
    TEST_REGISTER(test_calc_bandpass);
    TEST_REGISTER(test_calc_shelf_filters);

    printf("\nResults: %d passed, %d failed\n", test_pass_count, test_fail_count);

    return test_fail_count ? 1 : 0;
}
