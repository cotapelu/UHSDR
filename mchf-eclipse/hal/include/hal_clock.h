/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_clock.h                                                      **
 **  Description:   Abstract clock API for STM32 HAL shim layer                     **
 **                 Product code includes this header; NO vendor HAL includes.      **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __HAL_CLOCK_H
#define __HAL_CLOCK_H

#include "hal_common.h"

/* -------------------------------------------------------------------------
 * Abstract clock API
 * ------------------------------------------------------------------------- */
hal_status_t hal_clock_deinit(void);
hal_status_t hal_suspend_tick(void);
hal_status_t hal_resume_tick(void);

#endif /* __HAL_CLOCK_H */
