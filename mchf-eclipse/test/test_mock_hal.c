/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_mock_hal.c                                                 **
 **  Description:   Unit tests for mock HAL layer (GPIO, I2C, SPI)                  **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include "mock_hal.h"
#include <string.h>

/* -------------------------------------------------------------------------
 * Helper macros for port handles
 * ------------------------------------------------------------------------- */
#define PORT_0 ((hal_gpio_port_t)(uintptr_t)0)
#define PORT_1 ((hal_gpio_port_t)(uintptr_t)1)
#define I2C_HANDLE_0 ((hal_i2c_handle_t)(uintptr_t)0)
#define SPI_HANDLE_0 ((hal_spi_handle_t)(uintptr_t)0)

/* =========================================================================
 * GPIO Mock Tests
 * ========================================================================= */

static void test_gpio_init_increments_count(void)
{
    mock_gpio_reset();
    hal_gpio_config_t cfg = { .pin = 0x01, .mode = HAL_GPIO_MODE_OUTPUT_PP, .pull = HAL_GPIO_PULL_UP, .speed = HAL_GPIO_SPEED_HIGH };
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, hal_gpio_init(PORT_0, &cfg));
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, hal_gpio_init(PORT_0, &cfg));
    TEST_ASSERT_EQUAL(2, mock_gpio_ports[0].init_count);
}

static void test_gpio_write_and_read_pin(void)
{
    mock_gpio_reset();
    hal_gpio_config_t cfg = { .pin = 0x01, .mode = HAL_GPIO_MODE_OUTPUT_PP, .pull = HAL_GPIO_PULL_NONE, .speed = HAL_GPIO_SPEED_LOW };
    hal_gpio_init(PORT_0, &cfg);
    hal_gpio_write_pin(PORT_0, 0x01, HAL_GPIO_PIN_SET);
    TEST_ASSERT_EQUAL(HAL_GPIO_PIN_SET, hal_gpio_read_pin(PORT_0, 0x01));
    hal_gpio_write_pin(PORT_0, 0x01, HAL_GPIO_PIN_RESET);
    TEST_ASSERT_EQUAL(HAL_GPIO_PIN_RESET, hal_gpio_read_pin(PORT_0, 0x01));
}

static void test_gpio_toggle_pin(void)
{
    mock_gpio_reset();
    hal_gpio_config_t cfg = { .pin = 0x02, .mode = HAL_GPIO_MODE_OUTPUT_PP, .pull = HAL_GPIO_PULL_NONE, .speed = HAL_GPIO_SPEED_LOW };
    hal_gpio_init(PORT_1, &cfg);
    hal_gpio_write_pin(PORT_1, 0x02, HAL_GPIO_PIN_SET);
    TEST_ASSERT_EQUAL(HAL_GPIO_PIN_SET, hal_gpio_read_pin(PORT_1, 0x02));
    hal_gpio_toggle_pin(PORT_1, 0x02);
    TEST_ASSERT_EQUAL(HAL_GPIO_PIN_RESET, hal_gpio_read_pin(PORT_1, 0x02));
    hal_gpio_toggle_pin(PORT_1, 0x02);
    TEST_ASSERT_EQUAL(HAL_GPIO_PIN_SET, hal_gpio_read_pin(PORT_1, 0x02));
}

static void test_gpio_deinit_increments_count(void)
{
    mock_gpio_reset();
    hal_gpio_config_t cfg = { .pin = 0x04, .mode = HAL_GPIO_MODE_INPUT, .pull = HAL_GPIO_PULL_DOWN, .speed = HAL_GPIO_SPEED_LOW };
    hal_gpio_init(PORT_0, &cfg);
    hal_gpio_deinit(PORT_0, 0x04);
    TEST_ASSERT_EQUAL(1, mock_gpio_ports[0].deinit_count);
}

static void test_gpio_invalid_port_returns_error(void)
{
    mock_gpio_reset();
    hal_gpio_config_t cfg = { .pin = 0x01, .mode = HAL_GPIO_MODE_OUTPUT_PP, .pull = HAL_GPIO_PULL_NONE, .speed = HAL_GPIO_SPEED_LOW };
    TEST_ASSERT_EQUAL(HAL_STATUS_ERR_INVALID_PARAM, hal_gpio_init((hal_gpio_port_t)(uintptr_t)999, &cfg));
}

/* =========================================================================
 * I2C Mock Tests
 * ========================================================================= */

static void test_i2c_init_and_device_ready(void)
{
    mock_i2c_reset();
    mock_i2c_add_device(0, 0x50, 256);
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, hal_i2c_init(I2C_HANDLE_0));
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, hal_i2c_is_device_ready(I2C_HANDLE_0, 0x50, 3, 100));
}

static void test_i2c_mem_write_and_read(void)
{
    mock_i2c_reset();
    mock_i2c_add_device(0, 0x50, 256);
    uint8_t write_data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, hal_i2c_mem_write(I2C_HANDLE_0, 0x50, 0x10, 1, write_data, 4, 100));
    uint8_t read_data[4] = {0};
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, hal_i2c_mem_read(I2C_HANDLE_0, 0x50, 0x10, 1, read_data, 4, 100));
    TEST_ASSERT_EQUAL(0xDE, read_data[0]);
    TEST_ASSERT_EQUAL(0xAD, read_data[1]);
    TEST_ASSERT_EQUAL(0xBE, read_data[2]);
    TEST_ASSERT_EQUAL(0xEF, read_data[3]);
}

static void test_i2c_device_not_ready(void)
{
    mock_i2c_reset();
    mock_i2c_add_device(0, 0x50, 256);
    mock_i2c_set_device_ready(0, false);
    TEST_ASSERT_EQUAL(HAL_STATUS_ERR_HW_FAILURE, hal_i2c_is_device_ready(I2C_HANDLE_0, 0x50, 3, 100));
}

static void test_i2c_invalid_handle(void)
{
    mock_i2c_reset();
    mock_i2c_add_device(0, 0x50, 256);
    uint8_t data = 0x42;
    TEST_ASSERT_EQUAL(HAL_STATUS_ERR_INVALID_PARAM, hal_i2c_mem_read((hal_i2c_handle_t)(uintptr_t)999, 0x50, 0, 1, &data, 1, 100));
}

/* =========================================================================
 * SPI Mock Tests
 * ========================================================================= */

static void test_spi_init_and_enable(void)
{
    mock_spi_reset();
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, hal_spi_init(SPI_HANDLE_0));
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, hal_spi_enable(SPI_HANDLE_0));
    TEST_ASSERT_EQUAL(1, mock_spi_devices[0].init_count);
    TEST_ASSERT(mock_spi_devices[0].enabled);
}

static void test_spi_transmit_and_receive(void)
{
    mock_spi_reset();
    uint8_t tx_data[] = {0x01, 0x02, 0x03};
    uint8_t rx_data[3] = {0xAA, 0xBB, 0xCC};
    mock_spi_set_rx_response(0, rx_data, 3);
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, hal_spi_transmit(SPI_HANDLE_0, tx_data, 3, 100));
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, hal_spi_receive(SPI_HANDLE_0, rx_data, 3, 100));
    TEST_ASSERT_EQUAL(0x01, mock_spi_devices[0].tx_buffer[0]);
    TEST_ASSERT_EQUAL(0xAA, rx_data[0]);
    TEST_ASSERT_EQUAL(0xBB, rx_data[1]);
}

static void test_spi_transmit_receive(void)
{
    mock_spi_reset();
    uint8_t tx_data[] = {0x01, 0x02, 0x03};
    uint8_t rx_data[3] = {0x00, 0x00, 0x00};
    mock_spi_set_rx_response(0, rx_data, 3);
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, hal_spi_transmit_receive(SPI_HANDLE_0, tx_data, rx_data, 3, 100));
    TEST_ASSERT_EQUAL(0x01, mock_spi_devices[0].tx_buffer[0]);
    TEST_ASSERT_EQUAL(3, mock_spi_devices[0].tx_len);
}

static void test_spi_disable(void)
{
    mock_spi_reset();
    hal_spi_init(SPI_HANDLE_0);
    hal_spi_enable(SPI_HANDLE_0);
    hal_spi_disable(SPI_HANDLE_0);
    TEST_ASSERT(!mock_spi_devices[0].enabled);
}

/* =========================================================================
 * Registration
 * ========================================================================= */

void test_mock_hal_register(void)
{
    printf("--- mock_hal ---\n");
    /* GPIO tests */
    TEST_REGISTER(test_gpio_init_increments_count);
    TEST_REGISTER(test_gpio_write_and_read_pin);
    TEST_REGISTER(test_gpio_toggle_pin);
    TEST_REGISTER(test_gpio_deinit_increments_count);
    TEST_REGISTER(test_gpio_invalid_port_returns_error);
    /* I2C tests */
    TEST_REGISTER(test_i2c_init_and_device_ready);
    TEST_REGISTER(test_i2c_mem_write_and_read);
    TEST_REGISTER(test_i2c_device_not_ready);
    TEST_REGISTER(test_i2c_invalid_handle);
    /* SPI tests */
    TEST_REGISTER(test_spi_init_and_enable);
    TEST_REGISTER(test_spi_transmit_and_receive);
    TEST_REGISTER(test_spi_transmit_receive);
    TEST_REGISTER(test_spi_disable);
    printf("\n");
}
