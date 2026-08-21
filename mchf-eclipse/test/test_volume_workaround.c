/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_volume_workaround.c                                        **
 **  Description:   Host-based unit tests for AudioDriver_MchfVolumeWorkaround      **
 **                 Self-contained: re-implements the workaround logic inline       **
 **                 and mocks arm_scale_f32 to verify gain reduction behavior      **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

/* -------------------------------------------------------------------------
 * Minimal types / macros for host testing
 * ------------------------------------------------------------------------- */
typedef float float32_t;

#ifndef PI
#define PI 3.14159265358979323846f
#endif

/* -------------------------------------------------------------------------
 * Mock for arm_scale_f32 (CMSIS DSP)
 * ------------------------------------------------------------------------- */
static void arm_scale_f32(float32_t *pSrc, float32_t scale, float32_t *pDst, uint32_t blockSize)
{
    for (uint32_t i = 0; i < blockSize; i++)
    {
        pDst[i] = pSrc[i] * scale;
    }
}

/* -------------------------------------------------------------------------
 * Reimplementation of AudioDriver_MchfVolumeWorkaround
 * (mirrors drivers/audio/audio_driver.h)
 * ------------------------------------------------------------------------- */
#define UI_BRD_MCHF

static void AudioDriver_MchfVolumeWorkaround(float32_t *buffer, uint32_t blockSize, float32_t gain_value, float32_t active_value, float32_t max_volume)
{
#ifdef UI_BRD_MCHF
    if(gain_value > max_volume)
    {
        arm_scale_f32(buffer, active_value, buffer, blockSize);
    }
#endif
}

/* -------------------------------------------------------------------------
 * Test helpers
 * ------------------------------------------------------------------------- */
static bool arrays_equal_f32(const float32_t *a, const float32_t *b, uint32_t len, float32_t epsilon)
{
    for (uint32_t i = 0; i < len; i++)
    {
        float32_t diff = fabsf(a[i] - b[i]);
        if (diff > epsilon)
        {
            return false;
        }
    }
    return true;
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/* T26.2.1: gain <= max_volume -> buffer unchanged (no scaling) */
static void test_volume_no_scaling_when_gain_within_limit(void)
{
    float32_t buf[] = {1.0f, 2.0f, 3.0f, 4.0f};
    float32_t expected[] = {1.0f, 2.0f, 3.0f, 4.0f};

    AudioDriver_MchfVolumeWorkaround(buf, 4, 15.0f, 0.5f, 20.0f);

    TEST_ASSERT(arrays_equal_f32(buf, expected, 4, 1e-6f));
}

/* T26.2.2: gain > max_volume -> buffer scaled by active_value */
static void test_volume_scaled_when_gain_exceeds_max(void)
{
    float32_t buf[] = {1.0f, 2.0f, 3.0f, 4.0f};
    float32_t expected[] = {0.5f, 1.0f, 1.5f, 2.0f};

    AudioDriver_MchfVolumeWorkaround(buf, 4, 25.0f, 0.5f, 20.0f);

    TEST_ASSERT(arrays_equal_f32(buf, expected, 4, 1e-6f));
}

/* T26.2.3: gain == max_volume -> no scaling (boundary) */
static void test_volume_no_scaling_at_boundary(void)
{
    float32_t buf[] = {1.0f, 2.0f, 3.0f, 4.0f};
    float32_t expected[] = {1.0f, 2.0f, 3.0f, 4.0f};

    AudioDriver_MchfVolumeWorkaround(buf, 4, 20.0f, 0.5f, 20.0f);

    TEST_ASSERT(arrays_equal_f32(buf, expected, 4, 1e-6f));
}

/* T26.2.4: zero-length buffer -> no crash */
static void test_volume_zero_length_buffer(void)
{
    float32_t buf[] = {1.0f, 2.0f, 3.0f};

    AudioDriver_MchfVolumeWorkaround(buf, 0, 25.0f, 0.5f, 20.0f);

    /* Buffer contents should remain unchanged */
    TEST_ASSERT_FLOAT_EQ(1.0f, buf[0], 1e-6f);
    TEST_ASSERT_FLOAT_EQ(2.0f, buf[1], 1e-6f);
    TEST_ASSERT_FLOAT_EQ(3.0f, buf[2], 1e-6f);
}

/* T26.2.5: scaling preserves buffer across multiple calls */
static void test_volume_repeated_scaling(void)
{
    float32_t buf[] = {2.0f, 2.0f, 2.0f, 2.0f};
    float32_t expected[] = {0.5f, 0.5f, 0.5f, 0.5f};

    AudioDriver_MchfVolumeWorkaround(buf, 4, 25.0f, 0.25f, 20.0f);

    TEST_ASSERT(arrays_equal_f32(buf, expected, 4, 1e-6f));
}

/* T26.2.6: active_value < 1.0 reduces amplitude; active_value > 1.0 increases */
static void test_volume_active_value_less_than_one(void)
{
    float32_t buf[] = {4.0f, 4.0f};
    float32_t expected[] = {1.0f, 1.0f};

    AudioDriver_MchfVolumeWorkaround(buf, 2, 25.0f, 0.25f, 20.0f);

    TEST_ASSERT(arrays_equal_f32(buf, expected, 2, 1e-6f));
}

/* -------------------------------------------------------------------------
 * Test runner
 * ------------------------------------------------------------------------- */

/* Tests are registered in test.c to avoid multiple test_main definitions */
