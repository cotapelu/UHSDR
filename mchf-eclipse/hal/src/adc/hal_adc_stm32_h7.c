/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_adc_stm32.c                                                 **
 **  Description:   STM32 backend for hal_adc.h                                     **
 **                 ONLY this file includes vendor HAL ADC headers.                **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "hal_adc.h"

#include <stddef.h>

#include "stm32h7xx_hal.h"

/* -------------------------------------------------------------------------
 * API implementation
 * ------------------------------------------------------------------------- */
hal_status_t hal_adc_start(hal_adc_handle_t handle)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_ADC_Start((ADC_HandleTypeDef*)handle) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_adc_get_value(hal_adc_handle_t handle, uint32_t* value)
{
    if (handle == NULL || value == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    /* The result register is 32 bit wide on every supported family and holds
     * the 12 bit right aligned conversion result. */
    *value = HAL_ADC_GetValue((ADC_HandleTypeDef*)handle);

    return HAL_STATUS_OK;
}
