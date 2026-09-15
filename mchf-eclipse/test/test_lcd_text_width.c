/**
 * @file test_lcd_text_width.c
 * @brief Unit tests for ui_lcd_hy28.c UiLcdHy28_TextWidthLen()
 *
 * Reimplements the text width calculation logic inline to avoid
 * hardware-dependent headers and global state.
 */

#include "test.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Replicated sFONT struct from ui_lcd_hy28_fonts.h
 * ------------------------------------------------------------------------- */

typedef struct {
    const void *table;
    uint8_t  Width;
    uint8_t  Height;
    uint16_t maxCode;
    uint8_t BitCount;
    uint8_t firstCode;
    uint8_t  Spacing;
} sFONT;

/* -------------------------------------------------------------------------
 * Reimplementation of UiLcdHy28_TextWidthLen()
 * ------------------------------------------------------------------------- */

static uint16_t host_text_width_len(const char *str_start, uint16_t len, const sFONT *cf)
{
    int8_t char_width = (cf->Width + cf->Spacing) - ((cf->Width == 8 && cf->Height == 8) ? 1 : 0);
    return (str_start != NULL) ? (len * char_width) : 0;
}

static uint16_t host_text_width(const char *str_start, const sFONT *cf)
{
    return (str_start != NULL) ? host_text_width_len(str_start, strlen(str_start), cf) : 0;
}

/* =========================================================================
 * Tests: UiLcdHy28_TextWidthLen / UiLcdHy28_TextWidth
 * ========================================================================= */

static void test_text_width_null_returns_zero(void)
{
    sFONT font = { .Width = 8, .Height = 12, .Spacing = 1 };
    TEST_ASSERT_EQUAL(0, host_text_width_len(NULL, 5, &font));
    TEST_ASSERT_EQUAL(0, host_text_width(NULL, &font));
}

static void test_text_width_standard_font(void)
{
    /* Standard font: 8x12, spacing 1 */
    sFONT font = { .Width = 8, .Height = 12, .Spacing = 1 };
    /* char_width = (8 + 1) - 0 = 9 */
    TEST_ASSERT_EQUAL(27, host_text_width_len("ABC", 3, &font));
}

static void test_text_width_8x8_font_special_case(void)
{
    /* 8x8 font has special case: subtract 1 */
    sFONT font = { .Width = 8, .Height = 8, .Spacing = 1 };
    /* char_width = (8 + 1) - 1 = 8 */
    TEST_ASSERT_EQUAL(24, host_text_width_len("ABC", 3, &font));
}

static void test_text_width_8x8_no_spacing(void)
{
    sFONT font = { .Width = 8, .Height = 8, .Spacing = 0 };
    /* char_width = (8 + 0) - 1 = 7 */
    TEST_ASSERT_EQUAL(21, host_text_width_len("ABC", 3, &font));
}

static void test_text_width_16x16_font(void)
{
    sFONT font = { .Width = 16, .Height = 16, .Spacing = 2 };
    /* char_width = (16 + 2) - 0 = 18 */
    TEST_ASSERT_EQUAL(36, host_text_width_len("AB", 2, &font));
}

static void test_text_width_zero_length(void)
{
    sFONT font = { .Width = 8, .Height = 12, .Spacing = 1 };
    TEST_ASSERT_EQUAL(0, host_text_width_len("ABC", 0, &font));
}

static void test_text_width_large_font(void)
{
    sFONT font = { .Width = 24, .Height = 32, .Spacing = 4 };
    /* char_width = (24 + 4) - 0 = 28 */
    TEST_ASSERT_EQUAL(140, host_text_width_len("Hello", 5, &font));
}

static void test_text_width_monospace_consistency(void)
{
    /* All characters have same width in monospace font */
    sFONT font = { .Width = 12, .Height = 12, .Spacing = 1 };
    uint16_t width1 = host_text_width_len("A", 1, &font);
    uint16_t width2 = host_text_width_len("B", 1, &font);
    uint16_t width5 = host_text_width_len("ABCDE", 5, &font);
    TEST_ASSERT_EQUAL(width1, width2);
    TEST_ASSERT_EQUAL(width1 * 5, width5);
}

static void test_text_width_8x8_not_12x12(void)
{
    /* Verify 8x8 special case only applies when both width and height are 8 */
    sFONT font_8x8 = { .Width = 8, .Height = 8, .Spacing = 1 };
    sFONT font_8x12 = { .Width = 8, .Height = 12, .Spacing = 1 };
    sFONT font_12x8 = { .Width = 12, .Height = 8, .Spacing = 1 };

    TEST_ASSERT_EQUAL(8, (host_text_width_len("A", 1, &font_8x8)));
    TEST_ASSERT_EQUAL(9, (host_text_width_len("A", 1, &font_8x12)));
    TEST_ASSERT_EQUAL(13, (host_text_width_len("A", 1, &font_12x8)));
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_lcd_text_width_register(void)
{
    printf("--- lcd_text_width ---\n");

    TEST_REGISTER(test_text_width_null_returns_zero);
    TEST_REGISTER(test_text_width_standard_font);
    TEST_REGISTER(test_text_width_8x8_font_special_case);
    TEST_REGISTER(test_text_width_8x8_no_spacing);
    TEST_REGISTER(test_text_width_16x16_font);
    TEST_REGISTER(test_text_width_zero_length);
    TEST_REGISTER(test_text_width_large_font);
    TEST_REGISTER(test_text_width_monospace_consistency);
    TEST_REGISTER(test_text_width_8x8_not_12x12);

    printf("\n");
}
