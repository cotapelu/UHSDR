/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     mock_codec.h                                                    **
 **  Description:   Stub prototypes for drivers/audio/codec/codec.h API              **
 **  Last Modified: 2026-08-24                                                      **
 **  Licence:       GNU GPLv3                                                       **
 ************************************************************************************/

#ifndef __MOCK_CODEC_H
#define __MOCK_CODEC_H

#include <stdint.h>
#include <stdbool.h>

void Codec_SwitchTxRxMode(uint8_t txrx_mode);
void Codec_MCUInterfaceInit(uint32_t AudioFreq);
void Codec_VolumeLineOut(uint8_t txrx_mode);
void Codec_VolumeSpkr(uint8_t vol);
void Codec_LineInGainAdj(uint8_t gain);
void Codec_IQInGainAdj(uint8_t gain);
void Codec_MuteDAC(bool state);
uint32_t Codec_Reset(uint32_t AudioFreq);
void Codec_RestartI2S(void);
void Codec_TxSidetoneSetgain(uint8_t mode);
void Codec_SwitchMicTxRxMode(uint8_t mode);
void Codec_PrepareTx(uint8_t current_txrx_mode);
bool Codec_ReadyForIrqCall(void);

#endif /* __MOCK_CODEC_H */