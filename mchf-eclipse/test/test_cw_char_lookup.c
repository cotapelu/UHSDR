/**
 * @file test_cw_char_lookup.c
 * @brief Unit tests for cw_gen.c character lookup: CwGen_CharacterIdFunc()
 *
 * Reimplements the CW character lookup tables inline to avoid
 * hardware-dependent headers and large external table dependencies.
 */

#include "test.h"
#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Reimplementation of CW character lookup tables (from cw_gen.c)
 * ------------------------------------------------------------------------- */

#define CW_CHAR_CODES 47
#define CW_SIGN_CODES 10

static const uint32_t host_cw_char_codes[CW_CHAR_CODES] =
{
    0,      /* ' ' */
    2,      /* 'E' (. ) */
    3,      /* 'T' (- ) */
    10,     /* 'I' (..) */
    11,     /* 'A' (.-) */
    14,     /* 'N' (-.) */
    15,     /* 'M' (--) */
    42,     /* 'S' (...) */
    43,     /* 'U' (..-) */
    46,     /* 'R' (.-.) */
    47,     /* 'W' (.--) */
    58,     /* 'D' (-..) */
    59,     /* 'K' (-.-) */
    62,     /* 'G' (--.) */
    63,     /* 'O' (---) */
    170,    /* 'H' (....) */
    171,    /* 'V' (...-) */
    174,    /* 'F' (..-.) */
    186,    /* 'L' (.-..) */
    190,    /* 'P' (.--.) */
    191,    /* 'J' (.---) */
    234,    /* 'B' (-...) */
    235,    /* 'X' (-..-) */
    238,    /* 'C' (-.-.) */
    239,    /* 'Y' (-.--) */
    250,    /* 'Z' (--..) */
    251,    /* 'Q' (--.-) */
    682,    /* '5' (.....) */
    683,    /* '4' (....-) */
    687,    /* '3' (...--) */
    703,    /* '2' (..---) */
    767,    /* '1' (.----) */
    938,    /* '6' (-....) */
    939,    /* '=' (-...-) */
    942,    /* '/' (-..-.) */
    1002,   /* '7' (--...) */
    1018,   /* '8' (---..) */
    1022,   /* '9' (----.) */
    1023,   /* '0' (-----) */
    2810,   /* '?' (..--..) */
    2990,   /* '"' (.-..-.) */
    3003,   /* '.' (.-.-.-) */
    3054,   /* '@' (.--.-.) */
    3070,   /* '\'' (.----.) */
    3755,   /* '-' (-....-) */
    4015,   /* ',' (--..--) */
    4074    /* ':' (---...) */
};

static const char host_cw_char_chars[CW_CHAR_CODES] =
{
    ' ', 'E', 'T', 'I', 'A', 'N', 'M', 'S', 'U', 'R',
    'W', 'D', 'K', 'G', 'O', 'H', 'V', 'F', 'L', 'P',
    'J', 'B', 'X', 'C', 'Y', 'Z', 'Q', '5', '4', '3',
    '2', '1', '6', '=', '/', '7', '8', '9', '0', '?',
    '"', '.', '@', '\'', '-', ',', ':'
};

static const uint32_t host_cw_sign_codes[CW_SIGN_CODES] =
{
    187,   /* <AA> */
    750,   /* <AR> */
    746,   /* <AS> */
    61114, /* <CL> */
    955,   /* <CT> */
    43690, /* <HH> */
    958,   /* <KN> */
    3775,  /* <NJ> */
    2747,  /* <SK> */
    686    /* <SN> */
};

static const char host_cw_sign_onechar[CW_SIGN_CODES] =
{
    '^',   /* AA */
    '+',   /* AR */
    '&',   /* AS */
    '{',   /* CL */
    '}',   /* CT */
    0x7F,  /* HH */
    '(',   /* KN */
    '%',   /* NJ */
    '>',   /* SK */
    '~'    /* SN */
};

/* -------------------------------------------------------------------------
 * Reimplementation of CwGen_CharacterIdFunc() for host testing
 * ------------------------------------------------------------------------- */
static uint8_t host_CwGen_CharacterIdFunc(uint32_t code)
{
    uint8_t out = 0xFF;

    if (code == 0)
    {
        out = 0xFE;
    }
    else
    {
        for (int i = 0; i < CW_CHAR_CODES; i++)
        {
            if (host_cw_char_codes[i] == code)
            {
                out = (uint8_t)host_cw_char_chars[i];
                break;
            }
        }

        if (out == 0xFF)
        {
            for (int i = 0; i < CW_SIGN_CODES; i++)
            {
                if (host_cw_sign_codes[i] == code)
                {
                    out = (uint8_t)host_cw_sign_onechar[i];
                    break;
                }
            }
        }
    }

    return out;
}

/* =========================================================================
 * Tests: CwGen_CharacterIdFunc
 * ========================================================================= */

static void test_char_id_space(void)
{
    /* Code 0 is handled as special case returning 0xFE, not ' ' */
    TEST_ASSERT_EQUAL(0xFE, host_CwGen_CharacterIdFunc(0x00));
}

static void test_char_id_e(void)
{
    TEST_ASSERT_EQUAL('E', host_CwGen_CharacterIdFunc(0x02));
}

static void test_char_id_t(void)
{
    TEST_ASSERT_EQUAL('T', host_CwGen_CharacterIdFunc(0x03));
}

static void test_char_id_s(void)
{
    TEST_ASSERT_EQUAL('S', host_CwGen_CharacterIdFunc(0x2A));
}

static void test_char_id_o(void)
{
    TEST_ASSERT_EQUAL('O', host_CwGen_CharacterIdFunc(0x3F));
}

static void test_char_id_5(void)
{
    TEST_ASSERT_EQUAL('5', host_CwGen_CharacterIdFunc(682));
}

static void test_char_id_0(void)
{
    TEST_ASSERT_EQUAL('0', host_CwGen_CharacterIdFunc(1023));
}

static void test_char_id_question_mark(void)
{
    TEST_ASSERT_EQUAL('?', host_CwGen_CharacterIdFunc(2810));
}

static void test_char_id_slash(void)
{
    TEST_ASSERT_EQUAL('/', host_CwGen_CharacterIdFunc(942));
}

static void test_char_id_unknown_returns_error(void)
{
    TEST_ASSERT_EQUAL(0xFF, host_CwGen_CharacterIdFunc(9999));
}

static void test_char_id_zero_returns_special(void)
{
    TEST_ASSERT_EQUAL(0xFE, host_CwGen_CharacterIdFunc(0));
}

static void test_char_id_sign_ar(void)
{
    TEST_ASSERT_EQUAL('+', host_CwGen_CharacterIdFunc(750));
}

static void test_char_id_sign_kn(void)
{
    TEST_ASSERT_EQUAL('(', host_CwGen_CharacterIdFunc(958));
}

static void test_char_id_sign_hh(void)
{
    TEST_ASSERT_EQUAL(0x7F, host_CwGen_CharacterIdFunc(43690));
}

static void test_char_id_sign_sk(void)
{
    TEST_ASSERT_EQUAL('>', host_CwGen_CharacterIdFunc(2747));
}

static void test_char_id_all_chars_found(void)
{
    /* Every entry in the character table should be found (skip code 0 which returns 0xFE) */
    for (int i = 1; i < CW_CHAR_CODES; i++)
    {
        uint8_t result = host_CwGen_CharacterIdFunc(host_cw_char_codes[i]);
        TEST_ASSERT_EQUAL((uint8_t)host_cw_char_chars[i], result);
    }
}

static void test_char_id_all_signs_found(void)
{
    /* Every entry in the sign table should be found */
    for (int i = 0; i < CW_SIGN_CODES; i++)
    {
        uint8_t result = host_CwGen_CharacterIdFunc(host_cw_sign_codes[i]);
        TEST_ASSERT_EQUAL((uint8_t)host_cw_sign_onechar[i], result);
    }
}

static void test_char_id_no_duplicate_codes(void)
{
    /* Ensure no duplicate codes in character table */
    for (int i = 0; i < CW_CHAR_CODES; i++)
    {
        for (int j = i + 1; j < CW_CHAR_CODES; j++)
        {
            TEST_ASSERT(host_cw_char_codes[i] != host_cw_char_codes[j]);
        }
    }
}

static void test_char_id_no_duplicate_sign_codes(void)
{
    /* Ensure no duplicate codes in sign table */
    for (int i = 0; i < CW_SIGN_CODES; i++)
    {
        for (int j = i + 1; j < CW_SIGN_CODES; j++)
        {
            TEST_ASSERT(host_cw_sign_codes[i] != host_cw_sign_codes[j]);
        }
    }
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_cw_char_lookup_register(void)
{
    printf("--- cw_gen character lookup ---\n");

    /* Known character mappings */
    TEST_REGISTER(test_char_id_space);
    TEST_REGISTER(test_char_id_e);
    TEST_REGISTER(test_char_id_t);
    TEST_REGISTER(test_char_id_s);
    TEST_REGISTER(test_char_id_o);
    TEST_REGISTER(test_char_id_5);
    TEST_REGISTER(test_char_id_0);
    TEST_REGISTER(test_char_id_question_mark);
    TEST_REGISTER(test_char_id_slash);

    /* Error and special cases */
    TEST_REGISTER(test_char_id_unknown_returns_error);
    TEST_REGISTER(test_char_id_zero_returns_special);

    /* Sign (prosign) mappings */
    TEST_REGISTER(test_char_id_sign_ar);
    TEST_REGISTER(test_char_id_sign_kn);
    TEST_REGISTER(test_char_id_sign_hh);
    TEST_REGISTER(test_char_id_sign_sk);

    /* Table integrity */
    TEST_REGISTER(test_char_id_all_chars_found);
    TEST_REGISTER(test_char_id_all_signs_found);
    TEST_REGISTER(test_char_id_no_duplicate_codes);
    TEST_REGISTER(test_char_id_no_duplicate_sign_codes);

    printf("\n");
}
