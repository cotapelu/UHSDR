/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                                        UHSDR                                    **
**               a powerful firmware for STM32 based SDR transceivers              **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:     hal_tim.h                                                       **
**  Description:   Abstract timer/encoder API for STM32 HAL shim layer            **
**                 Product code includes this header; NO vendor HAL includes.      **
**  Last Modified: 2026-10-28                                                      **
**  Licence:       GNU GPLv3                                                       **
************************************************************************************/

#ifndef __HAL_TIM_H
#define __HAL_TIM_H

#include "hal_common.h"

/* -------------------------------------------------------------------------
 * Opaque timer handle (maps to TIM_HandleTypeDef* in backend)
 * ------------------------------------------------------------------------- */
typedef void* hal_tim_handle_t;

/* -------------------------------------------------------------------------
 * Timer channels. The vendor channel bitmasks differ between the MCU
 * families, so the shim uses its own numbering and the backend maps it onto
 * the vendor constants.
 * ------------------------------------------------------------------------- */
#define HAL_TIM_CHANNEL_1                0x01U
#define HAL_TIM_CHANNEL_2                0x02U
#define HAL_TIM_CHANNEL_3                0x04U
#define HAL_TIM_CHANNEL_4                0x08U

/* Both channels of a quadrature encoder, as expected by
 * hal_tim_encoder_start(). */
#define HAL_TIM_ENCODER_CHANNELS         (HAL_TIM_CHANNEL_1 | HAL_TIM_CHANNEL_2)

/* -------------------------------------------------------------------------
 * Abstract timer API
 * ------------------------------------------------------------------------- */

/* Starts a timer in encoder interface mode. The timer itself must already be
 * initialised (MSP/CubeMX generated init); this starts the encoder interface
 * and its DMA-less counter operation on the given channels. */
hal_status_t hal_tim_encoder_start(hal_tim_handle_t handle, uint32_t channels);

/* Reads / writes the timer counter (32 bit timer counter register). */
hal_status_t hal_tim_get_counter(hal_tim_handle_t handle, uint32_t* count);
hal_status_t hal_tim_set_counter(hal_tim_handle_t handle, uint32_t count);

#endif /* __HAL_TIM_H */
