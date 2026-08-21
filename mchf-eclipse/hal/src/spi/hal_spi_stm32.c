/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_spi_stm32.c                                                 **
 **  Description:   STM32 backend for hal_spi.h                                     **
 **                 ONLY this file includes vendor HAL SPI headers.                 **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "hal_spi.h"

#include <stddef.h>

#if defined(STM32F407xx)
#include "stm32f4xx_hal.h"
#elif defined(STM32F767xx)
#include "stm32f7xx_hal.h"
#elif defined(STM32H743xx)
#include "stm32h7xx_hal.h"
#endif

/* -------------------------------------------------------------------------
 * API implementation
 * ------------------------------------------------------------------------- */
hal_status_t hal_spi_init(hal_spi_handle_t handle)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_SPI_Init((SPI_HandleTypeDef*)handle) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_spi_deinit(hal_spi_handle_t handle)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    HAL_SPI_DeInit((SPI_HandleTypeDef*)handle);

    return HAL_STATUS_OK;
}

hal_status_t hal_spi_enable(hal_spi_handle_t handle)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    __HAL_SPI_ENABLE((SPI_HandleTypeDef*)handle);

    return HAL_STATUS_OK;
}

hal_status_t hal_spi_disable(hal_spi_handle_t handle)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    __HAL_SPI_DISABLE((SPI_HandleTypeDef*)handle);

    return HAL_STATUS_OK;
}

hal_status_t hal_spi_transmit(hal_spi_handle_t handle, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || data == NULL || size == 0)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_SPI_Transmit((SPI_HandleTypeDef*)handle, (uint8_t*)data, size, timeout) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_spi_receive(hal_spi_handle_t handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || data == NULL || size == 0)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_SPI_Receive((SPI_HandleTypeDef*)handle, data, size, timeout) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_spi_transmit_receive(hal_spi_handle_t handle, const uint8_t* tx, uint8_t* rx, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || tx == NULL || rx == NULL || size == 0)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_SPI_TransmitReceive((SPI_HandleTypeDef*)handle, (uint8_t*)tx, rx, size, timeout) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_spi_transmit_dma(hal_spi_handle_t handle, const uint8_t* data, uint16_t size)
{
    if (handle == NULL || data == NULL || size == 0)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_SPI_Transmit_DMA((SPI_HandleTypeDef*)handle, (uint8_t*)data, size) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_spi_receive_dma(hal_spi_handle_t handle, uint8_t* data, uint16_t size)
{
    if (handle == NULL || data == NULL || size == 0)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_SPI_Receive_DMA((SPI_HandleTypeDef*)handle, data, size) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}
