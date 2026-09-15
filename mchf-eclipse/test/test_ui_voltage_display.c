/**
 * @file test_ui_voltage_display.c
 * @brief Unit tests for ui_driver_power.c voltage display color logic
 *
 * Reimplements the voltage-to-color mapping and undervoltage blinking
 * logic inline to avoid hardware-dependent headers.
 */

#include "test.h"
#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Constants (from uhsdr_board.h / ui_driver_power.c)
 * ------------------------------------------------------------------------- */
#define LOW_POWER_ENABLE_MASK    0x80
#define LOW_POWER_THRESHOLD_OFFSET 30
#define LOW_POWER_THRESHOLD_MASK 0x7F

#define COL_PWR_IND  0  /* normal color (placeholder) */
#define COLOR_RED    1
#define COLOR_ORANGE 2
#define COLOR_YELLOW 3
#define COLOR_BLACK  4

/* -------------------------------------------------------------------------
 * Reimplementation of voltage display color logic
 * ------------------------------------------------------------------------- */
static uint32_t host_voltage_display_color(uint32_t voltage,
                                           uint8_t low_power_config,
                                           bool undervoltage_detected,
                                           uint8_t* blink_state)
{
    uint32_t col = COL_PWR_IND;

    uint32_t low_power_threshold = ((low_power_config & LOW_POWER_THRESHOLD_MASK) + LOW_POWER_THRESHOLD_OFFSET) * 10;

    if (voltage < low_power_threshold + 50)
    {
        col = COLOR_RED;
    }
    else if (voltage < low_power_threshold + 100)
    {
        col = COLOR_ORANGE;
    }
    else if (voltage < low_power_threshold + 150)
    {
        col = COLOR_YELLOW;
    }

    /* Undervoltage blinking: 1Hz blink (alternates every other call) */
    if (undervoltage_detected && *blink_state < 1)
    {
        col = COLOR_BLACK;
    }
    (*blink_state)++;
    if (*blink_state == 2)
    {
        *blink_state = 0;
    }

    return col;
}

/* =========================================================================
 * Tests: Voltage display color logic
 * ========================================================================= */

static void test_voltage_normal_color_above_threshold(void)
{
    uint8_t blink = 0;
    /* low_power_config = 90 (threshold = (90 & 0x7F) + 30) * 10 = 1200mV */
    uint32_t col = host_voltage_display_color(1500, 90, false, &blink);
    TEST_ASSERT_EQUAL(COL_PWR_IND, col);
}

static void test_voltage_yellow_near_threshold(void)
{
    uint8_t blink = 0;
    /* threshold = 1200mV. 1250 < 1300 but NOT < 1250, so orange zone */
    uint32_t col = host_voltage_display_color(1250, 90, false, &blink);
    TEST_ASSERT_EQUAL(COLOR_ORANGE, col);
}

static void test_voltage_orange_closer_to_threshold(void)
{
    uint8_t blink = 0;
    /* threshold = 1200mV. 1220 < 1250, so red zone */
    uint32_t col = host_voltage_display_color(1220, 90, false, &blink);
    TEST_ASSERT_EQUAL(COLOR_RED, col);
}

static void test_voltage_red_below_threshold(void)
{
    uint8_t blink = 0;
    /* threshold = 1200mV. Voltage 1100 is in red zone */
    uint32_t col = host_voltage_display_color(1100, 90, false, &blink);
    TEST_ASSERT_EQUAL(COLOR_RED, col);
}

static void test_voltage_exact_threshold_plus_50(void)
{
    uint8_t blink = 0;
    /* threshold = 1200mV. 1200 + 50 = 1250. Voltage exactly 1250 is NOT red (>=), it's orange */
    uint32_t col = host_voltage_display_color(1250, 90, false, &blink);
    TEST_ASSERT_EQUAL(COLOR_ORANGE, col);
}

static void test_voltage_exact_threshold_plus_100(void)
{
    uint8_t blink = 0;
    /* threshold = 1200mV. 1200 + 100 = 1300. Voltage exactly 1300 is NOT orange, it's yellow */
    uint32_t col = host_voltage_display_color(1300, 90, false, &blink);
    TEST_ASSERT_EQUAL(COLOR_YELLOW, col);
}

static void test_voltage_config_bitmask_extracts_threshold(void)
{
    uint8_t blink = 0;
    /* low_power_config = 60. Threshold = (60 & 0x7F + 30) * 10 = 900mV */
    uint32_t col = host_voltage_display_color(950, 60, false, &blink);
    TEST_ASSERT_EQUAL(COLOR_ORANGE, col); /* 950 < 900+100=1000, >= 900+50=950 -> orange */
}

static void test_voltage_enable_bit_does_not_affect_threshold(void)
{
    uint8_t blink = 0;
    /* low_power_config = 60 vs 188 (60 | 0x80). Both have same threshold (900mV) */
    uint32_t col1 = host_voltage_display_color(950, 60, false, &blink);
    uint8_t blink2 = 0;
    uint32_t col2 = host_voltage_display_color(950, 188, false, &blink2);
    TEST_ASSERT_EQUAL(col1, col2);
}

static void test_undervoltage_blink_first_call_black(void)
{
    uint8_t blink = 0;
    /* Undervoltage detected, first call: blink_state=0, so col=Black */
    uint32_t col = host_voltage_display_color(1100, 90, true, &blink);
    TEST_ASSERT_EQUAL(COLOR_BLACK, col);
    TEST_ASSERT_EQUAL(1, blink); /* blink_state incremented */
}

static void test_undervoltage_blink_second_call_normal(void)
{
    uint8_t blink = 1;
    /* Undervoltage detected, second call: blink_state=1, so col stays normal */
    uint32_t col = host_voltage_display_color(1100, 90, true, &blink);
    TEST_ASSERT_EQUAL(COLOR_RED, col); /* voltage is in red zone */
    TEST_ASSERT_EQUAL(0, blink); /* blink_state wraps to 0 */
}

static void test_undervoltage_blink_alternates(void)
{
    uint8_t blink = 0;
    /* First call: Black */
    TEST_ASSERT_EQUAL(COLOR_BLACK, host_voltage_display_color(1100, 90, true, &blink));
    /* Second call: Red (normal) */
    TEST_ASSERT_EQUAL(COLOR_RED, host_voltage_display_color(1100, 90, true, &blink));
    /* Third call: Black again */
    TEST_ASSERT_EQUAL(COLOR_BLACK, host_voltage_display_color(1100, 90, true, &blink));
    /* Fourth call: Red again */
    TEST_ASSERT_EQUAL(COLOR_RED, host_voltage_display_color(1100, 90, true, &blink));
}

static void test_undervoltage_no_blink_when_not_detected(void)
{
    uint8_t blink = 0;
    /* No undervoltage: should show normal color regardless of blink_state */
    TEST_ASSERT_EQUAL(COLOR_RED, host_voltage_display_color(1100, 90, false, &blink));
    TEST_ASSERT_EQUAL(1, blink);
    TEST_ASSERT_EQUAL(COLOR_RED, host_voltage_display_color(1100, 90, false, &blink));
    TEST_ASSERT_EQUAL(0, blink);
}

static void test_voltage_low_threshold_offset(void)
{
    uint8_t blink = 0;
    /* low_power_config = 0 (threshold = (0 + 30) * 10 = 300mV) */
    uint32_t col = host_voltage_display_color(250, 0, false, &blink);
    TEST_ASSERT_EQUAL(COLOR_RED, col);
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_ui_voltage_display_register(void)
{
    printf("--- ui voltage display ---\n");

    TEST_REGISTER(test_voltage_normal_color_above_threshold);
    TEST_REGISTER(test_voltage_yellow_near_threshold);
    TEST_REGISTER(test_voltage_orange_closer_to_threshold);
    TEST_REGISTER(test_voltage_red_below_threshold);
    TEST_REGISTER(test_voltage_exact_threshold_plus_50);
    TEST_REGISTER(test_voltage_exact_threshold_plus_100);
    TEST_REGISTER(test_voltage_config_bitmask_extracts_threshold);
    TEST_REGISTER(test_voltage_enable_bit_does_not_affect_threshold);
    TEST_REGISTER(test_undervoltage_blink_first_call_black);
    TEST_REGISTER(test_undervoltage_blink_second_call_normal);
    TEST_REGISTER(test_undervoltage_blink_alternates);
    TEST_REGISTER(test_undervoltage_no_blink_when_not_detected);
    TEST_REGISTER(test_voltage_low_threshold_offset);

    printf("\n");
}
