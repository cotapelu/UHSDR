/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_common.h                                                    **
 **  Description:   Common types for HAL shim layer                                 **
 **                 Product code includes this header; NO vendor HAL includes.      **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __HAL_COMMON_H
#define __HAL_COMMON_H

#include <stdint.h>


/* -------------------------------------------------------------------------
 * Common status type for HAL shim APIs
 * ------------------------------------------------------------------------- */
typedef enum {
    HAL_STATUS_OK = 0,
    HAL_STATUS_ERR_TIMEOUT = -1,
    HAL_STATUS_ERR_BUSY = -2,
    HAL_STATUS_ERR_INVALID_PARAM = -3,
    HAL_STATUS_ERR_NOT_SUPPORTED = -4,
    HAL_STATUS_ERR_HW_FAILURE = -5,
} hal_status_t;

#endif /* __HAL_COMMON_H */
