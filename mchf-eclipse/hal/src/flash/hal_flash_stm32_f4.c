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

#include "stm32f4xx_hal.h"

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
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_flash_program_flashword(uint32_t address, const uint32_t data[HAL_FLASH_WORD_BYTES])
{
    for (uint32_t idx = 0; idx < HAL_FLASH_WORD_BYTES; idx++)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + (idx * 4U), data[idx]) != HAL_OK)
        {
            return HAL_STATUS_ERR_HW_FAILURE;
        }
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_flash_erase_sector(uint32_t bank, uint32_t sector, uint32_t *failed_sector)
{
    FLASH_EraseInitTypeDef eraseOp;
    uint32_t sectorError = 0;

    /* The F4 has a single flash bank, the argument is ignored. */
    (void)bank;

    eraseOp.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseOp.Banks = FLASH_BANK_1;
    eraseOp.Sector = sector;
    eraseOp.NbSectors = 1;
    eraseOp.VoltageRange = VOLTAGE_RANGE_3;

    if (HAL_FLASHEx_Erase(&eraseOp, &sectorError) != HAL_OK)
    {
        if (failed_sector != NULL)
        {
            *failed_sector = sectorError;
        }
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_flash_get_rdp_level(hal_flash_rdp_level_t *level)
{
    FLASH_OBProgramInitTypeDef obInit;

    if (level == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    HAL_FLASHEx_OBGetConfig(&obInit);

    if (obInit.RDPLevel == OB_RDP_LEVEL_0)
    {
        *level = HAL_FLASH_RDP_LEVEL_0;
    }
    else if (obInit.RDPLevel == OB_RDP_LEVEL_1)
    {
        *level = HAL_FLASH_RDP_LEVEL_1;
    }
    else if (obInit.RDPLevel == OB_RDP_LEVEL_2)
    {
        *level = HAL_FLASH_RDP_LEVEL_2;
    }
    else
    {
        *level = HAL_FLASH_RDP_LEVEL_UNKNOWN;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_flash_set_single_bank_mode(void)
{
    /* The F4 has a single flash bank, the option does not exist. */
    return HAL_STATUS_ERR_NOT_SUPPORTED;
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
