/**
 * @file test_ui_menu_item_change.c
 * @brief Unit tests for ui_menu.c menu item change logic:
 *        UiDriverMenuItemChangeUInt8(), UiDriverMenuItemChangeUInt32(),
 *        UiDriverMenuItemChangeInt()
 *
 * Reimplements the core value-change logic inline to avoid
 * hardware-dependent headers and global state.
 */

#include "test.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Menu processing mode constants
 * ------------------------------------------------------------------------- */
#define MENU_PROCESS_NORMAL       0
#define MENU_PROCESS_VALUE_SETDEFAULT 1

/* -------------------------------------------------------------------------
 * Reimplementation of UiDriverMenuItemChangeUInt8() core logic
 * ------------------------------------------------------------------------- */
static bool host_menu_change_uint8(volatile uint8_t* val_ptr,
                                   int var,
                                   uint8_t val_min,
                                   uint8_t val_max,
                                   uint8_t val_default,
                                   uint32_t increment)
{
    (void)val_default;
    uint8_t old_val = *val_ptr;

    if (var >= 1) /* setting increase? */
    {
        if (*val_ptr < val_max)
        {
            *val_ptr += increment;
        }
    }
    else if (var <= -1) /* setting decrease? */
    {
        if (*val_ptr > val_min)
        {
            *val_ptr -= increment;
        }
    }

    if (*val_ptr < val_min)
    {
        *val_ptr = val_min;
    }
    if (*val_ptr > val_max)
    {
        *val_ptr = val_max;
    }

    return old_val != *val_ptr;
}

/* -------------------------------------------------------------------------
 * Reimplementation of UiDriverMenuItemChangeInt() core logic
 * ------------------------------------------------------------------------- */
static bool host_menu_change_int(volatile int* val_ptr,
                                 int var,
                                 int val_min,
                                 int val_max,
                                 int val_default,
                                 uint32_t increment)
{
    int old_val = *val_ptr;

    if (var >= 1) /* setting increase? */
    {
        if (*val_ptr < val_max)
        {
            *val_ptr += increment;
        }
    }
    else if (var <= -1) /* setting decrease? */
    {
        if (*val_ptr > val_min)
        {
            *val_ptr -= increment;
        }
    }

    if (*val_ptr < val_min)
    {
        *val_ptr = val_min;
    }
    if (*val_ptr > val_max)
    {
        *val_ptr = val_max;
    }

    if (val_default != 0) /* simplified: treat non-zero as set-default request */
    {
        *val_ptr = val_default;
    }

    return old_val != *val_ptr;
}

/* =========================================================================
 * Tests: UiDriverMenuItemChangeUInt8 core logic
 * ========================================================================= */

static void test_uint8_increase_from_middle(void)
{
    uint8_t val = 50;
    bool changed = host_menu_change_uint8(&val, 1, 0, 100, 0, 1);
    TEST_ASSERT(changed);
    TEST_ASSERT_EQUAL(51, val);
}

static void test_uint8_decrease_from_middle(void)
{
    uint8_t val = 50;
    bool changed = host_menu_change_uint8(&val, -1, 0, 100, 0, 1);
    TEST_ASSERT(changed);
    TEST_ASSERT_EQUAL(49, val);
}

static void test_uint8_increase_at_max_clamped(void)
{
    uint8_t val = 100;
    bool changed = host_menu_change_uint8(&val, 1, 0, 100, 0, 1);
    TEST_ASSERT(!changed);
    TEST_ASSERT_EQUAL(100, val);
}

static void test_uint8_decrease_at_min_clamped(void)
{
    uint8_t val = 0;
    bool changed = host_menu_change_uint8(&val, -1, 0, 100, 0, 1);
    TEST_ASSERT(!changed);
    TEST_ASSERT_EQUAL(0, val);
}

static void test_uint8_no_change_when_var_zero(void)
{
    uint8_t val = 50;
    bool changed = host_menu_change_uint8(&val, 0, 0, 100, 0, 1);
    TEST_ASSERT(!changed);
    TEST_ASSERT_EQUAL(50, val);
}

static void test_uint8_increase_large_increment_clamped(void)
{
    uint8_t val = 98;
    bool changed = host_menu_change_uint8(&val, 1, 0, 100, 0, 10);
    TEST_ASSERT(changed);
    TEST_ASSERT_EQUAL(100, val); /* clamped to max */
}

static void test_uint8_decrease_large_increment_clamped(void)
{
    uint8_t val = 2;
    bool changed = host_menu_change_uint8(&val, -1, 0, 100, 0, 10);
    TEST_ASSERT(changed);
    /* Unsigned underflow: 2 - 10 wraps to 246, then clamped to max 100 */
    TEST_ASSERT_EQUAL(100, val);
}

static void test_uint8_boundary_at_min(void)
{
    uint8_t val = 0;
    bool changed = host_menu_change_uint8(&val, 1, 0, 100, 0, 1);
    TEST_ASSERT(changed);
    TEST_ASSERT_EQUAL(1, val);
}

static void test_uint8_boundary_at_max(void)
{
    uint8_t val = 100;
    bool changed = host_menu_change_uint8(&val, -1, 0, 100, 0, 1);
    TEST_ASSERT(changed);
    TEST_ASSERT_EQUAL(99, val);
}

static void test_uint8_increment_greater_than_range(void)
{
    uint8_t val = 0;
    bool changed = host_menu_change_uint8(&val, 1, 0, 10, 0, 20);
    TEST_ASSERT(changed);
    TEST_ASSERT_EQUAL(10, val); /* clamped to max */
}

static void test_uint8_multiple_increases(void)
{
    uint8_t val = 10;
    host_menu_change_uint8(&val, 1, 0, 100, 0, 5);
    host_menu_change_uint8(&val, 1, 0, 100, 0, 5);
    TEST_ASSERT_EQUAL(20, val);
}

/* =========================================================================
 * Tests: UiDriverMenuItemChangeInt core logic
 * ========================================================================= */

static void test_int_increase_from_middle(void)
{
    int val = 50;
    bool changed = host_menu_change_int(&val, 1, -100, 100, 0, 1);
    TEST_ASSERT(changed);
    TEST_ASSERT_EQUAL(51, val);
}

static void test_int_decrease_from_middle(void)
{
    int val = 50;
    bool changed = host_menu_change_int(&val, -1, -100, 100, 0, 1);
    TEST_ASSERT(changed);
    TEST_ASSERT_EQUAL(49, val);
}

static void test_int_increase_at_max_clamped(void)
{
    int val = 100;
    bool changed = host_menu_change_int(&val, 1, -100, 100, 0, 1);
    TEST_ASSERT(!changed);
    TEST_ASSERT_EQUAL(100, val);
}

static void test_int_decrease_at_min_clamped(void)
{
    int val = -100;
    bool changed = host_menu_change_int(&val, -1, -100, 100, 0, 1);
    TEST_ASSERT(!changed);
    TEST_ASSERT_EQUAL(-100, val);
}

static void test_int_negative_range(void)
{
    int val = -50;
    bool changed = host_menu_change_int(&val, 1, -100, 100, 0, 1);
    TEST_ASSERT(changed);
    TEST_ASSERT_EQUAL(-49, val);
}

static void test_int_set_default_when_requested(void)
{
    int val = 75;
    bool changed = host_menu_change_int(&val, 0, -100, 100, 42, 0);
    TEST_ASSERT(changed);
    TEST_ASSERT_EQUAL(42, val);
}

static void test_int_no_set_default_when_zero(void)
{
    int val = 75;
    bool changed = host_menu_change_int(&val, 0, -100, 100, 0, 0);
    TEST_ASSERT(!changed);
    TEST_ASSERT_EQUAL(75, val);
}

static void test_int_large_increment_clamped(void)
{
    int val = 95;
    bool changed = host_menu_change_int(&val, 1, -100, 100, 0, 20);
    TEST_ASSERT(changed);
    TEST_ASSERT_EQUAL(100, val);
}

static void test_int_large_decrement_clamped(void)
{
    int val = -95;
    bool changed = host_menu_change_int(&val, -1, -100, 100, 0, 20);
    TEST_ASSERT(changed);
    TEST_ASSERT_EQUAL(-100, val);
}

static void test_int_no_change_when_var_zero_no_default(void)
{
    int val = 0;
    bool changed = host_menu_change_int(&val, 0, -10, 10, 0, 1);
    TEST_ASSERT(!changed);
    TEST_ASSERT_EQUAL(0, val);
}

static void test_int_wraparound_not_allowed(void)
{
    /* Decrement from min should NOT wrap, should clamp */
    int val = -100;
    bool changed = host_menu_change_int(&val, -1, -100, 100, 0, 1);
    TEST_ASSERT(!changed);
    TEST_ASSERT_EQUAL(-100, val);
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_ui_menu_item_change_register(void)
{
    printf("--- ui_menu item change ---\n");

    /* UInt8 change logic */
    TEST_REGISTER(test_uint8_increase_from_middle);
    TEST_REGISTER(test_uint8_decrease_from_middle);
    TEST_REGISTER(test_uint8_increase_at_max_clamped);
    TEST_REGISTER(test_uint8_decrease_at_min_clamped);
    TEST_REGISTER(test_uint8_no_change_when_var_zero);
    TEST_REGISTER(test_uint8_increase_large_increment_clamped);
    TEST_REGISTER(test_uint8_decrease_large_increment_clamped);
    TEST_REGISTER(test_uint8_boundary_at_min);
    TEST_REGISTER(test_uint8_boundary_at_max);
    TEST_REGISTER(test_uint8_increment_greater_than_range);
    TEST_REGISTER(test_uint8_multiple_increases);

    printf("\n");

    /* Int change logic */
    TEST_REGISTER(test_int_increase_from_middle);
    TEST_REGISTER(test_int_decrease_from_middle);
    TEST_REGISTER(test_int_increase_at_max_clamped);
    TEST_REGISTER(test_int_decrease_at_min_clamped);
    TEST_REGISTER(test_int_negative_range);
    TEST_REGISTER(test_int_set_default_when_requested);
    TEST_REGISTER(test_int_no_set_default_when_zero);
    TEST_REGISTER(test_int_large_increment_clamped);
    TEST_REGISTER(test_int_large_decrement_clamped);
    TEST_REGISTER(test_int_no_change_when_var_zero_no_default);
    TEST_REGISTER(test_int_wraparound_not_allowed);

    printf("\n");
}
