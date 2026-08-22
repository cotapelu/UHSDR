/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_dac_stm32.c                                                 **
 **  Description:   STM32 backend for hal_dac.h                                     **
 **                 ONLY this file includes vendor HAL DAC headers.                **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "hal_dac.h"

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
hal_status_t hal_dac_start(hal_dac_handle_t handle, uint32_t channel)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_DAC_Start((DAC_HandleTypeDef*)handle, channel) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_dac_set_value(hal_dac_handle_t handle, uint32_t channel, uint32_t align, uint32_t value)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_DAC_SetValue((DAC_HandleTypeDef*)handle, channel, align, value) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}
