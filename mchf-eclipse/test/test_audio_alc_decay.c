/**
 * @file test_audio_alc_decay.c
 * @brief Unit tests for audio_management.c ALC decay calculation:
 *        AudioManagement_CalcALCDecay()
 *
 * Reimplements the decay calculation logic inline to avoid
 * hardware-dependent headers and global state.
 */

#include "test.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

typedef float float32_t;

/* -------------------------------------------------------------------------
 * Reimplementation of AudioManagement_CalcALCDecay() core logic
 * ------------------------------------------------------------------------- */

static float32_t host_AudioManagement_CalcALCDecay(int32_t alc_decay_var)
{
    return powf(10.0f, -((((float32_t)alc_decay_var) + 35.0f) / 10.0f));
}

/* =========================================================================
 * Tests: AudioManagement_CalcALCDecay
 * ========================================================================= */

static void test_alc_decay_min_value(void)
{
    /* Test with minimum decay value */
    float32_t result = host_AudioManagement_CalcALCDecay(0);
    TEST_ASSERT_FLOAT_EQ(powf(10.0f, -3.5f), result, 0.0001f);
}

static void test_alc_decay_max_value(void)
{
    /* Test with maximum decay value */
    float32_t result = host_AudioManagement_CalcALCDecay(20);
    TEST_ASSERT_FLOAT_EQ(powf(10.0f, -5.5f), result, 0.0001f);
}

static void test_alc_decay_mid_value(void)
{
    /* Test with mid-range decay value */
    float32_t result = host_AudioManagement_CalcALCDecay(10);
    TEST_ASSERT_FLOAT_EQ(powf(10.0f, -4.5f), result, 0.0001f);
}

static void test_alc_decay_decreases_with_higher_var(void)
{
    /* Higher alc_decay_var should give smaller result (faster decay) */
    float32_t low = host_AudioManagement_CalcALCDecay(0);
    float32_t high = host_AudioManagement_CalcALCDecay(20);
    TEST_ASSERT(low > high);
}

static void test_alc_decay_range(void)
{
    /* All results should be in reasonable range for ALC time constants */
    for (int32_t i = 0; i <= 20; i++)
    {
        float32_t result = host_AudioManagement_CalcALCDecay(i);
        TEST_ASSERT(result > 0.0f);
        TEST_ASSERT(result < 1.0f);
    }
}

static void test_alc_decay_monotonic(void)
{
    /* Result should monotonically decrease as input increases */
    float32_t prev = host_AudioManagement_CalcALCDecay(0);
    for (int32_t i = 1; i <= 20; i++)
    {
        float32_t curr = host_AudioManagement_CalcALCDecay(i);
        TEST_ASSERT(curr < prev);
        prev = curr;
    }
}

static void test_alc_decay_negative_var(void)
{
    /* Negative decay var should still produce valid result */
    float32_t result = host_AudioManagement_CalcALCDecay(-5);
    TEST_ASSERT_FLOAT_EQ(powf(10.0f, -3.0f), result, 0.0001f);
}

static void test_alc_decay_formula_verification(void)
{
    /* Verify the exact formula: 10^(-((x + 35) / 10)) */
    int32_t test_val = 15;
    float32_t expected = powf(10.0f, -((test_val + 35) / 10.0f));
    float32_t result = host_AudioManagement_CalcALCDecay(test_val);
    TEST_ASSERT_FLOAT_EQ(expected, result, 0.0001f);
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_audio_alc_decay_register(void)
{
    printf("--- audio_alc_decay ---\n");

    TEST_REGISTER(test_alc_decay_min_value);
    TEST_REGISTER(test_alc_decay_max_value);
    TEST_REGISTER(test_alc_decay_mid_value);
    TEST_REGISTER(test_alc_decay_decreases_with_higher_var);
    TEST_REGISTER(test_alc_decay_range);
    TEST_REGISTER(test_alc_decay_monotonic);
    TEST_REGISTER(test_alc_decay_negative_var);
    TEST_REGISTER(test_alc_decay_formula_verification);

    printf("\n");
}
