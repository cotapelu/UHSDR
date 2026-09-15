/**
 * @file test_audio_biquad_coeffs.c
 * @brief Unit tests for audio_driver_filters.c biquad coefficient calculations:
 *        AudioDriver_CalcBandstop(), AudioDriver_CalcBandpass(),
 *        AudioDriver_CalcHighShelf(), AudioDriver_CalcLowShelf()
 *
 * Reimplements the coefficient calculation logic inline to avoid
 * hardware-dependent headers and global state.
 */

#include "test.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

typedef float float32_t;

#define B0 0
#define B1 1
#define B2 2
#define A1 3
#define A2 4
#define PI 3.14159265358979323846

/* -------------------------------------------------------------------------
 * Reimplementation of AudioDriver_ScaleBiquadCoeffs()
 * ------------------------------------------------------------------------- */

static void host_scale_biquad_coeffs(float32_t coeffs[5], const float32_t scalingA, const float32_t scalingB)
{
    coeffs[A1] = coeffs[A1] / scalingA;
    coeffs[A2] = coeffs[A2] / scalingA;
    coeffs[B0] = coeffs[B0] / scalingB;
    coeffs[B1] = coeffs[B1] / scalingB;
    coeffs[B2] = coeffs[B2] / scalingB;
}

/* -------------------------------------------------------------------------
 * Reimplementation of AudioDriver_CalcBandstop()
 * ------------------------------------------------------------------------- */

static void host_calc_bandstop(float32_t coeffs[5], float32_t f0, float32_t FS)
{
    float32_t Q = 10.0f;
    float32_t w0 = 2.0f * PI * f0 / FS;
    float32_t alpha = sinf(w0) / (2.0f * Q);

    coeffs[B0] = 1.0f;
    coeffs[B1] = -2.0f * cosf(w0);
    coeffs[B2] = 1.0f;
    float32_t scaling = 1.0f + alpha;
    coeffs[A1] = 2.0f * cosf(w0);
    coeffs[A2] = alpha - 1.0f;

    host_scale_biquad_coeffs(coeffs, scaling, scaling);
}

/* -------------------------------------------------------------------------
 * Reimplementation of AudioDriver_CalcBandpass()
 * ------------------------------------------------------------------------- */

static void host_calc_bandpass(float32_t coeffs[5], float32_t f0, float32_t FS)
{
    float32_t Q = 4.0f;
    float32_t BW = 0.03f;
    float32_t w0 = 2.0f * PI * f0 / FS;
    float32_t alpha = sinf(w0) * sinhf(log(2.0) / 2.0 * BW * w0 / sinf(w0));

    coeffs[B0] = Q * alpha;
    coeffs[B1] = 0.0f;
    coeffs[B2] = -Q * alpha;
    float32_t scaling = 1.0f + alpha;
    coeffs[A1] = 2.0f * cosf(w0);
    coeffs[A2] = alpha - 1.0f;

    host_scale_biquad_coeffs(coeffs, scaling, scaling);
}

/* -------------------------------------------------------------------------
 * Reimplementation of AudioDriver_CalcHighShelf()
 * ------------------------------------------------------------------------- */

static void host_calc_high_shelf(float32_t coeffs[5], float32_t f0, float32_t S, float32_t gain, float32_t FS)
{
    float32_t w0 = 2.0f * PI * f0 / FS;
    float32_t A = powf(10.0f, gain / 40.0f);
    float32_t alpha = sinf(w0) / 2.0f * sqrtf((A + 1.0f / A) * (1.0f / S - 1.0f) + 2.0f);
    float32_t cosw0 = cosf(w0);
    float32_t twoAa = 2.0f * sqrtf(A) * alpha;

    coeffs[B0] = A * ((A + 1.0f) + (A - 1.0f) * cosw0 + twoAa);
    coeffs[B1] = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosw0);
    coeffs[B2] = A * ((A + 1.0f) + (A - 1.0f) * cosw0 - twoAa);
    float32_t scaling = (A + 1.0f) - (A - 1.0f) * cosw0 + twoAa;
    coeffs[A1] = -2.0f * ((A - 1.0f) - (A + 1.0f) * cosw0);
    coeffs[A2] = twoAa - (A + 1.0f) + (A - 1.0f) * cosw0;

    float32_t DCgain = 1.0f * scaling;
    host_scale_biquad_coeffs(coeffs, scaling, DCgain);
}

/* -------------------------------------------------------------------------
 * Reimplementation of AudioDriver_CalcLowShelf()
 * ------------------------------------------------------------------------- */

static void host_calc_low_shelf(float32_t coeffs[5], float32_t f0, float32_t S, float32_t gain, float32_t FS)
{
    float32_t w0 = 2.0f * PI * f0 / FS;
    float32_t A = powf(10.0f, gain / 40.0f);

    float32_t alpha = sinf(w0) / 2.0f * sqrtf((A + 1.0f / A) * (1.0f / S - 1.0f) + 2.0f);
    float32_t cosw0 = cosf(w0);
    float32_t twoAa = 2.0f * sqrtf(A) * alpha;

    coeffs[B0] = A * ((A + 1.0f) - (A - 1.0f) * cosw0 + twoAa);
    coeffs[B1] = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosw0);
    coeffs[B2] = A * ((A + 1.0f) - (A - 1.0f) * cosw0 - twoAa);
    float32_t scaling = (A + 1.0f) + (A - 1.0f) * cosw0 + twoAa;
    coeffs[A1] = 2.0f * ((A - 1.0f) + (A + 1.0f) * cosw0);
    coeffs[A2] = twoAa - (A + 1.0f) - (A - 1.0f) * cosw0;

    float32_t DCgain = 1.0f * scaling;
    host_scale_biquad_coeffs(coeffs, scaling, DCgain);
}

/* =========================================================================
 * Helper
 * ========================================================================= */


/* =========================================================================
 * Helper: compute magnitude of biquad frequency response at given w0
 * ========================================================================= */

static float32_t host_biquad_magnitude(const float32_t coeffs[5], float32_t w0)
{
    /* Numerator: b0 + b1*e^-jw + b2*e^-2jw */
    float32_t num_re = coeffs[B0] + coeffs[B1] * cosf(w0) + coeffs[B2] * cosf(2.0f * w0);
    float32_t num_im = -(coeffs[B1] * sinf(w0) + coeffs[B2] * sinf(2.0f * w0));

    /* Denominator: 1 + a1*e^-jw + a2*e^-2jw, where a1/a2 are stored negated */
    float32_t den_re = 1.0f - coeffs[A1] * cosf(w0) - coeffs[A2] * cosf(2.0f * w0);
    float32_t den_im = coeffs[A1] * sinf(w0) + coeffs[A2] * sinf(2.0f * w0);

    float32_t num_mag2 = num_re * num_re + num_im * num_im;
    float32_t den_mag2 = den_re * den_re + den_im * den_im;

    if (den_mag2 < 1e-20f)
    {
        return 0.0f;
    }
    return sqrtf(num_mag2 / den_mag2);
}

/* =========================================================================
 * Tests: AudioDriver_CalcBandstop
 * ========================================================================= */

static void test_bandstop_center_frequency(void)
{
    float32_t coeffs[5];
    host_calc_bandstop(coeffs, 1000.0f, 48000.0f);

    /* At center frequency, the notch should attenuate */
    float32_t w0 = 2.0f * PI * 1000.0f / 48000.0f;
    float32_t mag = host_biquad_magnitude(coeffs, w0);
    TEST_ASSERT(mag < 0.1f);
}

static void test_bandstop_dc_gain(void)
{
    /* DC gain of bandstop should be ~1 (passes DC) */
    float32_t coeffs[5];
    host_calc_bandstop(coeffs, 1000.0f, 48000.0f);

    float32_t dc_gain = (coeffs[B0] + coeffs[B1] + coeffs[B2]) / (1.0f - coeffs[A1] - coeffs[A2]);
    TEST_ASSERT_FLOAT_EQ(1.0f, dc_gain, 0.01f);
}

static void test_bandstop_nyquist_gain(void)
{
    /* Nyquist gain of bandstop should be ~1 */
    float32_t coeffs[5];
    host_calc_bandstop(coeffs, 1000.0f, 48000.0f);

    float32_t w0 = PI;
    float32_t nyquist_gain = host_biquad_magnitude(coeffs, w0);
    TEST_ASSERT_FLOAT_EQ(1.0f, nyquist_gain, 0.01f);
}

static void test_bandstop_higher_freq(void)
{
    /* Bandstop at higher frequency should still be a valid filter */
    float32_t coeffs[5];
    host_calc_bandstop(coeffs, 5000.0f, 48000.0f);

    TEST_ASSERT(fabsf(coeffs[B0]) > 0.0f);
    TEST_ASSERT(fabsf(coeffs[A1]) > 0.0f);
}

static void test_bandstop_lower_freq(void)
{
    /* Bandstop at lower frequency should still be a valid filter */
    float32_t coeffs[5];
    host_calc_bandstop(coeffs, 100.0f, 48000.0f);

    TEST_ASSERT(fabsf(coeffs[B0]) > 0.0f);
}

/* =========================================================================
 * Tests: AudioDriver_CalcBandpass
 * ========================================================================= */

static void test_bandpass_center_frequency(void)
{
    /* At center frequency, bandpass should have peak gain */
    float32_t coeffs[5];
    host_calc_bandpass(coeffs, 1000.0f, 48000.0f);

    float32_t w0 = 2.0f * PI * 1000.0f / 48000.0f;
    float32_t mag = host_biquad_magnitude(coeffs, w0);
    TEST_ASSERT(mag > 0.5f);
}

static void test_bandpass_dc_zero(void)
{
    /* DC gain of bandpass should be ~0 */
    float32_t coeffs[5];
    host_calc_bandpass(coeffs, 1000.0f, 48000.0f);

    float32_t dc_gain = (coeffs[B0] + coeffs[B1] + coeffs[B2]) / (1.0f - coeffs[A1] - coeffs[A2]);
    TEST_ASSERT_FLOAT_EQ(0.0f, dc_gain, 0.01f);
}

static void test_bandpass_b0_nonzero(void)
{
    float32_t coeffs[5];
    host_calc_bandpass(coeffs, 1000.0f, 48000.0f);

    TEST_ASSERT(fabsf(coeffs[B0]) > 0.0f);
}

static void test_bandpass_b1_zero(void)
{
    float32_t coeffs[5];
    host_calc_bandpass(coeffs, 1000.0f, 48000.0f);

    TEST_ASSERT_FLOAT_EQ(0.0f, coeffs[B1], 0.0001f);
}

/* =========================================================================
 * Tests: AudioDriver_CalcHighShelf
 * ========================================================================= */

static void test_high_shelf_positive_gain(void)
{
    float32_t coeffs[5];
    host_calc_high_shelf(coeffs, 2000.0f, 1.0f, 6.0f, 48000.0f);

    TEST_ASSERT(fabsf(coeffs[B0]) > 0.0f);
    TEST_ASSERT(fabsf(coeffs[A1]) > 0.0f);
}

static void test_high_shelf_negative_gain(void)
{
    float32_t coeffs[5];
    host_calc_high_shelf(coeffs, 2000.0f, 1.0f, -6.0f, 48000.0f);

    TEST_ASSERT(fabsf(coeffs[B0]) > 0.0f);
}

static void test_high_shelf_zero_gain(void)
{
    float32_t coeffs[5];
    host_calc_high_shelf(coeffs, 2000.0f, 1.0f, 0.0f, 48000.0f);

    /* At 0dB gain, high shelf should be approximately flat */
    float32_t dc_gain = (coeffs[B0] + coeffs[B1] + coeffs[B2]) / (1.0f - coeffs[A1] - coeffs[A2]);
    TEST_ASSERT_FLOAT_EQ(1.0f, dc_gain, 0.1f);
}

static void test_high_shelf_different_Qs(void)
{
    float32_t coeffs[5];
    host_calc_high_shelf(coeffs, 2000.0f, 0.5f, 6.0f, 48000.0f);

    TEST_ASSERT(fabsf(coeffs[B0]) > 0.0f);
    TEST_ASSERT(fabsf(coeffs[A2]) > 0.0f);
}

/* =========================================================================
 * Tests: AudioDriver_CalcLowShelf
 * ========================================================================= */

static void test_low_shelf_positive_gain(void)
{
    float32_t coeffs[5];
    host_calc_low_shelf(coeffs, 200.0f, 1.0f, 6.0f, 48000.0f);

    TEST_ASSERT(fabsf(coeffs[B0]) > 0.0f);
    TEST_ASSERT(fabsf(coeffs[A1]) > 0.0f);
}

static void test_low_shelf_negative_gain(void)
{
    float32_t coeffs[5];
    host_calc_low_shelf(coeffs, 200.0f, 1.0f, -6.0f, 48000.0f);

    TEST_ASSERT(fabsf(coeffs[B0]) > 0.0f);
}

static void test_low_shelf_zero_gain(void)
{
    float32_t coeffs[5];
    host_calc_low_shelf(coeffs, 200.0f, 1.0f, 0.0f, 48000.0f);

    /* At 0dB gain, low shelf should be approximately flat */
    float32_t dc_gain = (coeffs[B0] + coeffs[B1] + coeffs[B2]) / (1.0f - coeffs[A1] - coeffs[A2]);
    TEST_ASSERT_FLOAT_EQ(1.0f, dc_gain, 0.1f);
}

static void test_low_shelf_different_Qs(void)
{
    float32_t coeffs[5];
    host_calc_low_shelf(coeffs, 200.0f, 2.0f, 6.0f, 48000.0f);

    TEST_ASSERT(fabsf(coeffs[B0]) > 0.0f);
    TEST_ASSERT(fabsf(coeffs[A2]) > 0.0f);
}

/* =========================================================================
 * Tests: AudioDriver_ScaleBiquadCoeffs
 * ========================================================================= */

static void test_scale_biquad_coeffs_unity(void)
{
    float32_t coeffs[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    host_scale_biquad_coeffs(coeffs, 1.0f, 1.0f);

    TEST_ASSERT_FLOAT_EQ(4.0f, coeffs[A1], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(5.0f, coeffs[A2], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, coeffs[B0], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(2.0f, coeffs[B1], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(3.0f, coeffs[B2], 0.0001f);
}

static void test_scale_biquad_coeffs_half(void)
{
    float32_t coeffs[5] = {2.0f, 4.0f, 6.0f, 8.0f, 10.0f};
    host_scale_biquad_coeffs(coeffs, 2.0f, 2.0f);

    TEST_ASSERT_FLOAT_EQ(4.0f, coeffs[A1], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(5.0f, coeffs[A2], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, coeffs[B0], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(2.0f, coeffs[B1], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(3.0f, coeffs[B2], 0.0001f);
}

static void test_scale_biquad_coeffs_different_scaling(void)
{
    float32_t coeffs[5] = {4.0f, 6.0f, 8.0f, 10.0f, 12.0f};
    host_scale_biquad_coeffs(coeffs, 2.0f, 4.0f);

    TEST_ASSERT_FLOAT_EQ(5.0f, coeffs[A1], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(6.0f, coeffs[A2], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, coeffs[B0], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(1.5f, coeffs[B1], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(2.0f, coeffs[B2], 0.0001f);
}

/* =========================================================================
 * Tests: Stability / edge cases
 * ========================================================================= */

static void test_bandstop_very_low_freq(void)
{
    float32_t coeffs[5];
    host_calc_bandstop(coeffs, 10.0f, 48000.0f);

    /* Check that coefficients are finite */
    for (int i = 0; i < 5; i++)
    {
        TEST_ASSERT(isfinite(coeffs[i]));
    }
}

static void test_bandpass_very_high_freq(void)
{
    float32_t coeffs[5];
    host_calc_bandpass(coeffs, 20000.0f, 48000.0f);

    for (int i = 0; i < 5; i++)
    {
        TEST_ASSERT(isfinite(coeffs[i]));
    }
}

static void test_high_shelf_very_high_gain(void)
{
    float32_t coeffs[5];
    host_calc_high_shelf(coeffs, 2000.0f, 1.0f, 12.0f, 48000.0f);

    for (int i = 0; i < 5; i++)
    {
        TEST_ASSERT(isfinite(coeffs[i]));
    }
}

static void test_low_shelf_very_low_freq(void)
{
    float32_t coeffs[5];
    host_calc_low_shelf(coeffs, 20.0f, 1.0f, 6.0f, 48000.0f);

    for (int i = 0; i < 5; i++)
    {
        TEST_ASSERT(isfinite(coeffs[i]));
    }
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_audio_biquad_coeffs_register(void)
{
    printf("--- audio_biquad_coeffs ---\n");

    /* Bandstop */
    TEST_REGISTER(test_bandstop_center_frequency);
    TEST_REGISTER(test_bandstop_dc_gain);
    TEST_REGISTER(test_bandstop_nyquist_gain);
    TEST_REGISTER(test_bandstop_higher_freq);
    TEST_REGISTER(test_bandstop_lower_freq);

    /* Bandpass */
    TEST_REGISTER(test_bandpass_center_frequency);
    TEST_REGISTER(test_bandpass_dc_zero);
    TEST_REGISTER(test_bandpass_b0_nonzero);
    TEST_REGISTER(test_bandpass_b1_zero);

    /* High shelf */
    TEST_REGISTER(test_high_shelf_positive_gain);
    TEST_REGISTER(test_high_shelf_negative_gain);
    TEST_REGISTER(test_high_shelf_zero_gain);
    TEST_REGISTER(test_high_shelf_different_Qs);

    /* Low shelf */
    TEST_REGISTER(test_low_shelf_positive_gain);
    TEST_REGISTER(test_low_shelf_negative_gain);
    TEST_REGISTER(test_low_shelf_zero_gain);
    TEST_REGISTER(test_low_shelf_different_Qs);

    /* Scale */
    TEST_REGISTER(test_scale_biquad_coeffs_unity);
    TEST_REGISTER(test_scale_biquad_coeffs_half);
    TEST_REGISTER(test_scale_biquad_coeffs_different_scaling);

    /* Edge cases */
    TEST_REGISTER(test_bandstop_very_low_freq);
    TEST_REGISTER(test_bandpass_very_high_freq);
    TEST_REGISTER(test_high_shelf_very_high_gain);
    TEST_REGISTER(test_low_shelf_very_low_freq);

    printf("\n");
}
