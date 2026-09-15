/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_cat_driver.c                                               **
 **  Description:   Host unit tests for cat_driver pure helper functions            **
 **  Last Modified: 2026-08-28                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include <stdint.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Reimplementation of cat_driver.c pure helpers for host testing
 * ------------------------------------------------------------------------- */
uint8_t limit_4bits(uint32_t in)
{
    return in > 15 ? 15 : (uint8_t)in;
}

uint8_t CatDriver_Clone_Checksum(uint8_t* buf, size_t len)
{
    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++)
    {
        sum += buf[i];
    }
    return sum;
}

/* -------------------------------------------------------------------------
 * Tests: limit_4bits
 * ------------------------------------------------------------------------- */
static void test_limit_4bits_under_15(void)
{
    TEST_ASSERT_EQUAL(5, limit_4bits(5));
}

static void test_limit_4bits_at_15(void)
{
    TEST_ASSERT_EQUAL(15, limit_4bits(15));
}

static void test_limit_4bits_over_15(void)
{
    TEST_ASSERT_EQUAL(15, limit_4bits(16));
}

static void test_limit_4bits_large_value(void)
{
    TEST_ASSERT_EQUAL(15, limit_4bits(255));
    TEST_ASSERT_EQUAL(15, limit_4bits(65535));
    TEST_ASSERT_EQUAL(15, limit_4bits(1000000));
}

static void test_limit_4bits_zero(void)
{
    TEST_ASSERT_EQUAL(0, limit_4bits(0));
}

/* -------------------------------------------------------------------------
 * Tests: CatDriver_Clone_Checksum
 * ------------------------------------------------------------------------- */
static void test_checksum_empty_buffer(void)
{
    uint8_t buf[1];
    TEST_ASSERT_EQUAL(0, CatDriver_Clone_Checksum(buf, 0));
}

static void test_checksum_single_byte(void)
{
    uint8_t buf[1] = {0x42};
    TEST_ASSERT_EQUAL(0x42, CatDriver_Clone_Checksum(buf, 1));
}

static void test_checksum_multiple_bytes(void)
{
    uint8_t buf[3] = {0x01, 0x02, 0x03};
    TEST_ASSERT_EQUAL(0x06, CatDriver_Clone_Checksum(buf, 3));
}

static void test_checksum_overflow_wraps(void)
{
    uint8_t buf[2] = {0xFF, 0x01};
    TEST_ASSERT_EQUAL(0x00, CatDriver_Clone_Checksum(buf, 2)); /* 255 + 1 = 256 = 0 (uint8_t) */
}

static void test_checksum_all_zeros(void)
{
    uint8_t buf[4] = {0, 0, 0, 0};
    TEST_ASSERT_EQUAL(0, CatDriver_Clone_Checksum(buf, 4));
}

static void test_checksum_all_ones(void)
{
    uint8_t buf[4] = {1, 1, 1, 1};
    TEST_ASSERT_EQUAL(4, CatDriver_Clone_Checksum(buf, 4));
}

static void test_checksum_known_vector(void)
{
    uint8_t buf[5] = {0x10, 0x20, 0x30, 0x40, 0x50};
    TEST_ASSERT_EQUAL(0xF0, CatDriver_Clone_Checksum(buf, 5)); /* 16+32+48+64+80 = 240 */
}

/* -------------------------------------------------------------------------
 * Registration
 * ------------------------------------------------------------------------- */
void test_cat_driver_register(void)
{
    printf("--- cat_driver ---\n");
    TEST_REGISTER(test_limit_4bits_under_15);
    TEST_REGISTER(test_limit_4bits_at_15);
    TEST_REGISTER(test_limit_4bits_over_15);
    TEST_REGISTER(test_limit_4bits_large_value);
    TEST_REGISTER(test_limit_4bits_zero);
    TEST_REGISTER(test_checksum_empty_buffer);
    TEST_REGISTER(test_checksum_single_byte);
    TEST_REGISTER(test_checksum_multiple_bytes);
    TEST_REGISTER(test_checksum_overflow_wraps);
    TEST_REGISTER(test_checksum_all_zeros);
    TEST_REGISTER(test_checksum_all_ones);
    TEST_REGISTER(test_checksum_known_vector);
    printf("\n");
}
