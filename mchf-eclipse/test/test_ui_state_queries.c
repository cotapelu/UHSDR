/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_ui_state_queries.c                                         **
 **  Description:   Host unit tests for ui_driver state query functions             **
 **                 (VFO mode, DSP feature flags)                                  **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Constants matching production code
 * ------------------------------------------------------------------------- */
#define VFO_MEM_MODE_VFO_B      0x40

#define DSP_NR_ENABLE           0x01
#define DSP_NR_POSTAGC_ENABLE   0x02
#define DSP_NOTCH_ENABLE        0x04
#define DSP_NB_ENABLE           0x08
#define DSP_MNOTCH_ENABLE       0x10
#define DSP_MPEAK_ENABLE       0x20

typedef enum {
    VFO_A = 0,
    VFO_B,
    VFO_MAX
} vfo_name_t;

/* -------------------------------------------------------------------------
 * Minimal transceiver state for testing
 * ------------------------------------------------------------------------- */
typedef struct {
    uint32_t vfo_mem_mode;
    struct {
        uint8_t active;
        uint8_t nb_setting;
    } dsp;
} test_transceiver_state_t;

static test_transceiver_state_t ts;

/* -------------------------------------------------------------------------
 * Reimplemented state query functions (from ui_driver.c)
 * ------------------------------------------------------------------------- */
static bool is_vfo_b(void)
{
    return (ts.vfo_mem_mode & VFO_MEM_MODE_VFO_B) != 0;
}

static vfo_name_t get_active_vfo(void)
{
    return is_vfo_b() ? VFO_B : VFO_A;
}

static bool is_dsp_nb(void)
{
    return (ts.dsp.active & DSP_NB_ENABLE) != 0;
}

static bool is_dsp_nb_active(void)
{
    return is_dsp_nb() && (ts.dsp.nb_setting > 0);
}

static bool is_dsp_nr(void)
{
    return (ts.dsp.active & DSP_NR_ENABLE) != 0;
}

static bool is_dsp_nr_postagc(void)
{
    return (ts.dsp.active & DSP_NR_POSTAGC_ENABLE) != 0;
}

static bool is_dsp_notch(void)
{
    return (ts.dsp.active & DSP_NOTCH_ENABLE) != 0;
}

static bool is_dsp_mnotch(void)
{
    return (ts.dsp.active & DSP_MNOTCH_ENABLE) != 0;
}

static bool is_dsp_mpeak(void)
{
    return (ts.dsp.active & DSP_MPEAK_ENABLE) != 0;
}

static uint32_t UiDriver_GetActiveDSPFunctions(void)
{
    return ts.dsp.active & (DSP_NOTCH_ENABLE | DSP_NR_ENABLE | DSP_MNOTCH_ENABLE | DSP_MPEAK_ENABLE);
}

/* -------------------------------------------------------------------------
 * Helper
 * ------------------------------------------------------------------------- */
static void reset_ts(void)
{
    memset(&ts, 0, sizeof(ts));
}

/* =========================================================================
 * VFO mode tests
 * ========================================================================= */

static void test_is_vfo_b_returns_false_when_flag_clear(void)
{
    reset_ts();
    ts.vfo_mem_mode = 0x00;
    TEST_ASSERT_EQUAL(0, is_vfo_b());
}

static void test_is_vfo_b_returns_true_when_flag_set(void)
{
    reset_ts();
    ts.vfo_mem_mode = VFO_MEM_MODE_VFO_B;
    TEST_ASSERT_EQUAL(1, is_vfo_b());
}

static void test_is_vfo_b_ignores_other_bits(void)
{
    reset_ts();
    ts.vfo_mem_mode = 0x80;
    TEST_ASSERT_EQUAL(0, is_vfo_b());
}

static void test_get_active_vfo_returns_a_when_not_b(void)
{
    reset_ts();
    ts.vfo_mem_mode = 0x00;
    TEST_ASSERT_EQUAL(VFO_A, get_active_vfo());
}

static void test_get_active_vfo_returns_b_when_b(void)
{
    reset_ts();
    ts.vfo_mem_mode = VFO_MEM_MODE_VFO_B;
    TEST_ASSERT_EQUAL(VFO_B, get_active_vfo());
}

/* =========================================================================
 * DSP NB tests
 * ========================================================================= */

static void test_is_dsp_nb_false_when_disabled(void)
{
    reset_ts();
    ts.dsp.active = 0x00;
    TEST_ASSERT_EQUAL(0, is_dsp_nb());
}

static void test_is_dsp_nb_true_when_enabled(void)
{
    reset_ts();
    ts.dsp.active = DSP_NB_ENABLE;
    TEST_ASSERT_EQUAL(1, is_dsp_nb());
}

static void test_is_dsp_nb_active_false_when_nb_disabled(void)
{
    reset_ts();
    ts.dsp.active = 0x00;
    ts.dsp.nb_setting = 5;
    TEST_ASSERT_EQUAL(0, is_dsp_nb_active());
}

static void test_is_dsp_nb_active_false_when_nb_setting_zero(void)
{
    reset_ts();
    ts.dsp.active = DSP_NB_ENABLE;
    ts.dsp.nb_setting = 0;
    TEST_ASSERT_EQUAL(0, is_dsp_nb_active());
}

static void test_is_dsp_nb_active_true_when_enabled_and_set(void)
{
    reset_ts();
    ts.dsp.active = DSP_NB_ENABLE;
    ts.dsp.nb_setting = 1;
    TEST_ASSERT_EQUAL(1, is_dsp_nb_active());
}

/* =========================================================================
 * DSP NR tests
 * ========================================================================= */

static void test_is_dsp_nr_false_when_disabled(void)
{
    reset_ts();
    ts.dsp.active = 0x00;
    TEST_ASSERT_EQUAL(0, is_dsp_nr());
}

static void test_is_dsp_nr_true_when_enabled(void)
{
    reset_ts();
    ts.dsp.active = DSP_NR_ENABLE;
    TEST_ASSERT_EQUAL(1, is_dsp_nr());
}

static void test_is_dsp_nr_postagc_false_when_disabled(void)
{
    reset_ts();
    ts.dsp.active = 0x00;
    TEST_ASSERT_EQUAL(0, is_dsp_nr_postagc());
}

static void test_is_dsp_nr_postagc_true_when_enabled(void)
{
    reset_ts();
    ts.dsp.active = DSP_NR_POSTAGC_ENABLE;
    TEST_ASSERT_EQUAL(1, is_dsp_nr_postagc());
}

static void test_is_dsp_nr_and_postagc_can_coexist(void)
{
    reset_ts();
    ts.dsp.active = DSP_NR_ENABLE | DSP_NR_POSTAGC_ENABLE;
    TEST_ASSERT_EQUAL(1, is_dsp_nr());
    TEST_ASSERT_EQUAL(1, is_dsp_nr_postagc());
}

/* =========================================================================
 * DSP Notch/Mnotch/Mpeak tests
 * ========================================================================= */

static void test_is_dsp_notch_false_when_disabled(void)
{
    reset_ts();
    ts.dsp.active = 0x00;
    TEST_ASSERT_EQUAL(0, is_dsp_notch());
}

static void test_is_dsp_notch_true_when_enabled(void)
{
    reset_ts();
    ts.dsp.active = DSP_NOTCH_ENABLE;
    TEST_ASSERT_EQUAL(1, is_dsp_notch());
}

static void test_is_dsp_mnotch_false_when_disabled(void)
{
    reset_ts();
    ts.dsp.active = 0x00;
    TEST_ASSERT_EQUAL(0, is_dsp_mnotch());
}

static void test_is_dsp_mnotch_true_when_enabled(void)
{
    reset_ts();
    ts.dsp.active = DSP_MNOTCH_ENABLE;
    TEST_ASSERT_EQUAL(1, is_dsp_mnotch());
}

static void test_is_dsp_mpeak_false_when_disabled(void)
{
    reset_ts();
    ts.dsp.active = 0x00;
    TEST_ASSERT_EQUAL(0, is_dsp_mpeak());
}

static void test_is_dsp_mpeak_true_when_enabled(void)
{
    reset_ts();
    ts.dsp.active = DSP_MPEAK_ENABLE;
    TEST_ASSERT_EQUAL(1, is_dsp_mpeak());
}

static void test_multiple_dsp_flags_independent(void)
{
    reset_ts();
    ts.dsp.active = DSP_NR_ENABLE | DSP_NOTCH_ENABLE | DSP_MNOTCH_ENABLE | DSP_MPEAK_ENABLE;
    TEST_ASSERT_EQUAL(1, is_dsp_nr());
    TEST_ASSERT_EQUAL(1, is_dsp_notch());
    TEST_ASSERT_EQUAL(1, is_dsp_mnotch());
    TEST_ASSERT_EQUAL(1, is_dsp_mpeak());
    TEST_ASSERT_EQUAL(0, is_dsp_nb());
    TEST_ASSERT_EQUAL(0, is_dsp_nr_postagc());
}

static void test_get_active_dsp_functions_empty(void)
{
    reset_ts();
    ts.dsp.active = 0x00;
    TEST_ASSERT_EQUAL(0, UiDriver_GetActiveDSPFunctions());
}

static void test_get_active_dsp_functions_single(void)
{
    reset_ts();
    ts.dsp.active = DSP_NR_ENABLE;
    TEST_ASSERT_EQUAL(DSP_NR_ENABLE, UiDriver_GetActiveDSPFunctions());
}

static void test_get_active_dsp_functions_multiple(void)
{
    reset_ts();
    ts.dsp.active = DSP_NR_ENABLE | DSP_NOTCH_ENABLE | DSP_MNOTCH_ENABLE | DSP_MPEAK_ENABLE;
    TEST_ASSERT_EQUAL(DSP_NR_ENABLE | DSP_NOTCH_ENABLE | DSP_MNOTCH_ENABLE | DSP_MPEAK_ENABLE, UiDriver_GetActiveDSPFunctions());
}

static void test_get_active_dsp_functions_ignores_nb(void)
{
    reset_ts();
    ts.dsp.active = DSP_NB_ENABLE | DSP_NR_ENABLE;
    TEST_ASSERT_EQUAL(DSP_NR_ENABLE, UiDriver_GetActiveDSPFunctions());
}

static void test_get_active_dsp_functions_ignores_postagc(void)
{
    reset_ts();
    ts.dsp.active = DSP_NR_POSTAGC_ENABLE | DSP_NOTCH_ENABLE;
    TEST_ASSERT_EQUAL(DSP_NOTCH_ENABLE, UiDriver_GetActiveDSPFunctions());
}

static void test_get_active_dsp_functions_all_combination(void)
{
    reset_ts();
    ts.dsp.active = DSP_NR_ENABLE | DSP_NOTCH_ENABLE | DSP_MNOTCH_ENABLE | DSP_MPEAK_ENABLE | DSP_NB_ENABLE | DSP_NR_POSTAGC_ENABLE;
    TEST_ASSERT_EQUAL(DSP_NR_ENABLE | DSP_NOTCH_ENABLE | DSP_MNOTCH_ENABLE | DSP_MPEAK_ENABLE, UiDriver_GetActiveDSPFunctions());
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_ui_state_queries_register(void)
{
    printf("--- ui_state_queries ---\n");
    TEST_REGISTER(test_is_vfo_b_returns_false_when_flag_clear);
    TEST_REGISTER(test_is_vfo_b_returns_true_when_flag_set);
    TEST_REGISTER(test_is_vfo_b_ignores_other_bits);
    TEST_REGISTER(test_get_active_vfo_returns_a_when_not_b);
    TEST_REGISTER(test_get_active_vfo_returns_b_when_b);
    TEST_REGISTER(test_is_dsp_nb_false_when_disabled);
    TEST_REGISTER(test_is_dsp_nb_true_when_enabled);
    TEST_REGISTER(test_is_dsp_nb_active_false_when_nb_disabled);
    TEST_REGISTER(test_is_dsp_nb_active_false_when_nb_setting_zero);
    TEST_REGISTER(test_is_dsp_nb_active_true_when_enabled_and_set);
    TEST_REGISTER(test_is_dsp_nr_false_when_disabled);
    TEST_REGISTER(test_is_dsp_nr_true_when_enabled);
    TEST_REGISTER(test_is_dsp_nr_postagc_false_when_disabled);
    TEST_REGISTER(test_is_dsp_nr_postagc_true_when_enabled);
    TEST_REGISTER(test_is_dsp_nr_and_postagc_can_coexist);
    TEST_REGISTER(test_is_dsp_notch_false_when_disabled);
    TEST_REGISTER(test_is_dsp_notch_true_when_enabled);
    TEST_REGISTER(test_is_dsp_mnotch_false_when_disabled);
    TEST_REGISTER(test_is_dsp_mnotch_true_when_enabled);
    TEST_REGISTER(test_is_dsp_mpeak_false_when_disabled);
    TEST_REGISTER(test_is_dsp_mpeak_true_when_enabled);
    TEST_REGISTER(test_multiple_dsp_flags_independent);
    TEST_REGISTER(test_get_active_dsp_functions_empty);
    TEST_REGISTER(test_get_active_dsp_functions_single);
    TEST_REGISTER(test_get_active_dsp_functions_multiple);
    TEST_REGISTER(test_get_active_dsp_functions_ignores_nb);
    TEST_REGISTER(test_get_active_dsp_functions_ignores_postagc);
    TEST_REGISTER(test_get_active_dsp_functions_all_combination);
    printf("\n");
}
