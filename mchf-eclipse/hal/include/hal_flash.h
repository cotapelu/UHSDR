/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_flash.h                                                      **
 **  Description:   Abstract flash API for STM32 HAL shim layer                     **
 **                 Product code includes this header; NO vendor HAL includes.      **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __HAL_FLASH_H
#define __HAL_FLASH_H

#include "hal_common.h"

/* -------------------------------------------------------------------------
 * Flash banks.
 * Only the H7 has more than one bank. On the F4/F7 the bank argument is
 * accepted and ignored, so that the product code stays board independent.
 * ------------------------------------------------------------------------- */
#define HAL_FLASH_BANK_1                0U
#define HAL_FLASH_BANK_2                1U

/* -------------------------------------------------------------------------
 * The H7 programs whole 256 bit (32 byte) flash words and only at 32 byte
 * aligned addresses; the F4/F7 program 32 bit words. The abstraction always
 * speaks "flash word" so that the callers do not have to know the difference.
 * ------------------------------------------------------------------------- */
#define HAL_FLASH_WORD_BYTES            8U

/* -------------------------------------------------------------------------
 * Sector indices. The numbering is identical on all supported families
 * (sector 0 is the first sector of the bank), so the product code can use
 * these constants instead of the vendor FLASH_SECTOR_x defines.
 * ------------------------------------------------------------------------- */
#define HAL_FLASH_SECTOR_0              0U
#define HAL_FLASH_SECTOR_1              1U
#define HAL_FLASH_SECTOR_2              2U
#define HAL_FLASH_SECTOR_3              3U
#define HAL_FLASH_SECTOR_4              4U
#define HAL_FLASH_SECTOR_5              5U
#define HAL_FLASH_SECTOR_6              6U
#define HAL_FLASH_SECTOR_7              7U
#define HAL_FLASH_SECTOR_8              8U
#define HAL_FLASH_SECTOR_9              9U
#define HAL_FLASH_SECTOR_10             10U
#define HAL_FLASH_SECTOR_11             11U
#define HAL_FLASH_SECTOR_12             12U
#define HAL_FLASH_SECTOR_13             13U
#define HAL_FLASH_SECTOR_14             14U
#define HAL_FLASH_SECTOR_15             15U

/* -------------------------------------------------------------------------
 * Normalised read-out-protection level.
 * The raw vendor constants differ between the MCU families (0xAA/0x55/0xCC on
 * F4/F7 and 0xAA00/0x5500/0xCC00 on H7), so the backend maps them onto this
 * enumeration.
 * ------------------------------------------------------------------------- */
typedef enum {
    HAL_FLASH_RDP_LEVEL_0 = 0,
    HAL_FLASH_RDP_LEVEL_1 = 1,
    HAL_FLASH_RDP_LEVEL_2 = 2,
    HAL_FLASH_RDP_LEVEL_UNKNOWN = 3
} hal_flash_rdp_level_t;

/* -------------------------------------------------------------------------
 * Abstract flash API
 * ------------------------------------------------------------------------- */
hal_status_t hal_flash_unlock(void);
hal_status_t hal_flash_lock(void);

/* Programs exactly 32 bits at 'address'. On the H7 this is implemented as a
 * read-modify-write of the containing 256 bit flash word, because the flash
 * controller cannot program a single 32 bit word. */
hal_status_t hal_flash_program_word(uint32_t address, uint32_t data);

/* Programs one whole 256 bit flash word (8 x 32 bit) at 'address'. On the
 * F4/F7 this is eight consecutive 32 bit word programs. */
hal_status_t hal_flash_program_flashword(uint32_t address, const uint32_t data[HAL_FLASH_WORD_BYTES]);

/* Erases a single sector of the given bank. On failure the number of the
 * sector that could not be erased is written to 'failed_sector' (when not
 * NULL). */
hal_status_t hal_flash_erase_sector(uint32_t bank, uint32_t sector, uint32_t *failed_sector);

hal_status_t hal_flash_ob_unlock(void);
hal_status_t hal_flash_ob_lock(void);
hal_status_t hal_flash_ob_launch(void);

/* Reads the current read-out-protection level. */
hal_status_t hal_flash_get_rdp_level(hal_flash_rdp_level_t *level);

/* Configures the flash into single bank mode. Only the F7 supports this
 * option; the F4 and the H7 return HAL_STATUS_ERR_NOT_SUPPORTED. */
hal_status_t hal_flash_set_single_bank_mode(void);

#endif /* __HAL_FLASH_H */
