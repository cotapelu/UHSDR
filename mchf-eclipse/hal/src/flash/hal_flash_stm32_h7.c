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

#include "stm32h7xx_hal.h"

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
    /* The H7 flash controller can only program whole 256 bit flash words and
     * only at 32 byte aligned addresses. A 32 bit write is therefore a
     * read-modify-write of the flash word that contains the address.
     *
     * The base address is computed by clearing the low 5 bits of the address
     * (address & ~0x1F). The previous implementation used the inverted mask
     * (~0xFFFFFFE0), which reduced the base address to the low 5 bits of the
     * address (0x00..0x1F) and therefore read the flash word from the wrong
     * location entirely.
     */
    const uint32_t aligned = address & ~0x1FU;
    const uint32_t wordIdx = (address - aligned) / 4U;
    const uint32_t* const src = (const uint32_t*)aligned;
    uint32_t word[HAL_FLASH_WORD_BYTES];

    for (uint32_t idx = 0; idx < HAL_FLASH_WORD_BYTES; idx++)
    {
        word[idx] = src[idx];
    }
    word[wordIdx] = data;

    return hal_flash_program_flashword(aligned, word);
}

hal_status_t hal_flash_program_flashword(uint32_t address, const uint32_t data[HAL_FLASH_WORD_BYTES])
{
    /* The vendor API expects the address of the 256 bit payload and writes
     * it to the (32 byte aligned) flash word at 'address'. */
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, (uint32_t)(uintptr_t)data) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_flash_erase_sector(uint32_t bank, uint32_t sector, uint32_t *failed_sector)
{
    FLASH_EraseInitTypeDef eraseOp;
    uint32_t sectorError = 0;

    eraseOp.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseOp.Banks = (bank == HAL_FLASH_BANK_2) ? FLASH_BANK_2 : FLASH_BANK_1;
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
    /* The H7 has two independent banks and no dual bank option. */
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
