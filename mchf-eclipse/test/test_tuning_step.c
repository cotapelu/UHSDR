/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                             Host unit tests: tuning step change                 **
 **                                                                                 **
 **--------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_tuning_step.c                                              **
 **  Description:   Tests for UiDriver_ChangeTuningStep()                           **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"

/* -------------------------------------------------------------------------
 * Reimplemented logic under test
 * ------------------------------------------------------------------------- */

#define T_STEP_10HZ_IDX    0
#define T_STEP_100HZ_IDX   1
#define T_STEP_500HZ_IDX   2
#define T_STEP_1KHZ_IDX    3
#define T_STEP_5KHZ_IDX    4
#define T_STEP_9KHZ_IDX    5
#define T_STEP_10KHZ_IDX   6
#define T_STEP_100KHZ_IDX  7
#define T_STEP_1MHZ_IDX    8
#define T_STEP_10MHZ_IDX   9
#define T_STEP_MAX_STEPS   10

typedef struct
{
    int32_t selected_idx;
    unsigned int tune_old;
    bool xvtr_adjust_flag;
    bool xverter_mode;
    int32_t tuning_step;
} DriverField;

typedef struct
{
    DriverField df;
    unsigned int dmod_mode;
    unsigned int filter;
} TuningStepState;

static int32_t UiDriver_ChangeTuningStep_under_test(TuningStepState *state, bool is_up)
{
    const int8_t step = is_up ? +1 : -1;

    int32_t idx = state->df.selected_idx;
    int8_t idx_limit = T_STEP_MAX_STEPS;

    if ((!state->df.xvtr_adjust_flag) && (!state->df.xverter_mode))
    {
        idx_limit = T_STEP_1MHZ_IDX;
    }

    idx = (idx + step + idx_limit) % idx_limit;

    if (idx == T_STEP_9KHZ_IDX && (state->df.tune_old > 1600001))
    {
        idx += step;
    }

    state->df.selected_idx = idx;
    state->df.tuning_step = idx;
    return idx;
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

static void test_step_up_from_first(void)
{
    TuningStepState state;
    memset(&state, 0, sizeof(state));
    state.df.selected_idx = T_STEP_10HZ_IDX;
    state.df.xvtr_adjust_flag = false;
    state.df.xverter_mode = false;

    int32_t result = UiDriver_ChangeTuningStep_under_test(&state, true);

    TEST_ASSERT_EQUAL(T_STEP_100HZ_IDX, result);
    TEST_ASSERT_EQUAL(T_STEP_100HZ_IDX, state.df.selected_idx);
}

static void test_step_down_from_first_wraps(void)
{
    TuningStepState state;
    memset(&state, 0, sizeof(state));
    state.df.selected_idx = T_STEP_10HZ_IDX;
    state.df.xvtr_adjust_flag = false;
    state.df.xverter_mode = false;

    int32_t result = UiDriver_ChangeTuningStep_under_test(&state, false);

    TEST_ASSERT_EQUAL(T_STEP_100KHZ_IDX, result); /* wraps from 0 to 7 */
    TEST_ASSERT_EQUAL(T_STEP_100KHZ_IDX, state.df.selected_idx);
}

static void test_step_up_from_last_wraps(void)
{
    TuningStepState state;
    memset(&state, 0, sizeof(state));
    state.df.selected_idx = T_STEP_100KHZ_IDX;
    state.df.xvtr_adjust_flag = false;
    state.df.xverter_mode = false;

    int32_t result = UiDriver_ChangeTuningStep_under_test(&state, true);

    TEST_ASSERT_EQUAL(T_STEP_10HZ_IDX, result); /* wraps from 7 to 0 */
    TEST_ASSERT_EQUAL(T_STEP_10HZ_IDX, state.df.selected_idx);
}

static void test_step_down_from_last(void)
{
    TuningStepState state;
    memset(&state, 0, sizeof(state));
    state.df.selected_idx = T_STEP_100KHZ_IDX;
    state.df.xvtr_adjust_flag = false;
    state.df.xverter_mode = false;

    int32_t result = UiDriver_ChangeTuningStep_under_test(&state, false);

    TEST_ASSERT_EQUAL(T_STEP_10KHZ_IDX, result);
    TEST_ASSERT_EQUAL(T_STEP_10KHZ_IDX, state.df.selected_idx);
}

static void test_9khz_skipped_when_high_freq(void)
{
    TuningStepState state;
    memset(&state, 0, sizeof(state));
    state.df.selected_idx = T_STEP_5KHZ_IDX; /* 5kHz */
    state.df.xvtr_adjust_flag = false;
    state.df.xverter_mode = false;
    state.df.tune_old = 2000000; /* > 1600001 */

    int32_t result = UiDriver_ChangeTuningStep_under_test(&state, true);

    TEST_ASSERT_EQUAL(T_STEP_10KHZ_IDX, result); /* skipped 9kHz */
    TEST_ASSERT_EQUAL(T_STEP_10KHZ_IDX, state.df.selected_idx);
}

static void test_9khz_not_skipped_when_low_freq(void)
{
    TuningStepState state;
    memset(&state, 0, sizeof(state));
    state.df.selected_idx = T_STEP_5KHZ_IDX; /* 5kHz */
    state.df.xvtr_adjust_flag = false;
    state.df.xverter_mode = false;
    state.df.tune_old = 1000000; /* <= 1600001 */

    int32_t result = UiDriver_ChangeTuningStep_under_test(&state, true);

    TEST_ASSERT_EQUAL(T_STEP_9KHZ_IDX, result); /* kept 9kHz */
    TEST_ASSERT_EQUAL(T_STEP_9KHZ_IDX, state.df.selected_idx);
}

static void test_9khz_skipped_down_when_high_freq(void)
{
    TuningStepState state;
    memset(&state, 0, sizeof(state));
    state.df.selected_idx = T_STEP_10KHZ_IDX;
    state.df.xvtr_adjust_flag = false;
    state.df.xverter_mode = false;
    state.df.tune_old = 5000000;

    int32_t result = UiDriver_ChangeTuningStep_under_test(&state, false);

    TEST_ASSERT_EQUAL(T_STEP_5KHZ_IDX, result); /* skipped 9kHz going down */
    TEST_ASSERT_EQUAL(T_STEP_5KHZ_IDX, state.df.selected_idx);
}

static void test_transverter_mode_uses_full_range(void)
{
    TuningStepState state;
    memset(&state, 0, sizeof(state));
    state.df.selected_idx = T_STEP_1MHZ_IDX; /* 8 */
    state.df.xvtr_adjust_flag = false;
    state.df.xverter_mode = true; /* transverter mode ON */

    int32_t result = UiDriver_ChangeTuningStep_under_test(&state, true);

    TEST_ASSERT_EQUAL(T_STEP_10MHZ_IDX, result); /* can go to 9 */
    TEST_ASSERT_EQUAL(T_STEP_10MHZ_IDX, state.df.selected_idx);
}

static void test_transverter_mode_wraps_full_range(void)
{
    TuningStepState state;
    memset(&state, 0, sizeof(state));
    state.df.selected_idx = T_STEP_10MHZ_IDX; /* 9 */
    state.df.xvtr_adjust_flag = false;
    state.df.xverter_mode = true;

    int32_t result = UiDriver_ChangeTuningStep_under_test(&state, true);

    TEST_ASSERT_EQUAL(T_STEP_10HZ_IDX, result); /* wraps to 0 */
    TEST_ASSERT_EQUAL(T_STEP_10HZ_IDX, state.df.selected_idx);
}

static void test_xvtr_adjust_flag_uses_full_range(void)
{
    TuningStepState state;
    memset(&state, 0, sizeof(state));
    state.df.selected_idx = T_STEP_10MHZ_IDX;
    state.df.xvtr_adjust_flag = true; /* xvtr adjust ON */
    state.df.xverter_mode = false;

    int32_t result = UiDriver_ChangeTuningStep_under_test(&state, true);

    TEST_ASSERT_EQUAL(T_STEP_10HZ_IDX, result);
    TEST_ASSERT_EQUAL(T_STEP_10HZ_IDX, state.df.selected_idx);
}

static void test_step_sequence_forward(void)
{
    TuningStepState state;
    memset(&state, 0, sizeof(state));
    state.df.selected_idx = T_STEP_10HZ_IDX;
    state.df.xvtr_adjust_flag = false;
    state.df.xverter_mode = false;

    /* Walk through all normal steps: 0,1,2,3,4,6,7 (skip 5=9kHz) */
    UiDriver_ChangeTuningStep_under_test(&state, true);
    TEST_ASSERT_EQUAL(T_STEP_100HZ_IDX, state.df.selected_idx);

    UiDriver_ChangeTuningStep_under_test(&state, true);
    TEST_ASSERT_EQUAL(T_STEP_500HZ_IDX, state.df.selected_idx);

    UiDriver_ChangeTuningStep_under_test(&state, true);
    TEST_ASSERT_EQUAL(T_STEP_1KHZ_IDX, state.df.selected_idx);

    UiDriver_ChangeTuningStep_under_test(&state, true);
    TEST_ASSERT_EQUAL(T_STEP_5KHZ_IDX, state.df.selected_idx);

    /* 9kHz should be skipped at high freq */
    state.df.tune_old = 2000000;
    UiDriver_ChangeTuningStep_under_test(&state, true);
    TEST_ASSERT_EQUAL(T_STEP_10KHZ_IDX, state.df.selected_idx);

    UiDriver_ChangeTuningStep_under_test(&state, true);
    TEST_ASSERT_EQUAL(T_STEP_100KHZ_IDX, state.df.selected_idx);
}

static void test_step_sequence_backward(void)
{
    TuningStepState state;
    memset(&state, 0, sizeof(state));
    state.df.selected_idx = T_STEP_100KHZ_IDX;
    state.df.xvtr_adjust_flag = false;
    state.df.xverter_mode = false;

    /* Walk backward: 7,6,4,3,2,1,0 (skip 5=9kHz) */
    state.df.tune_old = 2000000;
    UiDriver_ChangeTuningStep_under_test(&state, false);
    TEST_ASSERT_EQUAL(T_STEP_10KHZ_IDX, state.df.selected_idx);

    UiDriver_ChangeTuningStep_under_test(&state, false);
    TEST_ASSERT_EQUAL(T_STEP_5KHZ_IDX, state.df.selected_idx);

    UiDriver_ChangeTuningStep_under_test(&state, false);
    TEST_ASSERT_EQUAL(T_STEP_1KHZ_IDX, state.df.selected_idx);

    UiDriver_ChangeTuningStep_under_test(&state, false);
    TEST_ASSERT_EQUAL(T_STEP_500HZ_IDX, state.df.selected_idx);

    UiDriver_ChangeTuningStep_under_test(&state, false);
    TEST_ASSERT_EQUAL(T_STEP_100HZ_IDX, state.df.selected_idx);

    UiDriver_ChangeTuningStep_under_test(&state, false);
    TEST_ASSERT_EQUAL(T_STEP_10HZ_IDX, state.df.selected_idx);
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */

void test_tuning_step_register(void)
{
    printf("--- Tuning step change (UiDriver_ChangeTuningStep) ---\n");
    TEST_REGISTER(test_step_up_from_first);
    TEST_REGISTER(test_step_down_from_first_wraps);
    TEST_REGISTER(test_step_up_from_last_wraps);
    TEST_REGISTER(test_step_down_from_last);
    TEST_REGISTER(test_9khz_skipped_when_high_freq);
    TEST_REGISTER(test_9khz_not_skipped_when_low_freq);
    TEST_REGISTER(test_9khz_skipped_down_when_high_freq);
    TEST_REGISTER(test_transverter_mode_uses_full_range);
    TEST_REGISTER(test_transverter_mode_wraps_full_range);
    TEST_REGISTER(test_xvtr_adjust_flag_uses_full_range);
    TEST_REGISTER(test_step_sequence_forward);
    TEST_REGISTER(test_step_sequence_backward);
    printf("\n");
}
