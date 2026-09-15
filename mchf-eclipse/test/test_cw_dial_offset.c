/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_cw_dial_offset.c                                           **
 **  Description:   Host unit tests for RadioManagement_GetCWDialOffset()           **
 **                 (CW sidetone offset calculation)                                **
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
#define CW_OFFSET_USB_SHIFT     0
#define CW_OFFSET_LSB_SHIFT     1
#define CW_OFFSET_AUTO_SHIFT    2

/* -------------------------------------------------------------------------
 * Minimal transceiver state for testing
 * ------------------------------------------------------------------------- */
typedef struct {
    uint8_t cw_offset_mode;
    bool cw_lsb;
    uint32_t cw_sidetone_freq;
} test_transceiver_state_t;

static test_transceiver_state_t ts;

/* -------------------------------------------------------------------------
 * Reimplemented RadioManagement_GetCWDialOffset (from radio_management.c)
 * ------------------------------------------------------------------------- */
static int32_t RadioManagement_GetCWDialOffset(void)
{
    int32_t retval = 0;

    switch (ts.cw_offset_mode)
    {
    case CW_OFFSET_USB_SHIFT:
    case CW_OFFSET_LSB_SHIFT:
    case CW_OFFSET_AUTO_SHIFT:
        if (ts.cw_lsb)
        {
            retval += (int32_t)ts.cw_sidetone_freq;
        }
        else
        {
            retval -= (int32_t)ts.cw_sidetone_freq;
        }
    }

    return retval;
}

/* -------------------------------------------------------------------------
 * Helper
 * ------------------------------------------------------------------------- */
static void reset_ts(void)
{
    memset(&ts, 0, sizeof(ts));
    ts.cw_sidetone_freq = 0;
}

/* =========================================================================
 * RadioManagement_GetCWDialOffset tests
 * ========================================================================= */

static void test_unknown_mode_returns_zero(void)
{
    reset_ts();
    ts.cw_offset_mode = 255;
    ts.cw_lsb = false;
    ts.cw_sidetone_freq = 800;
    TEST_ASSERT_EQUAL(0, RadioManagement_GetCWDialOffset());
}

static void test_usb_shift_subtracts_sidetone(void)
{
    reset_ts();
    ts.cw_offset_mode = CW_OFFSET_USB_SHIFT;
    ts.cw_lsb = false;
    ts.cw_sidetone_freq = 800;
    TEST_ASSERT_EQUAL(-800, RadioManagement_GetCWDialOffset());
}

static void test_lsb_shift_adds_sidetone(void)
{
    reset_ts();
    ts.cw_offset_mode = CW_OFFSET_LSB_SHIFT;
    ts.cw_lsb = true;
    ts.cw_sidetone_freq = 800;
    TEST_ASSERT_EQUAL(800, RadioManagement_GetCWDialOffset());
}

static void test_auto_shift_usb_subtracts_sidetone(void)
{
    reset_ts();
    ts.cw_offset_mode = CW_OFFSET_AUTO_SHIFT;
    ts.cw_lsb = false;
    ts.cw_sidetone_freq = 600;
    TEST_ASSERT_EQUAL(-600, RadioManagement_GetCWDialOffset());
}

static void test_auto_shift_lsb_adds_sidetone(void)
{
    reset_ts();
    ts.cw_offset_mode = CW_OFFSET_AUTO_SHIFT;
    ts.cw_lsb = true;
    ts.cw_sidetone_freq = 600;
    TEST_ASSERT_EQUAL(600, RadioManagement_GetCWDialOffset());
}

static void test_zero_sidetone_returns_zero(void)
{
    reset_ts();
    ts.cw_offset_mode = CW_OFFSET_USB_SHIFT;
    ts.cw_lsb = false;
    ts.cw_sidetone_freq = 0;
    TEST_ASSERT_EQUAL(0, RadioManagement_GetCWDialOffset());
}

static void test_high_sidetone_frequency(void)
{
    reset_ts();
    ts.cw_offset_mode = CW_OFFSET_LSB_SHIFT;
    ts.cw_lsb = true;
    ts.cw_sidetone_freq = 5000;
    TEST_ASSERT_EQUAL(5000, RadioManagement_GetCWDialOffset());
}

static void test_mode_switch_preserves_sidetone_logic(void)
{
    reset_ts();
    ts.cw_sidetone_freq = 700;

    /* USB mode, cw_lsb=false -> subtract */
    ts.cw_offset_mode = CW_OFFSET_USB_SHIFT;
    ts.cw_lsb = false;
    TEST_ASSERT_EQUAL(-700, RadioManagement_GetCWDialOffset());

    /* LSB mode, cw_lsb=true -> add */
    ts.cw_offset_mode = CW_OFFSET_LSB_SHIFT;
    ts.cw_lsb = true;
    TEST_ASSERT_EQUAL(700, RadioManagement_GetCWDialOffset());

    /* AUTO mode, cw_lsb=false -> subtract */
    ts.cw_offset_mode = CW_OFFSET_AUTO_SHIFT;
    ts.cw_lsb = false;
    TEST_ASSERT_EQUAL(-700, RadioManagement_GetCWDialOffset());
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_cw_dial_offset_register(void)
{
    printf("--- cw_dial_offset ---\n");
    TEST_REGISTER(test_unknown_mode_returns_zero);
    TEST_REGISTER(test_usb_shift_subtracts_sidetone);
    TEST_REGISTER(test_lsb_shift_adds_sidetone);
    TEST_REGISTER(test_auto_shift_usb_subtracts_sidetone);
    TEST_REGISTER(test_auto_shift_lsb_adds_sidetone);
    TEST_REGISTER(test_zero_sidetone_returns_zero);
    TEST_REGISTER(test_high_sidetone_frequency);
    TEST_REGISTER(test_mode_switch_preserves_sidetone_logic);
    printf("\n");
}
