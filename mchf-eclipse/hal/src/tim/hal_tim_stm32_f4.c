/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                                        UHSDR                                    **
**               a powerful firmware for STM32 based SDR transceivers              **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:     hal_tim_stm32_f4.c                                            **
**  Description:   STM32 HAL backend for the abstract timer/encoder API           **
**                 This is the only file allowed to include the vendor HAL.       **
**  Last Modified: 2026-10-28                                                      **
**  Licence:       GNU GPLv3                                                       **
************************************************************************************/

#include "hal_tim.h"

#include <stddef.h>

#include "stm32f4xx_hal.h"

/* -------------------------------------------------------------------------
 * API implementation
 * ------------------------------------------------------------------------- */
hal_status_t hal_tim_encoder_start(hal_tim_handle_t handle, uint32_t channels)
{
    uint32_t vendor_channels = 0;

    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if ((channels & HAL_TIM_CHANNEL_1) != 0U)
    {
        vendor_channels |= TIM_CHANNEL_1;
    }
    if ((channels & HAL_TIM_CHANNEL_2) != 0U)
    {
        vendor_channels |= TIM_CHANNEL_2;
    }
    if ((channels & HAL_TIM_CHANNEL_3) != 0U)
    {
        vendor_channels |= TIM_CHANNEL_3;
    }
    if ((channels & HAL_TIM_CHANNEL_4) != 0U)
    {
        vendor_channels |= TIM_CHANNEL_4;
    }

    if (HAL_TIM_Encoder_Start((TIM_HandleTypeDef*)handle, vendor_channels) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_tim_get_counter(hal_tim_handle_t handle, uint32_t* count)
{
    if (handle == NULL || count == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    *count = __HAL_TIM_GET_COUNTER((TIM_HandleTypeDef*)handle);

    return HAL_STATUS_OK;
}

hal_status_t hal_tim_set_counter(hal_tim_handle_t handle, uint32_t count)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    __HAL_TIM_SET_COUNTER((TIM_HandleTypeDef*)handle, count);

    return HAL_STATUS_OK;
}
