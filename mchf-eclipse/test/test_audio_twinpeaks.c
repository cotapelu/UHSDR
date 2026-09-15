/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_audio_twinpeaks.c                                          **
 **  Description:   Host unit tests for AudioDriver_RxHandleTwinpeaks logic         **
 **                 Reimplements the twinpeak syndrome state machine with the same  **
 **                 thresholds (22.5 deg / M_PI/8) and exercises deterministic      **
 **                 phase-error transitions.                                       **
 **                 The production function is `static` in audio_rx.c; this test    **
 **                 validates the decision logic without pulling in HAL/vendor deps **
 **  Last Modified: 2026-08-25                                                      **
 **  Licence:       GNU GPLv3                                                       **
 ************************************************************************************/

#include "test.h"
#include "mock_uhsdr_board.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* -------------------------------------------------------------------------
 * Twinpeaks state machine (mirrors audio_rx.c)
 * Threshold: |phase_IQ| > PI/8 (22.5 deg) -> TWINPEAKS_CODEC_RESTART
 *
 * On the transition call from WAIT->SAMPLING, the same ISR call also
 * enters the sampling block if teta3 != 0, making phase_IQ_runs = 1.
 * Therefore tests set counter=1 to simulate "after transition".
 * ------------------------------------------------------------------------- */

#define TWINPEAKS_WAIT          0
#define TWINPEAKS_SAMPLING      1
#define TWINPEAKS_DONE          2
#define TWINPEAKS_CODEC_RESTART 4
#define TWINPEAKS_UNCORRECTABLE 5

#define TWINPEAKS_SAMPLES_MAX   50
#define TWINPEAKS_COUNTER_WAIT  1000
#define TWINPEAKS_RESTART_MAX   4

typedef struct {
    uint8_t  twinpeaks_tested;
    uint32_t counter;
    uint32_t codec_restarts;
} twinpeaks_ctx_t;

static void twinpeaks_reset(twinpeaks_ctx_t *ctx)
{
    ctx->twinpeaks_tested = TWINPEAKS_WAIT;
    ctx->counter = 0;
    ctx->codec_restarts = 0;
}

static void twinpeaks_feed(twinpeaks_ctx_t *ctx, float32_t teta1, float32_t teta3)
{
    if (ctx->twinpeaks_tested == TWINPEAKS_WAIT)
    {
        ctx->counter++;
    }

    if (ctx->counter > TWINPEAKS_COUNTER_WAIT)
    {
        ctx->twinpeaks_tested = TWINPEAKS_SAMPLING;
        ctx->counter = 0;
    }

    if (teta3 != 0.0f && ctx->twinpeaks_tested == TWINPEAKS_SAMPLING)
    {
        float32_t phase_cur = asinf(teta1 / teta3);

        ctx->counter++;

        if (ctx->counter == TWINPEAKS_SAMPLES_MAX)
        {
            if (fabsf(phase_cur) > (M_PI / 8.0f))
            {
                ctx->twinpeaks_tested = TWINPEAKS_CODEC_RESTART;
                ctx->codec_restarts++;
                if (ctx->codec_restarts >= TWINPEAKS_RESTART_MAX)
                {
                    ctx->twinpeaks_tested = TWINPEAKS_UNCORRECTABLE;
                    ctx->codec_restarts = 0;
                }
            }
            else
            {
                ctx->twinpeaks_tested = TWINPEAKS_DONE;
                ctx->codec_restarts = 0;
            }
        }
    }
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/* T35.5.1: full wait period with teta3==0 -> transitions to SAMPLING,
 *          sampling block never executes because teta3==0 */
static void test_twinpeaks_skip_zero_teta3(void)
{
    twinpeaks_ctx_t ctx;
    twinpeaks_reset(&ctx);

    for (uint32_t i = 0; i < TWINPEAKS_COUNTER_WAIT + 1; i++)
    {
        twinpeaks_feed(&ctx, 0.5f, 0.0f);
    }

    TEST_ASSERT(ctx.twinpeaks_tested == TWINPEAKS_SAMPLING);
    TEST_ASSERT(ctx.counter == 0);
    TEST_ASSERT(ctx.codec_restarts == 0);
}

/* T35.5.2: after wait, small phase -> TWINPEAKS_DONE */
static void test_twinpeaks_small_phase_done(void)
{
    twinpeaks_ctx_t ctx;
    twinpeaks_reset(&ctx);

    /* Simulate wait period complete: transition to SAMPLING with
     * phase_IQ_runs = 1 (the transition call also enters sampling block) */
    ctx.twinpeaks_tested = TWINPEAKS_SAMPLING;
    ctx.counter = 1;

    /* 49 more samples with small phase -> counter reaches 50 */
    for (int i = 0; i < 49; i++)
    {
        twinpeaks_feed(&ctx, 0.01f, 1.0f); /* phase ~0.01 rad << PI/8 */
    }

    TEST_ASSERT(ctx.twinpeaks_tested == TWINPEAKS_DONE);
    TEST_ASSERT(ctx.codec_restarts == 0);
}

/* T35.5.3: after wait, large phase -> TWINPEAKS_CODEC_RESTART */
static void test_twinpeaks_large_phase_restart(void)
{
    twinpeaks_ctx_t ctx;
    twinpeaks_reset(&ctx);

    ctx.twinpeaks_tested = TWINPEAKS_SAMPLING;
    ctx.counter = 1;

    for (int i = 0; i < 49; i++)
    {
        twinpeaks_feed(&ctx, 1.0f, 1.0f); /* asinf(1.0) = PI/2 > PI/8 */
    }

    TEST_ASSERT(ctx.twinpeaks_tested == TWINPEAKS_CODEC_RESTART);
    TEST_ASSERT(ctx.codec_restarts == 1);
}

/* T35.5.4: 4 restarts across sequential attempts -> TWINPEAKS_UNCORRECTABLE
 *
 * Production behavior: main loop resets ts.twinpeaks_tested back to WAIT
 * after handling CODEC_RESTART, but does NOT reset codec_restarts.
 * The ISR state machine then counts detections cumulatively.
 *
 * This test simulates that by resetting only state/counter per attempt,
 * while letting codec_restarts accumulate.
 */
static void test_twinpeaks_four_restarts_uncorrectable(void)
{
    twinpeaks_ctx_t ctx;
    twinpeaks_reset(&ctx);

    for (int attempt = 0; attempt < 4; attempt++)
    {
        /* simulate main-loop reset back to WAIT, then transition call */
        ctx.twinpeaks_tested = TWINPEAKS_SAMPLING;
        ctx.counter = 1;

        /* 49 more bad samples -> 50th sampling call reaches threshold */
        for (int i = 0; i < 49; i++)
        {
            twinpeaks_feed(&ctx, 1.0f, 1.0f);
        }

        if (attempt < 3)
        {
            TEST_ASSERT(ctx.twinpeaks_tested == TWINPEAKS_CODEC_RESTART);
            TEST_ASSERT(ctx.codec_restarts == (uint32_t)(attempt + 1));
        }
    }

    TEST_ASSERT(ctx.twinpeaks_tested == TWINPEAKS_UNCORRECTABLE);
    TEST_ASSERT(ctx.codec_restarts == 0);
}

/* T35.5.5: boundary near PI/8 -> should NOT restart (no exact float match)
 *
 * Note: sinf(M_PI/8) != M_PI/8 in float; asinf(sinf(x)) can be slightly
 * larger than x due to rounding. This test uses a safe phase clearly
 * below PI/8 to verify the "no restart" branch.
 */
static void test_twinpeaks_boundary_below_threshold(void)
{
    twinpeaks_ctx_t ctx;
    twinpeaks_reset(&ctx);

    ctx.twinpeaks_tested = TWINPEAKS_SAMPLING;
    ctx.counter = 1;

    /* teta1=0.37f -> asinf(0.37) ~= 0.379 rad < PI/8 (~0.393 rad) */
    for (int i = 0; i < 49; i++)
    {
        twinpeaks_feed(&ctx, 0.37f, 1.0f);
    }

    TEST_ASSERT(ctx.twinpeaks_tested == TWINPEAKS_DONE);
}