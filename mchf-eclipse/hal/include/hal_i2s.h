/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_i2s.h                                                       **
 **  Description:   Abstract audio interface API for STM32 HAL shim layer           **
 **                 Product code includes this header; NO vendor HAL includes.      **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __HAL_I2S_H
#define __HAL_I2S_H

#include "hal_common.h"

/* -------------------------------------------------------------------------
 * Audio interface API
 * ------------------------------------------------------------------------- */
hal_status_t hal_i2s_codec_start_dma(void);
hal_status_t hal_i2s_codec_stop_dma(void);
hal_status_t hal_i2s_codec_clear_tx_dma_buffer(void);

#endif /* __HAL_I2S_H */
