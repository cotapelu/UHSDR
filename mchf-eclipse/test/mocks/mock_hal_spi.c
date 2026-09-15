/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     mock_hal_spi.c                                                  **
 **  Description:   Mock HAL SPI implementation for host unit testing               **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "mock_hal_spi.h"
#include "hal_spi.h"
#include <stdbool.h>
#include <string.h>

/* Global mock state */
mock_spi_device_t mock_spi_devices[MOCK_SPI_MAX_DEVICES];
uint32_t mock_spi_current_handle;

void mock_spi_reset(void)
{
    memset(mock_spi_devices, 0, sizeof(mock_spi_devices));
    mock_spi_current_handle = 0;
}

void mock_spi_set_rx_response(uint32_t handle_idx, const uint8_t* data, uint16_t size)
{
    if (handle_idx < MOCK_SPI_MAX_DEVICES && data != NULL && size <= MOCK_SPI_MAX_TRANSFER)
    {
        memcpy(mock_spi_devices[handle_idx].rx_buffer, data, size);
        mock_spi_devices[handle_idx].rx_len = size;
    }
}

const uint8_t* mock_spi_get_tx_buffer(uint32_t handle_idx, uint16_t* size)
{
    if (handle_idx < MOCK_SPI_MAX_DEVICES)
    {
        if (size) *size = mock_spi_devices[handle_idx].tx_len;
        return mock_spi_devices[handle_idx].tx_buffer;
    }
    if (size) *size = 0;
    return NULL;
}

hal_status_t hal_spi_init(hal_spi_handle_t handle)
{
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle;
    if (handle_idx >= MOCK_SPI_MAX_DEVICES)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_spi_devices[handle_idx].init_count++;
    mock_spi_devices[handle_idx].enabled = true;
    return HAL_STATUS_OK;
}

hal_status_t hal_spi_deinit(hal_spi_handle_t handle)
{
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle;
    if (handle_idx >= MOCK_SPI_MAX_DEVICES)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_spi_devices[handle_idx].enabled = false;
    return HAL_STATUS_OK;
}

hal_status_t hal_spi_enable(hal_spi_handle_t handle)
{
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle;
    if (handle_idx >= MOCK_SPI_MAX_DEVICES)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_spi_devices[handle_idx].enabled = true;
    return HAL_STATUS_OK;
}

hal_status_t hal_spi_disable(hal_spi_handle_t handle)
{
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle;
    if (handle_idx >= MOCK_SPI_MAX_DEVICES)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_spi_devices[handle_idx].enabled = false;
    return HAL_STATUS_OK;
}

hal_status_t hal_spi_transmit(hal_spi_handle_t handle, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    (void)timeout;
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle;
    if (handle_idx >= MOCK_SPI_MAX_DEVICES || data == NULL || size > MOCK_SPI_MAX_TRANSFER)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_spi_devices[handle_idx].transmit_count++;
    mock_spi_devices[handle_idx].tx_len = size;
    memcpy(mock_spi_devices[handle_idx].tx_buffer, data, size);
    return HAL_STATUS_OK;
}

hal_status_t hal_spi_receive(hal_spi_handle_t handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    (void)timeout;
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle;
    if (handle_idx >= MOCK_SPI_MAX_DEVICES || data == NULL || size > MOCK_SPI_MAX_TRANSFER)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_spi_devices[handle_idx].receive_count++;
    mock_spi_devices[handle_idx].rx_len = size;
    memcpy(data, mock_spi_devices[handle_idx].rx_buffer, size);
    return HAL_STATUS_OK;
}

hal_status_t hal_spi_transmit_receive(hal_spi_handle_t handle, const uint8_t* tx, uint8_t* rx, uint16_t size, uint32_t timeout)
{
    (void)timeout;
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle;
    if (handle_idx >= MOCK_SPI_MAX_DEVICES || tx == NULL || rx == NULL || size > MOCK_SPI_MAX_TRANSFER)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_spi_devices[handle_idx].transmit_count++;
    mock_spi_devices[handle_idx].receive_count++;
    mock_spi_devices[handle_idx].tx_len = size;
    mock_spi_devices[handle_idx].rx_len = size;
    memcpy(mock_spi_devices[handle_idx].tx_buffer, tx, size);
    memcpy(rx, mock_spi_devices[handle_idx].rx_buffer, size);
    return HAL_STATUS_OK;
}

hal_status_t hal_spi_transmit_dma(hal_spi_handle_t handle, const uint8_t* data, uint16_t size)
{
    return hal_spi_transmit(handle, data, size, 0);
}

hal_status_t hal_spi_receive_dma(hal_spi_handle_t handle, uint8_t* data, uint16_t size)
{
    return hal_spi_receive(handle, data, size, 0);
}
