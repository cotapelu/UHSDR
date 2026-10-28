/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_dac.h                                                        **
 **  Description:   Abstract DAC API for STM32 HAL shim layer                       **
 **                 Product code includes this header; NO vendor HAL includes.      **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __HAL_DAC_H
#define __HAL_DAC_H

#include "hal_common.h"

/* -------------------------------------------------------------------------
 * Opaque DAC handle (maps to DAC_HandleTypeDef* in backend)
 * ------------------------------------------------------------------------- */
typedef void* hal_dac_handle_t;

/* -------------------------------------------------------------------------
 * DAC channel selection. The vendor channel constants differ between the MCU
 * families, so the shim uses its own numbering and the backend maps it onto
 * the vendor constants.
 * ------------------------------------------------------------------------- */
typedef enum {
    HAL_DAC_CHANNEL_1 = 0,
    HAL_DAC_CHANNEL_2 = 1,
} hal_dac_channel_t;

/* -------------------------------------------------------------------------
 * DAC data alignment, normalised for the shim API.
 * ------------------------------------------------------------------------- */
typedef enum {
    HAL_DAC_ALIGN_12B_R = 0,
    HAL_DAC_ALIGN_12B_L = 1,
    HAL_DAC_ALIGN_8B_R  = 2,
} hal_dac_align_t;

/* -------------------------------------------------------------------------
 * Abstract DAC API
 * ------------------------------------------------------------------------- */
hal_status_t hal_dac_start(hal_dac_handle_t handle, hal_dac_channel_t channel);
hal_status_t hal_dac_set_value(hal_dac_handle_t handle, hal_dac_channel_t channel, hal_dac_align_t align, uint32_t value);

#endif /* __HAL_DAC_H */
