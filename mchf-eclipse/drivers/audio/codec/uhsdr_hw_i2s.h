/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/

#ifndef __MCHF_HW_I2S_H
#define __MCHF_HW_I2S_H

#include "uhsdr_board_config.h"

void UhsdrHwI2s_Codec_StartDMA(void);
void UhsdrHwI2s_Codec_StopDMA(void);

void UhsdrHwI2s_Codec_ClearTxDmaBuffer(void);

typedef struct {
    void (*start)(void);
    void (*stop)(void);
    void (*clear_tx)(void);
    void (*set_bit_width)(void);
} audio_if_t;

extern const audio_if_t audio_if;

#endif

