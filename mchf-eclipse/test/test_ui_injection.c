/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                             Host unit tests: UI injection APIs                  **
 **                                                                                 **
 **--------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_ui_injection.c                                             **
 **  Description:   Tests for UiDriver_InjectButton() and UiDriverEncoderSetDelta() **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"

/* -------------------------------------------------------------------------
 * Reimplemented logic under test (avoids hardware-dependent headers)
 * ------------------------------------------------------------------------- */

#define USE_DETENTED_VALUE   4
#define ENC_MAX              4

typedef struct KeypadState
{
    unsigned long debounce_time;
    unsigned long button_id;
    bool button_pressed;
    bool button_released;
    bool button_processed;
    bool button_just_pressed;
    bool debounce_check_complete;
    bool press_hold;
    unsigned char dummy;
} KeypadState;

typedef struct
{
    unsigned short value_old;
    unsigned short value_new;
    unsigned char de_detent;
} EncoderSelection;

static void UiDriver_InjectButton_under_test(uint8_t button_id, KeypadState *ks)
{
    ks->button_id = button_id;
    ks->button_pressed = true;
    ks->button_processed = 1;
    ks->button_just_pressed = 1;
    ks->press_hold = 0;
    /* UiDriver_HandleKeyboard() omitted in host test */
}

static void UiDriverEncoderSetDelta_under_test(uint32_t encId, int8_t delta,
                                               EncoderSelection *encSel,
                                               unsigned int *counter)
{
    if (encId < ENC_MAX)
    {
        unsigned int cnt = *counter;
        *counter = cnt + delta;
        encSel[encId].value_old = cnt;
        unsigned char abs_delta = (delta < 0) ? (unsigned char)(-delta) : (unsigned char)delta;
        if (abs_delta < USE_DETENTED_VALUE)
        {
            encSel[encId].de_detent = USE_DETENTED_VALUE - abs_delta;
        }
        else
        {
            encSel[encId].de_detent = 0;
        }
    }
}

/* -------------------------------------------------------------------------
 * UiDriver_InjectButton tests
 * ------------------------------------------------------------------------- */

static void test_inject_button_sets_all_fields(void)
{
    KeypadState ks;
    memset(&ks, 0xAA, sizeof(ks));

    UiDriver_InjectButton_under_test(7, &ks);

    TEST_ASSERT_EQUAL(7, ks.button_id);
    TEST_ASSERT_EQUAL(1, ks.button_pressed);
    TEST_ASSERT_EQUAL(1, ks.button_processed);
    TEST_ASSERT_EQUAL(1, ks.button_just_pressed);
    TEST_ASSERT_EQUAL(0, ks.press_hold);
}

static void test_inject_button_zero_id(void)
{
    KeypadState ks;
    memset(&ks, 0, sizeof(ks));

    UiDriver_InjectButton_under_test(0, &ks);

    TEST_ASSERT_EQUAL(0, ks.button_id);
    TEST_ASSERT_EQUAL(1, ks.button_pressed);
    TEST_ASSERT_EQUAL(1, ks.button_processed);
}

static void test_inject_button_max_id(void)
{
    KeypadState ks;
    memset(&ks, 0, sizeof(ks));

    UiDriver_InjectButton_under_test(255, &ks);

    TEST_ASSERT_EQUAL(255, ks.button_id);
    TEST_ASSERT_EQUAL(1, ks.button_pressed);
    TEST_ASSERT_EQUAL(1, ks.button_processed);
}

static void test_inject_button_overwrites_previous_state(void)
{
    KeypadState ks;
    ks.button_id = 5;
    ks.button_pressed = false;
    ks.button_processed = 0;
    ks.button_just_pressed = 0;
    ks.press_hold = 1;

    UiDriver_InjectButton_under_test(3, &ks);

    TEST_ASSERT_EQUAL(3, ks.button_id);
    TEST_ASSERT_EQUAL(1, ks.button_pressed);
    TEST_ASSERT_EQUAL(1, ks.button_processed);
    TEST_ASSERT_EQUAL(1, ks.button_just_pressed);
    TEST_ASSERT_EQUAL(0, ks.press_hold);
}

/* -------------------------------------------------------------------------
 * UiDriverEncoderSetDelta tests
 * ------------------------------------------------------------------------- */

static void test_encoder_delta_positive_small(void)
{
    EncoderSelection encSel[ENC_MAX];
    unsigned int counter = 100;
    memset(&encSel, 0, sizeof(encSel));

    UiDriverEncoderSetDelta_under_test(0, 1, encSel, &counter);

    TEST_ASSERT_EQUAL(101, counter);
    TEST_ASSERT_EQUAL(100, encSel[0].value_old);
    TEST_ASSERT_EQUAL(3, encSel[0].de_detent);  /* 4 - 1 */
}

static void test_encoder_delta_negative_small(void)
{
    EncoderSelection encSel[ENC_MAX];
    unsigned int counter = 200;
    memset(&encSel, 0, sizeof(encSel));

    UiDriverEncoderSetDelta_under_test(1, -2, encSel, &counter);

    TEST_ASSERT_EQUAL(198, counter);
    TEST_ASSERT_EQUAL(200, encSel[1].value_old);
    TEST_ASSERT_EQUAL(2, encSel[1].de_detent);  /* 4 - 2 */
}

static void test_encoder_delta_zero(void)
{
    EncoderSelection encSel[ENC_MAX];
    unsigned int counter = 50;
    memset(&encSel, 0, sizeof(encSel));

    UiDriverEncoderSetDelta_under_test(0, 0, encSel, &counter);

    TEST_ASSERT_EQUAL(50, counter);
    TEST_ASSERT_EQUAL(50, encSel[0].value_old);
    TEST_ASSERT_EQUAL(4, encSel[0].de_detent);  /* 4 - 0 */
}

static void test_encoder_delta_large_positive(void)
{
    EncoderSelection encSel[ENC_MAX];
    unsigned int counter = 10;
    memset(&encSel, 0, sizeof(encSel));

    UiDriverEncoderSetDelta_under_test(2, 10, encSel, &counter);

    TEST_ASSERT_EQUAL(20, counter);
    TEST_ASSERT_EQUAL(10, encSel[2].value_old);
    TEST_ASSERT_EQUAL(0, encSel[2].de_detent);  /* >= 4 */
}

static void test_encoder_delta_large_negative(void)
{
    EncoderSelection encSel[ENC_MAX];
    unsigned int counter = 300;
    memset(&encSel, 0, sizeof(encSel));

    UiDriverEncoderSetDelta_under_test(3, -5, encSel, &counter);

    TEST_ASSERT_EQUAL(295, counter);
    TEST_ASSERT_EQUAL(300, encSel[3].value_old);
    TEST_ASSERT_EQUAL(0, encSel[3].de_detent);  /* >= 4 */
}

static void test_encoder_delta_exactly_four(void)
{
    EncoderSelection encSel[ENC_MAX];
    unsigned int counter = 77;
    memset(&encSel, 0, sizeof(encSel));

    UiDriverEncoderSetDelta_under_test(0, 4, encSel, &counter);

    TEST_ASSERT_EQUAL(81, counter);
    TEST_ASSERT_EQUAL(77, encSel[0].value_old);
    TEST_ASSERT_EQUAL(0, encSel[0].de_detent);  /* 4 >= 4 */
}

static void test_encoder_delta_invalid_encId(void)
{
    EncoderSelection encSel[ENC_MAX];
    unsigned int counter = 100;
    memset(&encSel, 0, sizeof(encSel));

    UiDriverEncoderSetDelta_under_test(ENC_MAX, 1, encSel, &counter);

    TEST_ASSERT_EQUAL(100, counter);
    TEST_ASSERT_EQUAL(0, encSel[0].de_detent);
}

static void test_encoder_delta_overflow_encId(void)
{
    EncoderSelection encSel[ENC_MAX];
    unsigned int counter = 100;
    memset(&encSel, 0, sizeof(encSel));

    UiDriverEncoderSetDelta_under_test(99, -1, encSel, &counter);

    TEST_ASSERT_EQUAL(100, counter);
    TEST_ASSERT_EQUAL(0, encSel[0].de_detent);
}

static void test_encoder_delta_multiple_calls_accumulate(void)
{
    EncoderSelection encSel[ENC_MAX];
    unsigned int counter = 0;
    memset(&encSel, 0, sizeof(encSel));

    UiDriverEncoderSetDelta_under_test(0, 1, encSel, &counter);
    TEST_ASSERT_EQUAL(1, counter);
    TEST_ASSERT_EQUAL(3, encSel[0].de_detent);

    UiDriverEncoderSetDelta_under_test(0, 1, encSel, &counter);
    TEST_ASSERT_EQUAL(2, counter);
    TEST_ASSERT_EQUAL(3, encSel[0].de_detent);

    UiDriverEncoderSetDelta_under_test(0, 1, encSel, &counter);
    TEST_ASSERT_EQUAL(3, counter);
    TEST_ASSERT_EQUAL(3, encSel[0].de_detent);
}

static void test_encoder_delta_independent_encoders(void)
{
    EncoderSelection encSel[ENC_MAX];
    unsigned int counter = 0;
    memset(&encSel, 0, sizeof(encSel));

    UiDriverEncoderSetDelta_under_test(0, 1, encSel, &counter);
    UiDriverEncoderSetDelta_under_test(1, 2, encSel, &counter);
    UiDriverEncoderSetDelta_under_test(2, -1, encSel, &counter);

    TEST_ASSERT_EQUAL(2, counter); /* shared mock counter */
    TEST_ASSERT_EQUAL(3, encSel[0].de_detent);
    TEST_ASSERT_EQUAL(2, encSel[1].de_detent);
    TEST_ASSERT_EQUAL(3, encSel[2].de_detent);
    TEST_ASSERT_EQUAL(0, encSel[3].de_detent);
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */

void test_ui_injection_register(void)
{
    printf("--- UI injection (UiDriver_InjectButton / UiDriverEncoderSetDelta) ---\n");
    TEST_REGISTER(test_inject_button_sets_all_fields);
    TEST_REGISTER(test_inject_button_zero_id);
    TEST_REGISTER(test_inject_button_max_id);
    TEST_REGISTER(test_inject_button_overwrites_previous_state);
    TEST_REGISTER(test_encoder_delta_positive_small);
    TEST_REGISTER(test_encoder_delta_negative_small);
    TEST_REGISTER(test_encoder_delta_zero);
    TEST_REGISTER(test_encoder_delta_large_positive);
    TEST_REGISTER(test_encoder_delta_large_negative);
    TEST_REGISTER(test_encoder_delta_exactly_four);
    TEST_REGISTER(test_encoder_delta_invalid_encId);
    TEST_REGISTER(test_encoder_delta_overflow_encId);
    TEST_REGISTER(test_encoder_delta_multiple_calls_accumulate);
    TEST_REGISTER(test_encoder_delta_independent_encoders);
    printf("\n");
}
