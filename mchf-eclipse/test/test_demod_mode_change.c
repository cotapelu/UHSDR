/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                     Host unit tests: demod mode change detection                **
 **                                                                                 **
 **--------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_demod_mode_change.c                                        **
 **  Description:   Tests for UiDriver_IsDemodModeChange()                          **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"

/* -------------------------------------------------------------------------
 * Reimplemented logic under test
 * ------------------------------------------------------------------------- */

#define DEMOD_USB   0
#define DEMOD_LSB   1
#define DEMOD_CW    2
#define DEMOD_AM    3
#define DEMOD_SAM   4
#define DEMOD_FM    5
#define DEMOD_DIGI  6

typedef struct
{
    uint8_t dmod_mode;
    uint8_t digital_mode;
} ui_driver_mode_t;

typedef struct
{
    ui_driver_mode_t ui_driver_state;
    uint8_t dmod_mode;
    uint8_t digital_mode;
} TestState;

static bool UiDriver_IsDemodModeChange_under_test(TestState *state)
{
    bool retval = (state->dmod_mode != state->ui_driver_state.dmod_mode);
    retval |= state->dmod_mode == DEMOD_DIGI && state->digital_mode != state->ui_driver_state.digital_mode;
    return retval;
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

static void test_no_change_when_same(void)
{
    TestState state;
    memset(&state, 0, sizeof(state));
    state.ui_driver_state.dmod_mode = DEMOD_USB;
    state.ui_driver_state.digital_mode = 0;
    state.dmod_mode = DEMOD_USB;
    state.digital_mode = 0;

    bool result = UiDriver_IsDemodModeChange_under_test(&state);

    TEST_ASSERT_EQUAL(0, result);
}

static void test_change_when_mode_differs(void)
{
    TestState state;
    memset(&state, 0, sizeof(state));
    state.ui_driver_state.dmod_mode = DEMOD_LSB;
    state.ui_driver_state.digital_mode = 0;
    state.dmod_mode = DEMOD_USB;
    state.digital_mode = 0;

    bool result = UiDriver_IsDemodModeChange_under_test(&state);

    TEST_ASSERT_EQUAL(1, result);
}

static void test_digi_mode_same_digital_no_change(void)
{
    TestState state;
    memset(&state, 0, sizeof(state));
    state.ui_driver_state.dmod_mode = DEMOD_DIGI;
    state.ui_driver_state.digital_mode = 1;
    state.dmod_mode = DEMOD_DIGI;
    state.digital_mode = 1;

    bool result = UiDriver_IsDemodModeChange_under_test(&state);

    TEST_ASSERT_EQUAL(0, result);
}

static void test_digi_mode_digital_change(void)
{
    TestState state;
    memset(&state, 0, sizeof(state));
    state.ui_driver_state.dmod_mode = DEMOD_DIGI;
    state.ui_driver_state.digital_mode = 1;
    state.dmod_mode = DEMOD_DIGI;
    state.digital_mode = 2;

    bool result = UiDriver_IsDemodModeChange_under_test(&state);

    TEST_ASSERT_EQUAL(1, result);
}

static void test_digi_mode_switching_to_analog_ignores_digital(void)
{
    TestState state;
    memset(&state, 0, sizeof(state));
    state.ui_driver_state.dmod_mode = DEMOD_DIGI;
    state.ui_driver_state.digital_mode = 1;
    state.dmod_mode = DEMOD_FM;
    state.digital_mode = 99; /* different digital_mode, but not DIGI */

    bool result = UiDriver_IsDemodModeChange_under_test(&state);

    TEST_ASSERT_EQUAL(1, result); /* mode changed */
}

static void test_non_digi_mode_ignores_digital_change(void)
{
    TestState state;
    memset(&state, 0, sizeof(state));
    state.ui_driver_state.dmod_mode = DEMOD_FM;
    state.ui_driver_state.digital_mode = 1;
    state.dmod_mode = DEMOD_FM;
    state.digital_mode = 2; /* digital_mode differs, but not DIGI */

    bool result = UiDriver_IsDemodModeChange_under_test(&state);

    TEST_ASSERT_EQUAL(0, result);
}

static void test_analog_to_digi_mode_change(void)
{
    TestState state;
    memset(&state, 0, sizeof(state));
    state.ui_driver_state.dmod_mode = DEMOD_AM;
    state.ui_driver_state.digital_mode = 0;
    state.dmod_mode = DEMOD_DIGI;
    state.digital_mode = 1;

    bool result = UiDriver_IsDemodModeChange_under_test(&state);

    TEST_ASSERT_EQUAL(1, result); /* mode changed */
}

static void test_digi_to_analog_mode_change(void)
{
    TestState state;
    memset(&state, 0, sizeof(state));
    state.ui_driver_state.dmod_mode = DEMOD_DIGI;
    state.ui_driver_state.digital_mode = 1;
    state.dmod_mode = DEMOD_CW;
    state.digital_mode = 1;

    bool result = UiDriver_IsDemodModeChange_under_test(&state);

    TEST_ASSERT_EQUAL(1, result); /* mode changed */
}

static void test_all_analog_modes_no_digital_effect(void)
{
    uint8_t modes[] = { DEMOD_USB, DEMOD_LSB, DEMOD_CW, DEMOD_AM, DEMOD_SAM, DEMOD_FM };
    for (int i = 0; i < 6; i++)
    {
        TestState state;
        memset(&state, 0, sizeof(state));
        state.ui_driver_state.dmod_mode = modes[i];
        state.ui_driver_state.digital_mode = 1;
        state.dmod_mode = modes[i];
        state.digital_mode = 2; /* different digital_mode */

        bool result = UiDriver_IsDemodModeChange_under_test(&state);
        TEST_ASSERT_EQUAL(0, result);
    }
}

static void test_digi_mode_all_digital_changes_detected(void)
{
    TestState state;
    memset(&state, 0, sizeof(state));
    state.ui_driver_state.dmod_mode = DEMOD_DIGI;
    state.ui_driver_state.digital_mode = 0;
    state.dmod_mode = DEMOD_DIGI;

    for (int d = 1; d < 5; d++)
    {
        state.digital_mode = d;
        bool result = UiDriver_IsDemodModeChange_under_test(&state);
        TEST_ASSERT_EQUAL(1, result);
    }
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */

void test_demod_mode_change_register(void)
{
    printf("--- Demod mode change detection (UiDriver_IsDemodModeChange) ---\n");
    TEST_REGISTER(test_no_change_when_same);
    TEST_REGISTER(test_change_when_mode_differs);
    TEST_REGISTER(test_digi_mode_same_digital_no_change);
    TEST_REGISTER(test_digi_mode_digital_change);
    TEST_REGISTER(test_digi_mode_switching_to_analog_ignores_digital);
    TEST_REGISTER(test_non_digi_mode_ignores_digital_change);
    TEST_REGISTER(test_analog_to_digi_mode_change);
    TEST_REGISTER(test_digi_to_analog_mode_change);
    TEST_REGISTER(test_all_analog_modes_no_digital_effect);
    TEST_REGISTER(test_digi_mode_all_digital_changes_detected);
    printf("\n");
}
