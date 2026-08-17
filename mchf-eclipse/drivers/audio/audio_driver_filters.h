/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     audio_driver_filters.h                                          **
 **  Description:   Biquad filter coefficient calculations extracted from audio_driver.c **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __AUDIO_DRIVER_FILTERS_H
#define __AUDIO_DRIVER_FILTERS_H

#include <stdint.h>
#include "audio_driver.h"
#include "arm_math.h"

#define B0 0
#define B1 1
#define B2 2
#define A1 3
#define A2 4

void AudioDriver_SetBiquadCoeffs(float32_t* coeffsTo, const float32_t* coeffsFrom);
void AudioDriver_SetBiquadCoeffsAllInstances(arm_biquad_casd_df1_inst_f32 biquad_inst_array[NUM_AUDIO_CHANNELS], uint32_t idx, const float32_t* coeffsFrom);
void AudioDriver_ScaleBiquadCoeffs(float32_t coeffs[5], const float32_t scalingA, const float32_t scalingB);
void AudioDriver_CalcBandstop(float32_t coeffs[5], float32_t f0, float32_t FS);
void AudioDriver_CalcBandpass(float32_t coeffs[5], float32_t f0, float32_t FS);
void AudioDriver_CalcHighShelf(float32_t coeffs[5], float32_t f0, float32_t S, float32_t gain, float32_t FS);
void AudioDriver_CalcLowShelf(float32_t coeffs[5], float32_t f0, float32_t S, float32_t gain, float32_t FS);

#endif
