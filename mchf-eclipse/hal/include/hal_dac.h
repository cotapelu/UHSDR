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
 * Abstract DAC API
 * ------------------------------------------------------------------------- */
hal_status_t hal_dac_start(hal_dac_handle_t handle, uint32_t channel);
hal_status_t hal_dac_set_value(hal_dac_handle_t handle, uint32_t channel, uint32_t align, uint32_t value);

#endif /* __HAL_DAC_H */
