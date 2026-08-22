/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_adc.h                                                        **
 **  Description:   Abstract ADC API for STM32 HAL shim layer                       **
 **                 Product code includes this header; NO vendor HAL includes.      **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __HAL_ADC_H
#define __HAL_ADC_H

#include "hal_common.h"

/* -------------------------------------------------------------------------
 * Opaque ADC handle (maps to ADC_HandleTypeDef* in backend)
 * ------------------------------------------------------------------------- */
typedef void* hal_adc_handle_t;

/* -------------------------------------------------------------------------
 * Abstract ADC API
 * ------------------------------------------------------------------------- */
hal_status_t hal_adc_start(hal_adc_handle_t handle);

#endif /* __HAL_ADC_H */
