/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:       audio_rx.h                                                    **
 **  Description:     RX audio processing API (extracted from audio_driver.c)       **
 **  Last Modified:   2026-08-22                                                     **
 **  Licence:         GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __AUDIO_RX_H
#define __AUDIO_RX_H

#include "audio_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize FM RX configuration.
 */
void AudioDriver_FM_Rx_Init(fm_conf_t* fm);

/**
 * @brief Initialize leaky LMS noise reduction.
 */
void AudioDriver_LeakyLmsNr_Init(void);

/**
 * @brief Initialize FreeDV RX processing.
 */
void AudioDriver_FreeDV_Rx_Init(void);

/**
 * @brief Initialize RX processor state.
 */
void RxProcessor_Init(void);

/**
 * @brief Set RX IQ correction parameters.
 */
void AudioDriver_SetRxIqCorrection(void);

/**
 * @brief Set audio processing filters for current demod mode.
 */
void AudioDriver_SetRxTxAudioProcessingAudioFilters(uint8_t dmod_mode);

/**
 * @brief Configure spectrum display parameters.
 */
void AudioDriver_Spectrum_Set(void);

/**
 * @brief Copy IQ buffers for spectrum display.
 */
void AudioDriver_SpectrumCopyIqBuffers(iq_buffer_t* iq_puf_b, const size_t blockSize);

/**
 * @brief Process IQ samples for spectrum display (no zoom).
 */
void AudioDriver_SpectrumNoZoomProcessSamples(iq_buffer_t* iq_puf_b, const uint16_t blockSize);

/**
 * @brief Process IQ samples for spectrum display (with zoom).
 */
void AudioDriver_SpectrumZoomProcessSamples(iq_buffer_t* iq_buf_p, const uint16_t blockSize);

/**
 * @brief Handle IQ correction for RX.
 */
void AudioDriver_RxHandleIqCorrection(float32_t* i_buffer, float32_t* q_buffer, const uint16_t blockSize);

/**
 * @brief Main RX processor - called from I2S callback.
 */
void AudioDriver_RxProcessor(IqSample_t * const srcCodec, AudioSample_t * const dst, const uint16_t blockSize, bool external_mute);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_RX_H */
