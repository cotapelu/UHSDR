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

#include "stm32f4xx_hal.h"

/* -------------------------------------------------------------------------
 * Vendor constant mapping. The shim API uses its own channel/alignment
 * numbering; the vendor constants differ between the MCU families.
 * ------------------------------------------------------------------------- */
static uint32_t hal_dac_vendor_channel(hal_dac_channel_t channel)
{
    return (channel == HAL_DAC_CHANNEL_2) ? DAC_CHANNEL_2 : DAC_CHANNEL_1;
}

static uint32_t hal_dac_vendor_align(hal_dac_align_t align)
{
    switch (align)
    {
        case HAL_DAC_ALIGN_12B_L: return DAC_ALIGN_12B_L;
        case HAL_DAC_ALIGN_8B_R:  return DAC_ALIGN_8B_R;
        case HAL_DAC_ALIGN_12B_R:
        default:                  return DAC_ALIGN_12B_R;
    }
}

/* -------------------------------------------------------------------------
 * API implementation
 * ------------------------------------------------------------------------- */
hal_status_t hal_dac_start(hal_dac_handle_t handle, hal_dac_channel_t channel)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_DAC_Start((DAC_HandleTypeDef*)handle, hal_dac_vendor_channel(channel)) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_dac_set_value(hal_dac_handle_t handle, hal_dac_channel_t channel, hal_dac_align_t align, uint32_t value)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_DAC_SetValue((DAC_HandleTypeDef*)handle, hal_dac_vendor_channel(channel), hal_dac_vendor_align(align), value) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}
