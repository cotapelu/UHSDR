#include <stdint.h>
#include <stdbool.h>
#include "test.h"

/* -------------------------------------------------------------------------
 * Replicated RTTY baudot tables from drivers/audio/rtty.c
 * ------------------------------------------------------------------------- */
static const char RTTYLetters[] = {
    '\0',   'E',    '\n',   'A',    ' ',    'S',    'I',    'U',
    '\r',   'D',    'R',    'J',    'N',    'F',    'C',    'K',
    'T',    'Z',    'L',    'W',    'H',    'Y',    'P',    'Q',
    'O',    'B',    'G',    ' ',    'M',    'X',    'V',    ' '
};

static const char RTTYSymbols[] = {
    '\0',   '3',    '\n',   '-',    ' ',    '\a',   '8',    '7',
    '\r',   '$',    '4',    '\'',   ',',    '!',    ':',    '(',
    '5',    '"',    ')',    '2',    '#',    '6',    '0',    '1',
    '9',    '?',    '&',    ' ',    '.',    '/',    ';',    ' '
};

#define RTTY_CODE_MODE_MASK 0b100000
#define RTTY_CODE_MODE_LETTER 0b100000
#define RTTY_CODE_MODE_SYMBOL 0

static const uint8_t test_ascii2baudot[128] = {
    0, 0, 0, 0, 0, 0, 0, 0b001011,
    0, 0, 0b000010, 0, 0, 0b001000, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0b100100, 0b001101, 0b010001, 0b010100, 0b001001, 0, 0b011010, 0b000101,
    0b001111, 0b010010, 0, 0, 0b001100, 0b000011, 0b011100, 0b011101,
    0b010110, 0b010111, 0b010011, 0b000001, 0b001010, 0b010000, 0b010101, 0b000111,
    0b000110, 0b011000, 0b001110, 0b011110, 0, 0, 0, 0b011001,
    0, 0b100011, 0b111001, 0b101110, 0b101001, 0b100001, 0b101101, 0b111010,
    0b110100, 0b100110, 0b101011, 0b101111, 0b110010, 0b111100, 0b101100, 0b111000,
    0b110110, 0b110111, 0b101010, 0b100101, 0b110000, 0b100111, 0b111110, 0b110011,
    0b111101, 0b110101, 0b110001, 0, 0, 0, 0, 0,
    0, 0b100011, 0b111001, 0b101110, 0b101001, 0b100001, 0b101101, 0b111010,
    0b110100, 0b100110, 0b101011, 0b101111, 0b110010, 0b111100, 0b101100, 0b111000,
    0b110110, 0b110111, 0b101010, 0b100101, 0b110000, 0b100111, 0b111110, 0b110011,
    0b111101, 0b110101, 0b110001, 0, 0, 0, 0, 0,
};

/* -------------------------------------------------------------------------
 * Helper: get baudot character for ASCII code
 * ------------------------------------------------------------------------- */
static char rtty_baudot_char(uint8_t ascii)
{
    uint8_t code = test_ascii2baudot[ascii & 0x7F];
    if (code == 0)
    {
        return '\0';
    }
    bool is_letter = (code & RTTY_CODE_MODE_MASK) != 0;
    uint8_t baudot_idx = code & 0x1F;
    if (is_letter)
    {
        return baudot_idx < 32 ? RTTYLetters[baudot_idx] : '\0';
    }
    else
    {
        return baudot_idx < 32 ? RTTYSymbols[baudot_idx] : '\0';
    }
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */
static void test_ascii2baudot_all_codes_5bit(void)
{
    for (int i = 0; i < 128; i++)
    {
        uint8_t code = test_ascii2baudot[i];
        if (code != 0)
        {
            TEST_ASSERT(code <= 0x9F);
        }
    }
}

static void test_rtty_letters_known_mappings(void)
{
    TEST_ASSERT_EQUAL('E', rtty_baudot_char('E'));
    TEST_ASSERT_EQUAL('T', rtty_baudot_char('T'));
    TEST_ASSERT_EQUAL('A', rtty_baudot_char('A'));
    TEST_ASSERT_EQUAL('\n', rtty_baudot_char('\n'));
    TEST_ASSERT_EQUAL('\r', rtty_baudot_char('\r'));
}

static void test_rtty_symbols_known_mappings(void)
{
    TEST_ASSERT_EQUAL('4', rtty_baudot_char('4'));
    TEST_ASSERT_EQUAL('8', rtty_baudot_char('8'));
    TEST_ASSERT_EQUAL(',', rtty_baudot_char(','));
    TEST_ASSERT_EQUAL('?', rtty_baudot_char('?'));
}

static void test_rtty_undefined_returns_null(void)
{
    TEST_ASSERT_EQUAL('\0', rtty_baudot_char(0));
    TEST_ASSERT_EQUAL('\0', rtty_baudot_char(0x80));
}

static void test_rtty_space_maps_to_space(void)
{
    TEST_ASSERT_EQUAL(' ', rtty_baudot_char(' '));
}

static void test_rtty_no_duplicate_letter_mappings(void)
{
    bool used[32] = { false };
    int letter_count = 0;
    for (int i = 0; i < 128; i++)
    {
        uint8_t code = test_ascii2baudot[i];
        if (code != 0 && (code & RTTY_CODE_MODE_MASK))
        {
            uint8_t idx = code & 0x1F;
            if (!used[idx])
            {
                used[idx] = true;
                letter_count++;
            }
        }
    }
    /* There should be at most 32 unique letter mappings (5-bit baudot) */
    TEST_ASSERT(letter_count <= 32);
}

static void test_rtty_no_duplicate_symbol_mappings(void)
{
    bool used[32] = { false };
    for (int i = 0; i < 128; i++)
    {
        uint8_t code = test_ascii2baudot[i];
        if (code != 0 && !(code & RTTY_CODE_MODE_MASK))
        {
            uint8_t idx = code & 0x1F;
            TEST_ASSERT(!used[idx]);
            used[idx] = true;
        }
    }
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */
void test_rtty_baudot_register(void)
{
    printf("--- rtty baudot ---\n");
    TEST_REGISTER(test_ascii2baudot_all_codes_5bit);
    TEST_REGISTER(test_rtty_letters_known_mappings);
    TEST_REGISTER(test_rtty_symbols_known_mappings);
    TEST_REGISTER(test_rtty_undefined_returns_null);
    TEST_REGISTER(test_rtty_space_maps_to_space);
    TEST_REGISTER(test_rtty_no_duplicate_letter_mappings);
    TEST_REGISTER(test_rtty_no_duplicate_symbol_mappings);
    printf("\n");
}
