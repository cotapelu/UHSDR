/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_clock_stm32.c                                               **
 **  Description:   STM32 backend for hal_clock.h                                   **
 **                 ONLY this file includes vendor HAL clock headers.              **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "hal_clock.h"

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
hal_status_t hal_clock_deinit(void)
{
    HAL_RCC_DeInit();

    return HAL_STATUS_OK;
}

hal_status_t hal_suspend_tick(void)
{
    HAL_SuspendTick();

    return HAL_STATUS_OK;
}

hal_status_t hal_resume_tick(void)
{
    HAL_ResumeTick();

    return HAL_STATUS_OK;
}
