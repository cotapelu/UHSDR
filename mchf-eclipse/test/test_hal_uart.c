/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_hal_uart.c                                                  **
 **  Description:   Host unit tests for HAL UART shim layer                         **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include "mock_hal.h"

static uint8_t g_uart_rx_test_byte = 0;

static void uart_rx_test_callback(uint8_t byte)
{
    g_uart_rx_test_byte = byte;
}

/* =========================================================================
 * UART Mock Tests
 * ========================================================================= */

static void test_uart_create_returns_non_null(void)
{
    mock_uart_reset();
    hal_uart_handle_t h = hal_uart_create();
    TEST_ASSERT(h != NULL);
}

static void test_uart_create_multiple_instances(void)
{
    mock_uart_reset();
    hal_uart_handle_t h1 = hal_uart_create();
    hal_uart_handle_t h2 = hal_uart_create();
    TEST_ASSERT(h1 != NULL);
    TEST_ASSERT(h2 != NULL);
    TEST_ASSERT((uintptr_t)h1 != (uintptr_t)h2);
}

static void test_uart_init_invalid_handle(void)
{
    mock_uart_reset();
    hal_uart_config_t cfg = { 115200, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE, UART_HWCONTROL_NONE };
    TEST_ASSERT_EQUAL(HAL_STATUS_ERR_INVALID_PARAM, hal_uart_init(NULL, &cfg));
}

static void test_uart_init_invalid_config(void)
{
    mock_uart_reset();
    hal_uart_handle_t h = hal_uart_create();
    TEST_ASSERT_EQUAL(HAL_STATUS_ERR_INVALID_PARAM, hal_uart_init(h, NULL));
}

static void test_uart_init_valid_increments_count(void)
{
    mock_uart_reset();
    hal_uart_handle_t h = hal_uart_create();
    hal_uart_config_t cfg = { 115200, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE, UART_HWCONTROL_NONE };
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, hal_uart_init(h, &cfg));
    TEST_ASSERT_EQUAL(1, mock_uart_devices[0].init_count);
    TEST_ASSERT_EQUAL(cfg.baud_rate, mock_uart_devices[0].config.baud_rate);
}

static void test_uart_register_rx_callback(void)
{
    mock_uart_reset();
    hal_uart_handle_t h = hal_uart_create();
    hal_uart_init(h, &(hal_uart_config_t){ 115200, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE, UART_HWCONTROL_NONE });
    hal_uart_register_rx_callback(h, uart_rx_test_callback);
    TEST_ASSERT(mock_uart_devices[0].rx_callback != NULL);
}

static void test_uart_start_rx_invalid_params(void)
{
    mock_uart_reset();
    hal_uart_handle_t h = hal_uart_create();
    TEST_ASSERT_EQUAL(HAL_STATUS_ERR_INVALID_PARAM, hal_uart_start_rx(h, NULL, 1));
    TEST_ASSERT_EQUAL(HAL_STATUS_ERR_INVALID_PARAM, hal_uart_start_rx(h, &(uint8_t){0}, 0));
}

static void test_uart_start_rx_sets_flag(void)
{
    mock_uart_reset();
    hal_uart_handle_t h = hal_uart_create();
    hal_uart_init(h, &(hal_uart_config_t){ 115200, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE, UART_HWCONTROL_NONE });
    uint8_t buf = 0;
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, hal_uart_start_rx(h, &buf, 1));
    TEST_ASSERT(mock_uart_devices[0].rx_in_progress != 0);
}

static void test_uart_stop_rx_clears_flag(void)
{
    mock_uart_reset();
    hal_uart_handle_t h = hal_uart_create();
    hal_uart_init(h, &(hal_uart_config_t){ 115200, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE, UART_HWCONTROL_NONE });
    uint8_t buf = 0;
    hal_uart_start_rx(h, &buf, 1);
    TEST_ASSERT(mock_uart_devices[0].rx_in_progress != 0);
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, hal_uart_stop_rx(h));
    TEST_ASSERT(mock_uart_devices[0].rx_in_progress == 0);
}

static void test_uart_transmit_stores_data(void)
{
    mock_uart_reset();
    hal_uart_handle_t h = hal_uart_create();
    hal_uart_init(h, &(hal_uart_config_t){ 115200, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE, UART_HWCONTROL_NONE });
    uint8_t data[] = {0xAA, 0xBB, 0xCC};
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, hal_uart_transmit(h, data, 3, 100));
    TEST_ASSERT_EQUAL(3, mock_uart_devices[0].tx_len);
    TEST_ASSERT_EQUAL(0xAA, mock_uart_devices[0].tx_buffer[0]);
    TEST_ASSERT_EQUAL(0xBB, mock_uart_devices[0].tx_buffer[1]);
    TEST_ASSERT_EQUAL(0xCC, mock_uart_devices[0].tx_buffer[2]);
}

static void test_uart_receive_returns_data(void)
{
    mock_uart_reset();
    hal_uart_handle_t h = hal_uart_create();
    hal_uart_init(h, &(hal_uart_config_t){ 115200, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE, UART_HWCONTROL_NONE });
    uint8_t rx_data[] = {0xDD, 0xEE};
    mock_uart_set_rx_response(0, rx_data, 2);
    uint8_t buf[2] = {0};
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, hal_uart_receive(h, buf, 2, 100));
    TEST_ASSERT_EQUAL(0xDD, buf[0]);
    TEST_ASSERT_EQUAL(0xEE, buf[1]);
}

static void test_uart_inject_rx_byte_triggers_callback(void)
{
    mock_uart_reset();
    hal_uart_handle_t h = hal_uart_create();
    hal_uart_init(h, &(hal_uart_config_t){ 115200, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE, UART_HWCONTROL_NONE });
    g_uart_rx_test_byte = 0;
    hal_uart_register_rx_callback(h, uart_rx_test_callback);
    mock_uart_inject_rx_byte(0, 0xAB);
    TEST_ASSERT_EQUAL(0xAB, g_uart_rx_test_byte);
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_hal_uart_register(void)
{
    printf("--- hal_uart ---\n");
    TEST_REGISTER(test_uart_create_returns_non_null);
    TEST_REGISTER(test_uart_create_multiple_instances);
    TEST_REGISTER(test_uart_init_invalid_handle);
    TEST_REGISTER(test_uart_init_invalid_config);
    TEST_REGISTER(test_uart_init_valid_increments_count);
    TEST_REGISTER(test_uart_register_rx_callback);
    TEST_REGISTER(test_uart_start_rx_invalid_params);
    TEST_REGISTER(test_uart_start_rx_sets_flag);
    TEST_REGISTER(test_uart_stop_rx_clears_flag);
    TEST_REGISTER(test_uart_transmit_stores_data);
    TEST_REGISTER(test_uart_receive_returns_data);
    TEST_REGISTER(test_uart_inject_rx_byte_triggers_callback);
    printf("\n");
}
