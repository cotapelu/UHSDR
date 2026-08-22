/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_delay.h                                                      **
 **  Description:   Abstract delay API for STM32 HAL shim layer                     **
 **                 Product code includes this header; NO vendor HAL includes.      **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __HAL_DELAY_H
#define __HAL_DELAY_H

#include "hal_common.h"

/* -------------------------------------------------------------------------
 * Abstract delay API
 * ------------------------------------------------------------------------- */
void hal_delay_ms(uint32_t ms);

#endif /* __HAL_DELAY_H */
