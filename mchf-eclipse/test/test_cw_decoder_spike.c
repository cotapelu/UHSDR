/**
 * @file test_cw_decoder_spike.c
 * @brief Unit tests for cw_decoder.c CwDecoder_IsSpike()
 *
 * Reimplements the spike detection logic inline to avoid
 * hardware-dependent headers and global state.
 */

#include "test.h"
#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Replicated constants from cw_decoder.h / cw_decoder.c
 * ------------------------------------------------------------------------- */

#define CW_SPIKECANCEL_MODE_OFF   0
#define CW_SPIKECANCEL_MODE_SPIKE 1
#define CW_SPIKECANCEL_MODE_SHORT 2
#define CW_SPIKECANCEL_MAX_DURATION 8

/* -------------------------------------------------------------------------
 * Reimplementation of CwDecoder_IsSpike()
 * ------------------------------------------------------------------------- */

static bool host_cw_decoder_is_spike(uint32_t t, uint32_t spikecancel, bool initialized, uint32_t dot_avg)
{
    bool retval = false;

    if (spikecancel == CW_SPIKECANCEL_MODE_SPIKE)
    {
        retval = t <= CW_SPIKECANCEL_MAX_DURATION;
    }
    else if (spikecancel == CW_SPIKECANCEL_MODE_SHORT)
    {
        retval = (3 * t < dot_avg) && (initialized == true);
    }
    return retval;
}

/* =========================================================================
 * Tests: CwDecoder_IsSpike
 * ========================================================================= */

static void test_spike_mode_off_always_false(void)
{
    /* When spike cancel is OFF, always return false */
    TEST_ASSERT(!host_cw_decoder_is_spike(1, CW_SPIKECANCEL_MODE_OFF, true, 100));
    TEST_ASSERT(!host_cw_decoder_is_spike(100, CW_SPIKECANCEL_MODE_OFF, true, 100));
}

static void test_spike_mode_threshold(void)
{
    /* In SPIKE mode, duration <= 8 is a spike */
    TEST_ASSERT(host_cw_decoder_is_spike(8, CW_SPIKECANCEL_MODE_SPIKE, true, 100));
    TEST_ASSERT(host_cw_decoder_is_spike(1, CW_SPIKECANCEL_MODE_SPIKE, true, 100));
}

static void test_spike_mode_above_threshold(void)
{
    /* In SPIKE mode, duration > 8 is not a spike */
    TEST_ASSERT(!host_cw_decoder_is_spike(9, CW_SPIKECANCEL_MODE_SPIKE, true, 100));
    TEST_ASSERT(!host_cw_decoder_is_spike(100, CW_SPIKECANCEL_MODE_SPIKE, true, 100));
}

static void test_spike_mode_edge_at_threshold(void)
{
    /* Exact threshold boundary */
    TEST_ASSERT(host_cw_decoder_is_spike(8, CW_SPIKECANCEL_MODE_SPIKE, true, 100));
    TEST_ASSERT(!host_cw_decoder_is_spike(9, CW_SPIKECANCEL_MODE_SPIKE, true, 100));
}

static void test_short_mode_not_initialized(void)
{
    /* In SHORT mode, not initialized -> never a spike */
    TEST_ASSERT(!host_cw_decoder_is_spike(1, CW_SPIKECANCEL_MODE_SHORT, false, 100));
    TEST_ASSERT(!host_cw_decoder_is_spike(10, CW_SPIKECANCEL_MODE_SHORT, false, 100));
}

static void test_short_mode_short_duration(void)
{
    /* In SHORT mode, short duration with initialized=true is spike */
    TEST_ASSERT(host_cw_decoder_is_spike(10, CW_SPIKECANCEL_MODE_SHORT, true, 100));
    TEST_ASSERT(host_cw_decoder_is_spike(30, CW_SPIKECANCEL_MODE_SHORT, true, 100));
}

static void test_short_mode_long_duration(void)
{
    /* In SHORT mode, long duration is not spike */
    TEST_ASSERT(!host_cw_decoder_is_spike(50, CW_SPIKECANCEL_MODE_SHORT, true, 100));
}

static void test_short_mode_edge_at_one_third_dot(void)
{
    /* In SHORT mode, 3*t < dot_avg is spike */
    TEST_ASSERT(host_cw_decoder_is_spike(33, CW_SPIKECANCEL_MODE_SHORT, true, 100));
    TEST_ASSERT(!host_cw_decoder_is_spike(34, CW_SPIKECANCEL_MODE_SHORT, true, 100));
}

static void test_short_mode_different_dot_avg(void)
{
    /* Different dot_avg values should change threshold */
    TEST_ASSERT(host_cw_decoder_is_spike(10, CW_SPIKECANCEL_MODE_SHORT, true, 50));
    TEST_ASSERT(!host_cw_decoder_is_spike(10, CW_SPIKECANCEL_MODE_SHORT, true, 25));
}

static void test_zero_duration_in_spike_mode(void)
{
    /* Zero duration should be a spike in SPIKE mode */
    TEST_ASSERT(host_cw_decoder_is_spike(0, CW_SPIKECANCEL_MODE_SPIKE, true, 100));
}

static void test_zero_duration_in_short_mode(void)
{
    /* Zero duration should be a spike in SHORT mode when initialized */
    TEST_ASSERT(host_cw_decoder_is_spike(0, CW_SPIKECANCEL_MODE_SHORT, true, 100));
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_cw_decoder_spike_register(void)
{
    printf("--- cw_decoder_spike ---\n");

    TEST_REGISTER(test_spike_mode_off_always_false);
    TEST_REGISTER(test_spike_mode_threshold);
    TEST_REGISTER(test_spike_mode_above_threshold);
    TEST_REGISTER(test_spike_mode_edge_at_threshold);
    TEST_REGISTER(test_short_mode_not_initialized);
    TEST_REGISTER(test_short_mode_short_duration);
    TEST_REGISTER(test_short_mode_long_duration);
    TEST_REGISTER(test_short_mode_edge_at_one_third_dot);
    TEST_REGISTER(test_short_mode_different_dot_avg);
    TEST_REGISTER(test_zero_duration_in_spike_mode);
    TEST_REGISTER(test_zero_duration_in_short_mode);

    printf("\n");
}
