/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_ui_driver_remote.c                                         **
 **  Description:   Host unit tests for UiDriver_Remote_Init/Poll edge detection    **
 **                 (T46.1: remote input capture on remote unit)                    **
 **  Last Modified: 2026-08-28                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include <stdint.h>

/* -------------------------------------------------------------------------
 * Reimplemented logic under test (avoids hardware-dependent headers)
 * ------------------------------------------------------------------------- */
#define BUTTON_NUM 18

typedef struct {
    int32_t last_enc_freq;
    uint32_t last_button_states;
} remote_ctx_t;

static int8_t sent_encoder_deltas[64];
static uint32_t sent_encoder_count;
static uint8_t sent_button_ids[128];
static uint8_t sent_button_pressed[128];
static uint32_t sent_button_count;

static void record_reset(void)
{
    sent_encoder_count = 0;
    sent_button_count = 0;
    memset(sent_encoder_deltas, 0, sizeof(sent_encoder_deltas));
    memset(sent_button_ids, 0, sizeof(sent_button_ids));
    memset(sent_button_pressed, 0, sizeof(sent_button_pressed));
}

static void mock_Remote_SendEncoderDelta(int8_t delta)
{
    if (sent_encoder_count < sizeof(sent_encoder_deltas)) {
        sent_encoder_deltas[sent_encoder_count++] = delta;
    }
}

static void mock_Remote_SendButton(uint8_t button_id, uint8_t pressed)
{
    if (sent_button_count < sizeof(sent_button_ids)) {
        sent_button_ids[sent_button_count] = button_id;
        sent_button_pressed[sent_button_count] = pressed;
        sent_button_count++;
    }
}

static void UiDriver_Remote_Init_under_test(remote_ctx_t *ctx)
{
    ctx->last_enc_freq = 0;
    ctx->last_button_states = 0; /* Keypad_ButtonStates() mocked as 0 at init */
}

static void UiDriver_Remote_Poll_under_test(remote_ctx_t *ctx,
                                            int32_t encoder_delta,
                                            uint32_t button_states)
{
    int32_t delta;
    uint32_t cur_button_states;
    uint32_t changed;
    uint32_t pressed_now;
    uint32_t released_now;
    int i;

    /* ---- Encoder capture ---- */
    delta = encoder_delta;
    if (delta != 0)
    {
        int8_t sdelta = (int8_t)delta;
        if (sdelta != 0)
        {
            mock_Remote_SendEncoderDelta(sdelta);
        }
    }

    /* ---- Button capture ---- */
    cur_button_states = button_states;

    changed = cur_button_states ^ ctx->last_button_states;
    pressed_now = changed & cur_button_states;
    released_now = changed & ctx->last_button_states;

    /* Send press events */
    for (i = 0; i < BUTTON_NUM; i++)
    {
        if (pressed_now & (1U << i))
        {
            mock_Remote_SendButton(i, 1);
        }
    }

    /* Send release events */
    for (i = 0; i < BUTTON_NUM; i++)
    {
        if (released_now & (1U << i))
        {
            mock_Remote_SendButton(i, 0);
        }
    }

    ctx->last_button_states = cur_button_states;
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */
static void test_init_resets_state(void)
{
    remote_ctx_t ctx;
    ctx.last_enc_freq = 123;
    ctx.last_button_states = 0xFFFF;

    UiDriver_Remote_Init_under_test(&ctx);

    TEST_ASSERT_EQUAL(0, ctx.last_enc_freq);
    TEST_ASSERT_EQUAL(0, ctx.last_button_states);
}

static void test_poll_no_events_when_idle(void)
{
    remote_ctx_t ctx;
    UiDriver_Remote_Init_under_test(&ctx);
    record_reset();

    UiDriver_Remote_Poll_under_test(&ctx, 0, 0);

    TEST_ASSERT_EQUAL(0, sent_encoder_count);
    TEST_ASSERT_EQUAL(0, sent_button_count);
}

static void test_poll_sends_encoder_delta(void)
{
    remote_ctx_t ctx;
    UiDriver_Remote_Init_under_test(&ctx);
    record_reset();

    UiDriver_Remote_Poll_under_test(&ctx, 5, 0);

    TEST_ASSERT_EQUAL(1, sent_encoder_count);
    TEST_ASSERT_EQUAL(5, sent_encoder_deltas[0]);
}

static void test_poll_sends_negative_encoder_delta(void)
{
    remote_ctx_t ctx;
    UiDriver_Remote_Init_under_test(&ctx);
    record_reset();

    UiDriver_Remote_Poll_under_test(&ctx, -3, 0);

    TEST_ASSERT_EQUAL(1, sent_encoder_count);
    TEST_ASSERT_EQUAL(-3, sent_encoder_deltas[0]);
}

static void test_poll_ignores_zero_encoder_delta(void)
{
    remote_ctx_t ctx;
    UiDriver_Remote_Init_under_test(&ctx);
    record_reset();

    UiDriver_Remote_Poll_under_test(&ctx, 0, 0);

    TEST_ASSERT_EQUAL(0, sent_encoder_count);
}

static void test_poll_detects_button_press(void)
{
    remote_ctx_t ctx;
    UiDriver_Remote_Init_under_test(&ctx);
    record_reset();

    /* Button 3 pressed (bit 3 set) */
    UiDriver_Remote_Poll_under_test(&ctx, 0, (1U << 3));

    TEST_ASSERT_EQUAL(0, sent_encoder_count);
    TEST_ASSERT_EQUAL(1, sent_button_count);
    TEST_ASSERT_EQUAL(3, sent_button_ids[0]);
    TEST_ASSERT_EQUAL(1, sent_button_pressed[0]);
}

static void test_poll_detects_button_release(void)
{
    remote_ctx_t ctx;
    UiDriver_Remote_Init_under_test(&ctx);

    /* First poll: button 5 pressed */
    UiDriver_Remote_Poll_under_test(&ctx, 0, (1U << 5));
    record_reset();

    /* Second poll: button 5 released */
    UiDriver_Remote_Poll_under_test(&ctx, 0, 0);

    TEST_ASSERT_EQUAL(1, sent_button_count);
    TEST_ASSERT_EQUAL(5, sent_button_ids[0]);
    TEST_ASSERT_EQUAL(0, sent_button_pressed[0]);
}

static void test_poll_sends_multiple_button_events(void)
{
    remote_ctx_t ctx;
    UiDriver_Remote_Init_under_test(&ctx);
    record_reset();

    /* Buttons 1 and 4 pressed together */
    UiDriver_Remote_Poll_under_test(&ctx, 0, (1U << 1) | (1U << 4));

    TEST_ASSERT_EQUAL(2, sent_button_count);
    TEST_ASSERT_EQUAL(1, sent_button_ids[0]);
    TEST_ASSERT_EQUAL(1, sent_button_pressed[0]);
    TEST_ASSERT_EQUAL(4, sent_button_ids[1]);
    TEST_ASSERT_EQUAL(1, sent_button_pressed[1]);
}

static void test_poll_no_duplicate_events_when_stable(void)
{
    remote_ctx_t ctx;
    UiDriver_Remote_Init_under_test(&ctx);

    /* First poll: button 2 pressed */
    UiDriver_Remote_Poll_under_test(&ctx, 0, (1U << 2));
    record_reset();

    /* Second poll: same state, no change */
    UiDriver_Remote_Poll_under_test(&ctx, 0, (1U << 2));

    TEST_ASSERT_EQUAL(0, sent_button_count);
}

static void test_poll_simultaneous_press_and_release(void)
{
    remote_ctx_t ctx;
    UiDriver_Remote_Init_under_test(&ctx);

    /* First poll: button 0 pressed, button 7 pressed */
    UiDriver_Remote_Poll_under_test(&ctx, 0, (1U << 0) | (1U << 7));
    record_reset();

    /* Second poll: button 0 released, button 7 still pressed */
    UiDriver_Remote_Poll_under_test(&ctx, 0, (1U << 7));

    TEST_ASSERT_EQUAL(1, sent_button_count);
    TEST_ASSERT_EQUAL(0, sent_button_ids[0]);
    TEST_ASSERT_EQUAL(0, sent_button_pressed[0]);
}

static void test_poll_encoder_and_button_same_cycle(void)
{
    remote_ctx_t ctx;
    UiDriver_Remote_Init_under_test(&ctx);
    record_reset();

    /* Encoder delta + button press in same poll */
    UiDriver_Remote_Poll_under_test(&ctx, 2, (1U << 9));

    TEST_ASSERT_EQUAL(1, sent_encoder_count);
    TEST_ASSERT_EQUAL(2, sent_encoder_deltas[0]);
    TEST_ASSERT_EQUAL(1, sent_button_count);
    TEST_ASSERT_EQUAL(9, sent_button_ids[0]);
    TEST_ASSERT_EQUAL(1, sent_button_pressed[0]);
}

static void test_poll_button_beyond_num_ignored(void)
{
    remote_ctx_t ctx;
    UiDriver_Remote_Init_under_test(&ctx);
    record_reset();

    /* Set bit 18 (BUTTON_NUM or beyond) */
    UiDriver_Remote_Poll_under_test(&ctx, 0, (1U << 18));

    /* Should be ignored because loop runs only < BUTTON_NUM */
    TEST_ASSERT_EQUAL(0, sent_button_count);
}

static void test_poll_encoder_delta_truncation_to_int8(void)
{
    remote_ctx_t ctx;
    UiDriver_Remote_Init_under_test(&ctx);
    record_reset();

    /* 200 fits in int8_t (max 127)? No, 200 > 127, will truncate.
     * But production code expects UiDriverEncoderRead to return small values.
     * We test the cast behavior anyway. */
    UiDriver_Remote_Poll_under_test(&ctx, 200, 0);

    /* int8_t cast of 200 is implementation-defined; on two's complement it's -56 */
    TEST_ASSERT_EQUAL(1, sent_encoder_count);
    TEST_ASSERT_EQUAL(-56, sent_encoder_deltas[0]);
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */
void test_ui_driver_remote_register(void)
{
    printf("--- ui_driver_remote (T46.1) ---\n");
    TEST_REGISTER(test_init_resets_state);
    TEST_REGISTER(test_poll_no_events_when_idle);
    TEST_REGISTER(test_poll_sends_encoder_delta);
    TEST_REGISTER(test_poll_sends_negative_encoder_delta);
    TEST_REGISTER(test_poll_ignores_zero_encoder_delta);
    TEST_REGISTER(test_poll_detects_button_press);
    TEST_REGISTER(test_poll_detects_button_release);
    TEST_REGISTER(test_poll_sends_multiple_button_events);
    TEST_REGISTER(test_poll_no_duplicate_events_when_stable);
    TEST_REGISTER(test_poll_simultaneous_press_and_release);
    TEST_REGISTER(test_poll_encoder_and_button_same_cycle);
    TEST_REGISTER(test_poll_button_beyond_num_ignored);
    TEST_REGISTER(test_poll_encoder_delta_truncation_to_int8);
    printf("\n");
}
