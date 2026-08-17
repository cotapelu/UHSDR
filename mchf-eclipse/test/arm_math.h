/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     arm_math.h                                                      **
 **  Description:   Minimal mock of CMSIS DSP arm_math.h for host testing           **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __ARM_MATH_H
#define __ARM_MATH_H

#include <stdint.h>

/* Minimal type definitions */
typedef float float32_t;

/* Biquad filter instance structure - only what audio_driver_filters needs */
typedef struct
{
    uint32_t numStages;            /*!< Number of stages in the filter */
    float32_t *pCoeffs;            /*!< Pointer to the filter coefficients */
    float32_t *pState;             /*!< Pointer to the filter state variables */
} arm_biquad_casd_df1_inst_f32;

#endif /* __ARM_MATH_H */
