/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_dma_stm32.c                                                 **
 **  Description:   STM32 backend for hal_dma.h                                     **
 **                 ONLY this file includes vendor HAL DMA headers.                 **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "hal_dma.h"

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
hal_status_t hal_dma_init(hal_dma_handle_t handle)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_DMA_Init((DMA_HandleTypeDef*)handle) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_dma_deinit(hal_dma_handle_t handle)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    HAL_DMA_DeInit((DMA_HandleTypeDef*)handle);

    return HAL_STATUS_OK;
}

hal_status_t hal_dma_start(hal_dma_handle_t handle, uint32_t src, uint32_t dst, uint32_t len)
{
    if (handle == NULL || len == 0)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_DMA_Start((DMA_HandleTypeDef*)handle, src, dst, len) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_dma_stop(hal_dma_handle_t handle)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

#if defined(STM32H743xx)
    HAL_DMA_Abort((DMA_HandleTypeDef*)handle);
#else
    HAL_DMA_Stop((DMA_HandleTypeDef*)handle);
#endif

    return HAL_STATUS_OK;
}

hal_status_t hal_dma_wait_complete(hal_dma_handle_t handle, uint32_t timeout)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    uint32_t tickstart = HAL_GetTick();
    while (((DMA_HandleTypeDef*)handle)->State != HAL_DMA_STATE_READY)
    {
        if (timeout != HAL_MAX_DELAY)
        {
            if ((HAL_GetTick() - tickstart) > timeout)
            {
                return HAL_STATUS_ERR_TIMEOUT;
            }
        }
    }

    return HAL_STATUS_OK;
}
