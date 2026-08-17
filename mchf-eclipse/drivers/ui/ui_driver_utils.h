/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     ui_driver_utils.h                                               **
 **  Description:   Shared UI helper utilities declarations                         **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __UI_DRIVER_UTILS_H
#define __UI_DRIVER_UTILS_H

#include <stdint.h>

int32_t change_and_limit_int(volatile int32_t val, int32_t change, int32_t min, int32_t max);
uint32_t change_and_limit_uint(volatile uint32_t val, int32_t change, uint32_t min, uint32_t max);
uint32_t change_and_wrap_uint(volatile uint32_t val, int32_t change, uint32_t min, uint32_t max);

void incr_wrap_uint8(volatile uint8_t* ptr, uint8_t min, uint8_t max);
void incr_wrap_uint16(volatile uint16_t* ptr, uint16_t min, uint16_t max);
void decr_wrap_uint8(volatile uint8_t* ptr, uint8_t min, uint8_t max);
void decr_wrap_uint16(volatile uint16_t* ptr, uint16_t min, uint16_t max);

#endif
