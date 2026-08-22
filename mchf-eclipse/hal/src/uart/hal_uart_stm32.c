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

#if defined(STM32F407xx)
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"
#elif defined(STM32F767xx)
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_uart.h"
#elif defined(STM32H743xx)
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_uart.h"
#endif

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
