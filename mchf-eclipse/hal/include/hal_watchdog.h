/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_watchdog.h                                                   **
 **  Description:   Abstract watchdog API for STM32 HAL shim layer                  **
 **                 Product code includes this header; NO vendor HAL includes.      **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __HAL_WATCHDOG_H
#define __HAL_WATCHDOG_H

#include "hal_common.h"

/* -------------------------------------------------------------------------
 * Opaque watchdog handle (maps to IWDG_HandleTypeDef* in backend)
 * ------------------------------------------------------------------------- */
typedef void* hal_watchdog_handle_t;

/* -------------------------------------------------------------------------
 * Abstract watchdog API
 * ------------------------------------------------------------------------- */
hal_status_t hal_watchdog_init(hal_watchdog_handle_t handle);
hal_status_t hal_watchdog_refresh(hal_watchdog_handle_t handle);

#endif /* __HAL_WATCHDOG_H */
