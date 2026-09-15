/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_uart_stm32.c                                                **
 **  Description:   STM32 backend for hal_uart.h                                    **
 **                 ONLY this file includes vendor HAL UART headers.               **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "hal_uart.h"

#include <stddef.h>

#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_uart.h"

/* -------------------------------------------------------------------------
 * Handle management (static allocation — no heap)
 * ------------------------------------------------------------------------- */
static UART_HandleTypeDef g_uart_inst;

hal_uart_handle_t hal_uart_create(void)
{
    return &g_uart_inst;
}

/* -------------------------------------------------------------------------
 * Internal state
 * ------------------------------------------------------------------------- */
static hal_uart_rx_callback_t g_uart_rx_cb = NULL;
static uint8_t* g_uart_rx_buf = NULL;

/* -------------------------------------------------------------------------
 * API implementation
 * ------------------------------------------------------------------------- */
hal_status_t hal_uart_transmit(hal_uart_handle_t handle, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || data == NULL || size == 0)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_UART_Transmit((UART_HandleTypeDef*)handle, (uint8_t*)data, size, timeout) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_uart_receive(hal_uart_handle_t handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || data == NULL || size == 0)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_UART_Receive((UART_HandleTypeDef*)handle, data, size, timeout) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_uart_init(hal_uart_handle_t handle, const hal_uart_config_t* config)
{
    UART_HandleTypeDef* huart = (UART_HandleTypeDef*)handle;

    if (huart == NULL || config == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    huart->Init.BaudRate   = config->baud_rate;
    huart->Init.WordLength = config->word_length;
    huart->Init.StopBits   = config->stop_bits;
    huart->Init.Parity     = config->parity;
    huart->Init.HwFlowCtl  = config->hw_flow_ctrl;
    huart->Init.Mode       = UART_MODE_TX_RX;

    if (HAL_UART_Init(huart) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_uart_register_rx_callback(hal_uart_handle_t handle, hal_uart_rx_callback_t cb)
{
    (void)handle;
    g_uart_rx_cb = cb;
    return HAL_STATUS_OK;
}

hal_status_t hal_uart_start_rx(hal_uart_handle_t handle, uint8_t* buf, uint16_t size)
{
    if (handle == NULL || buf == NULL || size == 0)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    g_uart_rx_buf = buf;

    if (HAL_UART_Receive_IT((UART_HandleTypeDef*)handle, buf, size) != HAL_OK)
    {
        g_uart_rx_buf = NULL;
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_uart_stop_rx(hal_uart_handle_t handle)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    g_uart_rx_buf = NULL;

    if (HAL_UART_AbortReceive_IT((UART_HandleTypeDef*)handle) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

/* -------------------------------------------------------------------------
 * Weak callback override — forward to registered callback
 * ------------------------------------------------------------------------- */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    (void)huart;
    if (g_uart_rx_cb != NULL)
    {
        uint8_t byte = (g_uart_rx_buf != NULL) ? g_uart_rx_buf[0] : 0;
        g_uart_rx_cb(byte);
    }
}
