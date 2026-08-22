/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_delay_stm32.c                                               **
 **  Description:   STM32 backend for hal_delay.h                                   **
 **                 ONLY this file includes vendor HAL delay headers.              **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "hal_delay.h"

#include <stddef.h>

#if defined(STM32F407xx)
#include "stm32f4xx_hal.h"
#elif defined(STM32F767xx)
#include "stm32f7xx_hal.h"
#elif defined(STM32H743xx)
#include "stm32h7xx_hal.h"
#endif

/* -------------------------------------------------------------------------
 * API implementation
 * ------------------------------------------------------------------------- */
void hal_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}
