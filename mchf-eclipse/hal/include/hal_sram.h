/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_sram.h                                                       **
 **  Description:   Abstract SRAM API for STM32 HAL shim layer                      **
 **                 Product code includes this header; NO vendor HAL includes.      **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __HAL_SRAM_H
#define __HAL_SRAM_H

#include "hal_common.h"

/* -------------------------------------------------------------------------
 * Opaque SRAM handle (maps to SRAM_HandleTypeDef* in backend)
 * ------------------------------------------------------------------------- */
typedef void* hal_sram_handle_t;

/* -------------------------------------------------------------------------
 * Abstract SRAM API
 * ------------------------------------------------------------------------- */
hal_status_t hal_sram_init(hal_sram_handle_t handle);
hal_status_t hal_sram_deinit(hal_sram_handle_t handle);

#endif /* __HAL_SRAM_H */
