/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_flash_stm32.c                                               **
 **  Description:   STM32 backend for hal_flash.h                                   **
 **                 ONLY this file includes vendor HAL flash headers.              **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "hal_flash.h"

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
hal_status_t hal_flash_unlock(void)
{
    if (HAL_FLASH_Unlock() != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_flash_lock(void)
{
    HAL_FLASH_Lock();

    return HAL_STATUS_OK;
}

hal_status_t hal_flash_program_word(uint32_t address, uint32_t data)
{
#if defined(STM32H743xx)
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, data) != HAL_OK)
#else
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data) != HAL_OK)
#endif
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_flash_ob_unlock(void)
{
    if (HAL_FLASH_OB_Unlock() != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_flash_ob_lock(void)
{
    HAL_FLASH_OB_Lock();

    return HAL_STATUS_OK;
}

hal_status_t hal_flash_ob_launch(void)
{
    if (HAL_FLASH_OB_Launch() != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}
