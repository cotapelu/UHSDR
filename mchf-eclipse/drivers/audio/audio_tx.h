/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------** **                                                                                 **
 **  File name:       audio_tx.h                                                    **
 **  Description:     TX audio processing API (extracted from audio_driver.c)       **
 **  Last Modified:   2026-08-22                                                     **
 **  Licence:         GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __AUDIO_TX_H
#define __AUDIO_TX_H

#include "audio_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set audio processing filters for current demod mode (RX + TX).
 */
void AudioDriver_SetRxTxAudioProcessingAudioFilters(uint8_t dmod_mode);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_TX_H */
