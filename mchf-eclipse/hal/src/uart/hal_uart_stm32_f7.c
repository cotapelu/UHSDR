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

/* USART2 is the conventional default on F7 *and* matches the F4 backend.
 * The handle used to be left zero-initialised (Instance == NULL), which makes
 * HAL_UART_Init() dereference NULL inside UART_SetConfig(). */
#define UART_BACKEND_INSTANCE      USART2

hal_uart_handle_t hal_uart_create(void)
{
    g_uart_inst.Instance = UART_BACKEND_INSTANCE;
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

    /* OVI40 (F7): the remote UI-pair pinout is not yet confirmed (see
     * hardware/board_configs/UHSDR_UI_ovi40_config.h, REMOTE_UI_PAIR_ENABLE).
     * Refuse to touch hardware rather than guess a pin map. */
#if !REMOTE_UI_PAIR_ENABLE
    return HAL_STATUS_ERR_NOT_SUPPORTED;
#else
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
#endif /* !REMOTE_UI_PAIR_ENABLE */
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

    /* The F7 UART HAL shipped in basesw/ovi40 has no HAL_UART_AbortReceive_IT
     * (nor HAL_UART_AbortReceive), so this used to call an undeclared
     * function and would not have linked. Drop the RX interrupt line and mark
     * the handle ready instead — this is what that legacy driver does
     * internally when a reception is aborted. */
    __HAL_UART_DISABLE_IT((UART_HandleTypeDef*)handle, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT((UART_HandleTypeDef*)handle, UART_IT_PE);
    ((UART_HandleTypeDef*)handle)->RxState = HAL_UART_STATE_READY;

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
