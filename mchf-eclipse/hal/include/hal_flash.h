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
 * Abstract flash API
 * ------------------------------------------------------------------------- */
hal_status_t hal_flash_unlock(void);
hal_status_t hal_flash_lock(void);
hal_status_t hal_flash_program_word(uint32_t address, uint32_t data);
hal_status_t hal_flash_ob_unlock(void);
hal_status_t hal_flash_ob_lock(void);
hal_status_t hal_flash_ob_launch(void);

#endif /* __HAL_FLASH_H */
