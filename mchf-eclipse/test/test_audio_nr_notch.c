/**
 * @file test_audio_nr_notch.c
 * @brief Unit tests for audio_nr.c biquad notch filter coefficient calculation:
 *        AudioNr_CalculateAutoNotch()
 *
 * Reimplements the coefficient calculation logic inline to avoid
 * hardware-dependent headers and global state.
 */

#include "test.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

typedef float float32_t;

/* -------------------------------------------------------------------------
 * Reimplementation of AudioNr_CalculateAutoNotch() core logic
 * ------------------------------------------------------------------------- */

static void host_AudioNr_CalculateAutoNotch(float32_t coeffs[6],
                                            uint8_t notch1_bin,
                                            bool notch1_active,
                                            float32_t BW,
                                            float32_t FS,
                                            uint16_t NR_FFT_L)
{
    (void)notch1_active; /* not used in coefficient calculation */
    (void)BW;            /* not used in current implementation (Q is fixed at 10) */

    float32_t bin_BW = FS / (float32_t)NR_FFT_L;
    float32_t f0 = ((float32_t)notch1_bin + 0.5f) * bin_BW;
    float32_t Q = 10.0f;
    float32_t w0 = 6.28318530717958f * f0 / FS;
    float32_t alpha = sinf(w0) / (2.0f * Q);

    coeffs[0] = 1.0f;
    coeffs[1] = -2.0f * cosf(w0);
    coeffs[2] = 1.0f;
    coeffs[3] = 1.0f + alpha;
    coeffs[4] = 2.0f * cosf(w0);
    coeffs[5] = alpha - 1.0f;
}

/* =========================================================================
 * Tests: AudioNr_CalculateAutoNotch
 * ========================================================================= */

static void test_notch_coeffs_symmetry(void)
{
    /* Notch filter should have a0 = a2 = 1 */
    float32_t coeffs[6];
    host_AudioNr_CalculateAutoNotch(coeffs, 10, true, 1.0f, 12000.0f, 256);
    TEST_ASSERT_FLOAT_EQ(1.0f, coeffs[0], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, coeffs[2], 0.0001f);
}

static void test_notch_coeffs_b1_negative_cos(void)
{
    /* b1 should be -2*cos(w0) */
    float32_t coeffs[6];
    host_AudioNr_CalculateAutoNotch(coeffs, 0, true, 1.0f, 12000.0f, 256);
    TEST_ASSERT_FLOAT_EQ(-2.0f * cosf(6.28318530717958f * 0.5f * (12000.0f/256.0f) / 12000.0f), coeffs[1], 0.0001f);
}

static void test_notch_coeffs_a1_positive_cos(void)
{
    /* a1 should be 2*cos(w0) = -b1 */
    float32_t coeffs[6];
    host_AudioNr_CalculateAutoNotch(coeffs, 10, true, 1.0f, 12000.0f, 256);
    TEST_ASSERT_FLOAT_EQ(-coeffs[1], coeffs[4], 0.0001f);
}

static void test_notch_coeffs_b0_equals_b2(void)
{
    /* For notch: b0 = b2 = 1 */
    float32_t coeffs[6];
    host_AudioNr_CalculateAutoNotch(coeffs, 50, true, 1.0f, 48000.0f, 256);
    TEST_ASSERT_FLOAT_EQ(coeffs[0], coeffs[2], 0.0001f);
}

static void test_notch_coeffs_center_frequency(void)
{
    /* Verify coefficient changes with different center frequencies */
    float32_t coeffs_low[6];
    float32_t coeffs_high[6];
    host_AudioNr_CalculateAutoNotch(coeffs_low, 5, true, 1.0f, 12000.0f, 256);
    host_AudioNr_CalculateAutoNotch(coeffs_high, 50, true, 1.0f, 12000.0f, 256);
    /* Different bins should produce different coefficients */
    TEST_ASSERT(coeffs_low[1] != coeffs_high[1] || coeffs_low[4] != coeffs_high[4]);
}

static void test_notch_coeffs_sample_rate_effect(void)
{
    /* Same bin, different sample rate: coefficients are independent of FS
     * because w0 = 2*PI * (bin+0.5) / NR_FFT_L (FS cancels out) */
    float32_t coeffs_12k[6];
    float32_t coeffs_48k[6];
    host_AudioNr_CalculateAutoNotch(coeffs_12k, 10, true, 1.0f, 12000.0f, 256);
    host_AudioNr_CalculateAutoNotch(coeffs_48k, 10, true, 1.0f, 48000.0f, 256);
    TEST_ASSERT_FLOAT_EQ(coeffs_12k[0], coeffs_48k[0], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(coeffs_12k[1], coeffs_48k[1], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(coeffs_12k[4], coeffs_48k[4], 0.0001f);
}

static void test_notch_coeffs_fft_size_effect(void)
{
    /* Same bin, same sample rate, different FFT size: different bin_BW, different coeffs */
    float32_t coeffs_128[6];
    float32_t coeffs_256[6];
    host_AudioNr_CalculateAutoNotch(coeffs_128, 10, true, 1.0f, 12000.0f, 128);
    host_AudioNr_CalculateAutoNotch(coeffs_256, 10, true, 1.0f, 12000.0f, 256);
    TEST_ASSERT(coeffs_128[1] != coeffs_256[1] || coeffs_128[4] != coeffs_256[4]);
}

static void test_notch_coeffs_bin_zero(void)
{
    /* Bin 0: lowest frequency notch */
    float32_t coeffs[6];
    host_AudioNr_CalculateAutoNotch(coeffs, 0, true, 1.0f, 12000.0f, 256);
    TEST_ASSERT_FLOAT_EQ(1.0f, coeffs[0], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, coeffs[2], 0.0001f);
    /* a1 should be close to 2 for very low frequency */
    TEST_ASSERT(coeffs[4] > 1.9f);
}

static void test_notch_coeffs_bin_max(void)
{
    /* Highest valid bin: NR_FFT_L/2 - 1 */
    float32_t coeffs[6];
    host_AudioNr_CalculateAutoNotch(coeffs, 127, true, 1.0f, 12000.0f, 256);
    TEST_ASSERT_FLOAT_EQ(1.0f, coeffs[0], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(1.0f, coeffs[2], 0.0001f);
}

static void test_notch_coeffs_q_constant(void)
{
    /* Q is hardcoded to 10, alpha = sin(w0)/(2*Q) */
    float32_t coeffs[6];
    host_AudioNr_CalculateAutoNotch(coeffs, 20, true, 1.0f, 12000.0f, 256);
    float32_t w0 = 6.28318530717958f * ((20.0f + 0.5f) * (12000.0f/256.0f)) / 12000.0f;
    float32_t expected_alpha = sinf(w0) / 20.0f;
    TEST_ASSERT_FLOAT_EQ(expected_alpha, coeffs[5] + 1.0f, 0.0001f);
}

static void test_notch_coeffs_a3_a5_sign(void)
{
    /* a3 = 1 + alpha > 1 (since alpha > 0 for w0 in (0, PI)) */
    float32_t coeffs[6];
    host_AudioNr_CalculateAutoNotch(coeffs, 10, true, 1.0f, 12000.0f, 256);
    TEST_ASSERT(coeffs[3] > 1.0f);
    /* a5 = alpha - 1 < 0 (since alpha < 1 for reasonable Q) */
    TEST_ASSERT(coeffs[5] < 0.0f);
}

static void test_notch_coeffs_active_flag_ignored(void)
{
    /* notch1_active does not affect coefficient calculation */
    float32_t coeffs_active[6];
    float32_t coeffs_inactive[6];
    host_AudioNr_CalculateAutoNotch(coeffs_active, 25, true, 1.0f, 12000.0f, 256);
    host_AudioNr_CalculateAutoNotch(coeffs_inactive, 25, false, 1.0f, 12000.0f, 256);
    TEST_ASSERT_FLOAT_EQ(coeffs_active[0], coeffs_inactive[0], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(coeffs_active[1], coeffs_inactive[1], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(coeffs_active[5], coeffs_inactive[5], 0.0001f);
}

static void test_notch_coeffs_bw_ignored(void)
{
    /* BW parameter is not used in current implementation (Q is fixed at 10) */
    float32_t coeffs_bw1[6];
    float32_t coeffs_bw2[6];
    host_AudioNr_CalculateAutoNotch(coeffs_bw1, 30, true, 1.0f, 12000.0f, 256);
    host_AudioNr_CalculateAutoNotch(coeffs_bw2, 30, true, 5.0f, 12000.0f, 256);
    TEST_ASSERT_FLOAT_EQ(coeffs_bw1[0], coeffs_bw2[0], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(coeffs_bw1[1], coeffs_bw2[1], 0.0001f);
    TEST_ASSERT_FLOAT_EQ(coeffs_bw1[5], coeffs_bw2[5], 0.0001f);
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_audio_nr_notch_register(void)
{
    printf("--- audio_nr notch coeffs ---\n");

    TEST_REGISTER(test_notch_coeffs_symmetry);
    TEST_REGISTER(test_notch_coeffs_b1_negative_cos);
    TEST_REGISTER(test_notch_coeffs_a1_positive_cos);
    TEST_REGISTER(test_notch_coeffs_b0_equals_b2);
    TEST_REGISTER(test_notch_coeffs_center_frequency);
    TEST_REGISTER(test_notch_coeffs_sample_rate_effect);
    TEST_REGISTER(test_notch_coeffs_fft_size_effect);
    TEST_REGISTER(test_notch_coeffs_bin_zero);
    TEST_REGISTER(test_notch_coeffs_bin_max);
    TEST_REGISTER(test_notch_coeffs_q_constant);
    TEST_REGISTER(test_notch_coeffs_a3_a5_sign);
    TEST_REGISTER(test_notch_coeffs_active_flag_ignored);
    TEST_REGISTER(test_notch_coeffs_bw_ignored);

    printf("\n");
}
