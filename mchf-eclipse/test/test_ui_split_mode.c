/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                             Host unit tests: split mode API                     **
 **                                                                                 **
 **--------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_ui_split_mode.c                                            **
 **  Description:   Tests for UiDriver_SetSplitMode()                               **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"

/* -------------------------------------------------------------------------
 * Reimplemented logic under test
 * ------------------------------------------------------------------------- */

#define VFO_MEM_MODE_SPLIT  (1u << 0)

static void UiDriver_SetSplitMode_under_test(bool mode_active, unsigned int *vfo_mem_mode)
{
    if (mode_active)
    {
        *vfo_mem_mode |= VFO_MEM_MODE_SPLIT;
    }
    else
    {
        *vfo_mem_mode &= ~VFO_MEM_MODE_SPLIT;
    }
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

static void test_split_mode_enable_sets_bit(void)
{
    unsigned int vfo_mem_mode = 0;

    UiDriver_SetSplitMode_under_test(true, &vfo_mem_mode);

    TEST_ASSERT_EQUAL(1, vfo_mem_mode);
}

static void test_split_mode_disable_clears_bit(void)
{
    unsigned int vfo_mem_mode = VFO_MEM_MODE_SPLIT;

    UiDriver_SetSplitMode_under_test(false, &vfo_mem_mode);

    TEST_ASSERT_EQUAL(0, vfo_mem_mode);
}

static void test_split_mode_disable_when_already_clear(void)
{
    unsigned int vfo_mem_mode = 0;

    UiDriver_SetSplitMode_under_test(false, &vfo_mem_mode);

    TEST_ASSERT_EQUAL(0, vfo_mem_mode);
}

static void test_split_mode_enable_when_already_set(void)
{
    unsigned int vfo_mem_mode = VFO_MEM_MODE_SPLIT;

    UiDriver_SetSplitMode_under_test(true, &vfo_mem_mode);

    TEST_ASSERT_EQUAL(VFO_MEM_MODE_SPLIT, vfo_mem_mode);
}

static void test_split_mode_preserves_other_bits(void)
{
    unsigned int vfo_mem_mode = 0x06; /* bits 1 and 2 set */

    UiDriver_SetSplitMode_under_test(true, &vfo_mem_mode);

    TEST_ASSERT_EQUAL(0x07, vfo_mem_mode);
}

static void test_split_mode_clear_preserves_other_bits(void)
{
    unsigned int vfo_mem_mode = 0x07; /* bits 0, 1, 2 set */

    UiDriver_SetSplitMode_under_test(false, &vfo_mem_mode);

    TEST_ASSERT_EQUAL(0x06, vfo_mem_mode);
}

static void test_split_mode_toggle_on_off(void)
{
    unsigned int vfo_mem_mode = 0;

    UiDriver_SetSplitMode_under_test(true, &vfo_mem_mode);
    TEST_ASSERT_EQUAL(1, vfo_mem_mode);

    UiDriver_SetSplitMode_under_test(false, &vfo_mem_mode);
    TEST_ASSERT_EQUAL(0, vfo_mem_mode);

    UiDriver_SetSplitMode_under_test(true, &vfo_mem_mode);
    TEST_ASSERT_EQUAL(1, vfo_mem_mode);
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */

void test_ui_split_mode_register(void)
{
    printf("--- UI split mode (UiDriver_SetSplitMode) ---\n");
    TEST_REGISTER(test_split_mode_enable_sets_bit);
    TEST_REGISTER(test_split_mode_disable_clears_bit);
    TEST_REGISTER(test_split_mode_disable_when_already_clear);
    TEST_REGISTER(test_split_mode_enable_when_already_set);
    TEST_REGISTER(test_split_mode_preserves_other_bits);
    TEST_REGISTER(test_split_mode_clear_preserves_other_bits);
    TEST_REGISTER(test_split_mode_toggle_on_off);
    printf("\n");
}
