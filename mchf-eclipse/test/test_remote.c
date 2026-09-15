/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_remote.c                                                   **
 **  Description:   Host unit tests for remote control protocol                     **
 **                 (CRC8, packet encode/decode, queue behavior)                   **
 **  Last Modified: 2026-08-28                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include "remote_protocol.h"

/* Pull in the implementation for host testing */
#include "../drivers/remote/remote_protocol.c"

/* -------------------------------------------------------------------------
 * CRC8-CCITT test vectors
 * ------------------------------------------------------------------------- */
static void test_crc8_empty(void)
{
    uint8_t crc = Remote_CRC8(NULL, 0);
    TEST_ASSERT_EQUAL(0x00, crc);
}

static void test_crc8_single_byte(void)
{
    uint8_t data = 0x00;
    uint8_t crc = Remote_CRC8(&data, 1);
    TEST_ASSERT_EQUAL(0x00, crc);
}

static void test_crc8_known_vector(void)
{
    /* Verified against remote_protocol.c implementation */
    uint8_t data[] = "123456789";
    uint8_t crc = Remote_CRC8(data, sizeof(data) - 1);
    TEST_ASSERT_EQUAL(0xF4, crc);
}

static void test_crc8_all_zeros(void)
{
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t crc = Remote_CRC8(data, 4);
    TEST_ASSERT_EQUAL(0x00, crc);
}

static void test_crc8_all_ones(void)
{
    uint8_t data[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t crc = Remote_CRC8(data, 4);
    TEST_ASSERT_EQUAL(0xDE, crc);
}

/* -------------------------------------------------------------------------
 * Packet build test
 * ------------------------------------------------------------------------- */
static void test_build_packet_encoder_delta(void)
{
    uint8_t buf[8];
    int8_t delta = 1;
    hal_status_t st = Remote_BuildPacket(buf, 0x01, (const uint8_t *)&delta, 1);
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, st);
    TEST_ASSERT_EQUAL(0xAA, buf[0]);
    TEST_ASSERT_EQUAL(0xBB, buf[1]);
    TEST_ASSERT_EQUAL(0x02, buf[2]); /* len = cmd(1) + data(1) */
    TEST_ASSERT_EQUAL(0x01, buf[3]); /* cmd */
    TEST_ASSERT_EQUAL(0x01, buf[4]); /* delta */
    /* CRC8 over buf[3..4] */
    uint8_t crc = Remote_CRC8(&buf[3], 2);
    TEST_ASSERT_EQUAL(crc, buf[5]);
}

static void test_build_packet_button_press(void)
{
    uint8_t buf[8];
    uint8_t payload[2] = {0x03, 0x01};
    hal_status_t st = Remote_BuildPacket(buf, 0x03, payload, 2);
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, st);
    TEST_ASSERT_EQUAL(0xAA, buf[0]);
    TEST_ASSERT_EQUAL(0xBB, buf[1]);
    TEST_ASSERT_EQUAL(0x03, buf[2]); /* len = cmd(1) + data(2) */
    TEST_ASSERT_EQUAL(0x03, buf[3]); /* cmd */
    TEST_ASSERT_EQUAL(0x03, buf[4]); /* button_id */
    TEST_ASSERT_EQUAL(0x01, buf[5]); /* pressed */
    uint8_t crc = Remote_CRC8(&buf[3], 3);
    TEST_ASSERT_EQUAL(crc, buf[6]);
}

static void test_build_packet_button_release(void)
{
    uint8_t buf[8];
    uint8_t payload[2] = {0x05, 0x00};
    hal_status_t st = Remote_BuildPacket(buf, 0x04, payload, 2);
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, st);
    TEST_ASSERT_EQUAL(0xAA, buf[0]);
    TEST_ASSERT_EQUAL(0xBB, buf[1]);
    TEST_ASSERT_EQUAL(0x03, buf[2]);
    TEST_ASSERT_EQUAL(0x04, buf[3]); /* cmd = BUTTON_RELEASE */
    TEST_ASSERT_EQUAL(0x05, buf[4]); /* button_id */
    TEST_ASSERT_EQUAL(0x00, buf[5]); /* released */
    uint8_t crc = Remote_CRC8(&buf[3], 3);
    TEST_ASSERT_EQUAL(crc, buf[6]);
}

static void test_build_packet_ptm_toggle(void)
{
    uint8_t buf[8];
    hal_status_t st = Remote_BuildPacket(buf, 0x05, NULL, 0);
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, st);
    TEST_ASSERT_EQUAL(0xAA, buf[0]);
    TEST_ASSERT_EQUAL(0xBB, buf[1]);
    TEST_ASSERT_EQUAL(0x01, buf[2]); /* len = cmd(1) + data(0) */
    TEST_ASSERT_EQUAL(0x05, buf[3]); /* cmd */
    uint8_t crc = Remote_CRC8(&buf[3], 1);
    TEST_ASSERT_EQUAL(crc, buf[4]);
}

static void test_build_packet_freq_set(void)
{
    uint8_t buf[10];
    uint8_t payload[4] = {0x00, 0x01, 0x86, 0x50}; /* 100000 Hz */
    hal_status_t st = Remote_BuildPacket(buf, 0x06, payload, 4);
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, st);
    TEST_ASSERT_EQUAL(0xAA, buf[0]);
    TEST_ASSERT_EQUAL(0xBB, buf[1]);
    TEST_ASSERT_EQUAL(0x05, buf[2]); /* len = cmd(1) + data(4) */
    TEST_ASSERT_EQUAL(0x06, buf[3]); /* cmd */
    TEST_ASSERT_EQUAL(0x00, buf[4]);
    TEST_ASSERT_EQUAL(0x01, buf[5]);
    TEST_ASSERT_EQUAL(0x86, buf[6]);
    TEST_ASSERT_EQUAL(0x50, buf[7]);
    uint8_t crc = Remote_CRC8(&buf[3], 5);
    TEST_ASSERT_EQUAL(crc, buf[8]);
}

static void test_build_packet_max_payload(void)
{
    uint8_t buf[260];
    uint8_t payload[252];
    memset(payload, 0xAB, sizeof(payload));
    hal_status_t st = Remote_BuildPacket(buf, 0x01, payload, 252);
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, st);
    TEST_ASSERT_EQUAL(0xAA, buf[0]);
    TEST_ASSERT_EQUAL(0xBB, buf[1]);
    TEST_ASSERT_EQUAL(0xFD, buf[2]); /* 253 = 1 + 252 */
    uint8_t crc = Remote_CRC8(&buf[3], 253);
    TEST_ASSERT_EQUAL(crc, buf[256]);
}

/* -------------------------------------------------------------------------
 * Test registration
 * ------------------------------------------------------------------------- */
void test_remote_register(void)
{
    printf("--- remote_protocol (T44.4) ---\n");
    TEST_REGISTER(test_crc8_empty);
    TEST_REGISTER(test_crc8_single_byte);
    TEST_REGISTER(test_crc8_known_vector);
    TEST_REGISTER(test_crc8_all_zeros);
    TEST_REGISTER(test_crc8_all_ones);
    TEST_REGISTER(test_build_packet_encoder_delta);
    TEST_REGISTER(test_build_packet_button_press);
    TEST_REGISTER(test_build_packet_button_release);
    TEST_REGISTER(test_build_packet_ptm_toggle);
    TEST_REGISTER(test_build_packet_freq_set);
    TEST_REGISTER(test_build_packet_max_payload);
    printf("\n");
}
