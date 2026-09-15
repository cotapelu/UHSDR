/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_uhsdr_keypad.c                                             **
 **  Description:   Host unit tests for keypad button/key state queries             **
 **                 (hardware-independent logic only)                               **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Reimplemented keypad state logic (avoids hardware-dependent headers)
 * ------------------------------------------------------------------------- */
#define BUTTON_NUM 18

static uint32_t keypad_button_states;
static uint32_t keypad_key_states;

static void keypad_state_reset(void)
{
    keypad_button_states = 0;
    keypad_key_states = 0;
}

static void keypad_set_button(uint32_t button_num, bool pressed)
{
    if (pressed) {
        keypad_button_states |= (1U << button_num);
    } else {
        keypad_button_states &= ~(1U << button_num);
    }
}

static void keypad_set_key(uint32_t key_num, bool pressed)
{
    if (pressed) {
        keypad_key_states |= (1U << key_num);
    } else {
        keypad_key_states &= ~(1U << key_num);
    }
}

static bool Keypad_IsButtonPressed(uint32_t button_num)
{
    return ((1U << button_num) & keypad_button_states) != 0;
}

static bool Keypad_IsAnyButtonPressed(void)
{
    return keypad_button_states != 0;
}

static bool Keypad_IsKeyPressed(uint32_t key_num)
{
    return ((1U << key_num) & keypad_key_states) != 0;
}

static bool Keypad_IsAnyKeyPressed(void)
{
    return keypad_key_states != 0;
}

static uint32_t Keypad_KeyStates(void)
{
    return keypad_key_states;
}

static uint32_t Keypad_ButtonStates(void)
{
    return keypad_button_states;
}

static const char* Keypad_GetLabelOfButton(uint32_t id_button)
{
    static const char* buttons[BUTTON_NUM] = {
        "M2", "G3", "G2", "Band-", "G4", "M3", "Step-", "Step+",
        "M1", "F3", "F1", "F2", "F4", "Band+", "F5", "G1",
        "Power", "Touch"
    };
    if (id_button < BUTTON_NUM) {
        return buttons[id_button];
    }
    return "Unknown";
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */
static void test_any_button_pressed_false_when_none_set(void)
{
    keypad_state_reset();
    TEST_ASSERT_EQUAL(false, Keypad_IsAnyButtonPressed());
}

static void test_any_button_pressed_true_when_one_set(void)
{
    keypad_state_reset();
    keypad_set_button(3, true);
    TEST_ASSERT_EQUAL(true, Keypad_IsAnyButtonPressed());
}

static void test_specific_button_pressed_true(void)
{
    keypad_state_reset();
    keypad_set_button(5, true);
    TEST_ASSERT_EQUAL(true, Keypad_IsButtonPressed(5));
}

static void test_specific_button_pressed_false_for_other(void)
{
    keypad_state_reset();
    keypad_set_button(5, true);
    TEST_ASSERT_EQUAL(false, Keypad_IsButtonPressed(3));
}

static void test_button_states_returns_mask(void)
{
    keypad_state_reset();
    keypad_set_button(0, true);
    keypad_set_button(2, true);
    keypad_set_button(7, true);
    TEST_ASSERT_EQUAL((1U << 0) | (1U << 2) | (1U << 7), Keypad_ButtonStates());
}

static void test_any_key_pressed_false_when_none_set(void)
{
    keypad_state_reset();
    TEST_ASSERT_EQUAL(false, Keypad_IsAnyKeyPressed());
}

static void test_any_key_pressed_true_when_one_set(void)
{
    keypad_state_reset();
    keypad_set_key(4, true);
    TEST_ASSERT_EQUAL(true, Keypad_IsAnyKeyPressed());
}

static void test_specific_key_pressed_true(void)
{
    keypad_state_reset();
    keypad_set_key(9, true);
    TEST_ASSERT_EQUAL(true, Keypad_IsKeyPressed(9));
}

static void test_specific_key_pressed_false_for_other(void)
{
    keypad_state_reset();
    keypad_set_key(9, true);
    TEST_ASSERT_EQUAL(false, Keypad_IsKeyPressed(1));
}

static void test_key_states_returns_mask(void)
{
    keypad_state_reset();
    keypad_set_key(0, true);
    keypad_set_key(3, true);
    keypad_set_key(15, true);
    TEST_ASSERT_EQUAL((1U << 0) | (1U << 3) | (1U << 15), Keypad_KeyStates());
}

static void test_get_label_of_button_valid_range(void)
{
    TEST_ASSERT_EQUAL_STRING("M2", Keypad_GetLabelOfButton(0));
    TEST_ASSERT_EQUAL_STRING("Band+", Keypad_GetLabelOfButton(13));
    TEST_ASSERT_EQUAL_STRING("Power", Keypad_GetLabelOfButton(16));
    TEST_ASSERT_EQUAL_STRING("Touch", Keypad_GetLabelOfButton(17));
}

static void test_get_label_of_button_out_of_range(void)
{
    TEST_ASSERT_EQUAL_STRING("Unknown", Keypad_GetLabelOfButton(18));
    TEST_ASSERT_EQUAL_STRING("Unknown", Keypad_GetLabelOfButton(100));
}

static void test_button_and_key_states_independent(void)
{
    keypad_state_reset();
    keypad_set_button(2, true);
    keypad_set_key(2, true);
    keypad_set_button(5, true);
    /* key 5 remains clear */
    TEST_ASSERT_EQUAL(true, Keypad_IsButtonPressed(2));
    TEST_ASSERT_EQUAL(true, Keypad_IsButtonPressed(5));
    TEST_ASSERT_EQUAL(false, Keypad_IsKeyPressed(5));
    TEST_ASSERT_EQUAL(true, Keypad_IsKeyPressed(2));
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */
void test_uhsdr_keypad_register(void)
{
    printf("--- uhsdr_keypad ---\n");
    TEST_REGISTER(test_any_button_pressed_false_when_none_set);
    TEST_REGISTER(test_any_button_pressed_true_when_one_set);
    TEST_REGISTER(test_specific_button_pressed_true);
    TEST_REGISTER(test_specific_button_pressed_false_for_other);
    TEST_REGISTER(test_button_states_returns_mask);
    TEST_REGISTER(test_any_key_pressed_false_when_none_set);
    TEST_REGISTER(test_any_key_pressed_true_when_one_set);
    TEST_REGISTER(test_specific_key_pressed_true);
    TEST_REGISTER(test_specific_key_pressed_false_for_other);
    TEST_REGISTER(test_key_states_returns_mask);
    TEST_REGISTER(test_get_label_of_button_valid_range);
    TEST_REGISTER(test_get_label_of_button_out_of_range);
    TEST_REGISTER(test_button_and_key_states_independent);
    printf("\n");
}
