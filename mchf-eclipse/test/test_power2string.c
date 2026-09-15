/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                     Host unit tests: power-to-string conversion                 **
 **                                                                                 **
 **--------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_power2string.c                                             **
 **  Description:   Tests for UiDriver_Power2String()                               **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"

/* -------------------------------------------------------------------------
 * Reimplemented logic under test
 * ------------------------------------------------------------------------- */

static void UiDriver_Power2String_under_test(char* txt, size_t txt_len, uint32_t power_mW)
{
    if (power_mW == 0)
    {
        snprintf(txt, txt_len, "FULL");
    }
    else if (power_mW < 100)
    {
        snprintf(txt, txt_len, "%lumW", (unsigned long)power_mW);
    }
    else if (power_mW < 1000)
    {
        snprintf(txt, txt_len, "0.%luW", (unsigned long)(power_mW / 100));
    }
    else
    {
        snprintf(txt, txt_len, "%luW", (unsigned long)(power_mW / 1000));
    }
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

static void test_power_zero_returns_full(void)
{
    char txt[32];
    memset(txt, 0, sizeof(txt));

    UiDriver_Power2String_under_test(txt, sizeof(txt), 0);

    TEST_ASSERT(strcmp(txt, "FULL") == 0);
}

static void test_power_1mw(void)
{
    char txt[32];
    memset(txt, 0, sizeof(txt));

    UiDriver_Power2String_under_test(txt, sizeof(txt), 1);

    TEST_ASSERT(strcmp(txt, "1mW") == 0);
}

static void test_power_99mw(void)
{
    char txt[32];
    memset(txt, 0, sizeof(txt));

    UiDriver_Power2String_under_test(txt, sizeof(txt), 99);

    TEST_ASSERT(strcmp(txt, "99mW") == 0);
}

static void test_power_100mw_boundary(void)
{
    char txt[32];
    memset(txt, 0, sizeof(txt));

    UiDriver_Power2String_under_test(txt, sizeof(txt), 100);

    TEST_ASSERT(strcmp(txt, "0.1W") == 0);
}

static void test_power_500mw(void)
{
    char txt[32];
    memset(txt, 0, sizeof(txt));

    UiDriver_Power2String_under_test(txt, sizeof(txt), 500);

    TEST_ASSERT(strcmp(txt, "0.5W") == 0);
}

static void test_power_999mw(void)
{
    char txt[32];
    memset(txt, 0, sizeof(txt));

    UiDriver_Power2String_under_test(txt, sizeof(txt), 999);

    TEST_ASSERT(strcmp(txt, "0.9W") == 0);
}

static void test_power_1w_boundary(void)
{
    char txt[32];
    memset(txt, 0, sizeof(txt));

    UiDriver_Power2String_under_test(txt, sizeof(txt), 1000);

    TEST_ASSERT(strcmp(txt, "1W") == 0);
}

static void test_power_1500mw(void)
{
    char txt[32];
    memset(txt, 0, sizeof(txt));

    UiDriver_Power2String_under_test(txt, sizeof(txt), 1500);

    TEST_ASSERT(strcmp(txt, "1W") == 0);
}

static void test_power_2500mw(void)
{
    char txt[32];
    memset(txt, 0, sizeof(txt));

    UiDriver_Power2String_under_test(txt, sizeof(txt), 2500);

    TEST_ASSERT(strcmp(txt, "2W") == 0);
}

static void test_power_5w(void)
{
    char txt[32];
    memset(txt, 0, sizeof(txt));

    UiDriver_Power2String_under_test(txt, sizeof(txt), 5000);

    TEST_ASSERT(strcmp(txt, "5W") == 0);
}

static void test_power_10w(void)
{
    char txt[32];
    memset(txt, 0, sizeof(txt));

    UiDriver_Power2String_under_test(txt, sizeof(txt), 10000);

    TEST_ASSERT(strcmp(txt, "10W") == 0);
}

static void test_power_max_uint32(void)
{
    char txt[32];
    memset(txt, 0, sizeof(txt));

    UiDriver_Power2String_under_test(txt, sizeof(txt), UINT32_MAX);

    /* Should not overflow or crash */
    TEST_ASSERT(txt[0] != '\0');
}

static void test_power_small_buffer_truncates(void)
{
    char txt[8]; /* intentionally small */
    memset(txt, 0, sizeof(txt));

    UiDriver_Power2String_under_test(txt, sizeof(txt), 12345);

    /* snprintf should null-terminate even in small buffer */
    TEST_ASSERT_EQUAL(0, txt[sizeof(txt) - 1]);
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */

void test_power2string_register(void)
{
    printf("--- Power-to-string (UiDriver_Power2String) ---\n");
    TEST_REGISTER(test_power_zero_returns_full);
    TEST_REGISTER(test_power_1mw);
    TEST_REGISTER(test_power_99mw);
    TEST_REGISTER(test_power_100mw_boundary);
    TEST_REGISTER(test_power_500mw);
    TEST_REGISTER(test_power_999mw);
    TEST_REGISTER(test_power_1w_boundary);
    TEST_REGISTER(test_power_1500mw);
    TEST_REGISTER(test_power_2500mw);
    TEST_REGISTER(test_power_5w);
    TEST_REGISTER(test_power_10w);
    TEST_REGISTER(test_power_max_uint32);
    TEST_REGISTER(test_power_small_buffer_truncates);
    printf("\n");
}
