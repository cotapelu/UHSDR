/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     mock_hal_uart.c                                                  **
 **  Description:   Mock HAL UART implementation for host unit testing               **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "mock_hal_uart.h"
#include <stdbool.h>
#include <string.h>

/* Global mock state */
mock_uart_device_t mock_uart_devices[MOCK_UART_MAX_DEVICES];
uint32_t mock_uart_current_handle;

void mock_uart_reset(void)
{
    memset(mock_uart_devices, 0, sizeof(mock_uart_devices));
    mock_uart_current_handle = 0;
}

void mock_uart_set_rx_response(uint32_t handle_idx, const uint8_t* data, uint16_t size)
{
    if (handle_idx < MOCK_UART_MAX_DEVICES && data != NULL && size <= MOCK_UART_MAX_TRANSFER)
    {
        memcpy(mock_uart_devices[handle_idx].rx_buffer, data, size);
        mock_uart_devices[handle_idx].rx_len = size;
    }
}

const uint8_t* mock_uart_get_tx_buffer(uint32_t handle_idx, uint16_t* size)
{
    if (handle_idx < MOCK_UART_MAX_DEVICES)
    {
        if (size) *size = mock_uart_devices[handle_idx].tx_len;
        return mock_uart_devices[handle_idx].tx_buffer;
    }
    if (size) *size = 0;
    return NULL;
}

void mock_uart_inject_rx_byte(uint32_t handle_idx, uint8_t byte)
{
    if (handle_idx < MOCK_UART_MAX_DEVICES)
    {
        if (mock_uart_devices[handle_idx].rx_callback != NULL)
        {
            mock_uart_devices[handle_idx].rx_callback(byte);
        }
    }
}

hal_status_t mock_uart_get_init_status(uint32_t handle_idx)
{
    if (handle_idx < MOCK_UART_MAX_DEVICES && mock_uart_devices[handle_idx].init_count > 0)
    {
        return HAL_STATUS_OK;
    }
    return HAL_STATUS_ERR_HW_FAILURE;
}

hal_uart_handle_t hal_uart_create(void)
{
    if (mock_uart_current_handle >= MOCK_UART_MAX_DEVICES)
    {
        return NULL;
    }
    return (hal_uart_handle_t)(uintptr_t)(mock_uart_current_handle++ + 1);
}

hal_status_t hal_uart_init(hal_uart_handle_t handle, const hal_uart_config_t* config)
{
    if (handle == NULL || config == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle - 1;
    if (handle_idx >= MOCK_UART_MAX_DEVICES)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_uart_devices[handle_idx].init_count++;
    mock_uart_devices[handle_idx].config = *config;
    return HAL_STATUS_OK;
}

hal_status_t hal_uart_register_rx_callback(hal_uart_handle_t handle, hal_uart_rx_callback_t cb)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle - 1;
    if (handle_idx >= MOCK_UART_MAX_DEVICES)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_uart_devices[handle_idx].rx_callback = cb;
    return HAL_STATUS_OK;
}

hal_status_t hal_uart_start_rx(hal_uart_handle_t handle, uint8_t* buf, uint16_t size)
{
    if (handle == NULL || buf == NULL || size == 0)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle - 1;
    if (handle_idx >= MOCK_UART_MAX_DEVICES)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_uart_devices[handle_idx].rx_in_progress = true;
    mock_uart_devices[handle_idx].rx_len = size;
    return HAL_STATUS_OK;
}

hal_status_t hal_uart_stop_rx(hal_uart_handle_t handle)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle - 1;
    if (handle_idx >= MOCK_UART_MAX_DEVICES)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_uart_devices[handle_idx].rx_in_progress = false;
    return HAL_STATUS_OK;
}

hal_status_t hal_uart_transmit(hal_uart_handle_t handle, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    (void)timeout;
    if (handle == NULL || data == NULL || size == 0 || size > MOCK_UART_MAX_TRANSFER)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle - 1;
    if (handle_idx >= MOCK_UART_MAX_DEVICES)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_uart_devices[handle_idx].transmit_count++;
    mock_uart_devices[handle_idx].tx_len = size;
    memcpy(mock_uart_devices[handle_idx].tx_buffer, data, size);
    return HAL_STATUS_OK;
}

hal_status_t hal_uart_receive(hal_uart_handle_t handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    (void)timeout;
    if (handle == NULL || data == NULL || size == 0 || size > MOCK_UART_MAX_TRANSFER)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle - 1;
    if (handle_idx >= MOCK_UART_MAX_DEVICES)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_uart_devices[handle_idx].receive_count++;
    mock_uart_devices[handle_idx].rx_len = size;
    memcpy(data, mock_uart_devices[handle_idx].rx_buffer, size);
    return HAL_STATUS_OK;
}
