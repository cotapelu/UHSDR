/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_sram_stm32.c                                                **
 **  Description:   STM32 backend for hal_sram.h                                    **
 **                 ONLY this file includes vendor HAL SRAM headers.               **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "hal_sram.h"

#include <stddef.h>

#if defined(STM32F407xx)
#include "stm32f4xx_hal.h"
#include "fsmc.h"
#elif defined(STM32F767xx)
#include "stm32f7xx_hal.h"
#include "fmc.h"
#elif defined(STM32H743xx)
#include "stm32h7xx_hal.h"
#include "fmc.h"
#endif

/* -------------------------------------------------------------------------
 * API implementation
 * ------------------------------------------------------------------------- */
hal_status_t hal_sram_init(hal_sram_handle_t handle)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

#if defined(STM32F407xx)
    MX_FSMC_Init();
#elif defined(STM32F767xx) || defined(STM32H743xx)
    MX_FMC_Init();
#endif

    return HAL_STATUS_OK;
}

hal_status_t hal_sram_deinit(hal_sram_handle_t handle)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    HAL_SRAM_DeInit((SRAM_HandleTypeDef*)handle);

    return HAL_STATUS_OK;
}
