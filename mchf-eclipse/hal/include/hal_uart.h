/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_uart.h                                                       **
 **  Description:   Abstract UART API for STM32 HAL shim layer                      **
 **                 Product code includes this header; NO vendor HAL includes.      **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __HAL_UART_H
#define __HAL_UART_H

#include "hal_common.h"

/* -------------------------------------------------------------------------
 * Opaque UART handle (maps to UART_HandleTypeDef* in backend)
 * ------------------------------------------------------------------------- */
typedef void* hal_uart_handle_t;

/* -------------------------------------------------------------------------
 * Abstract UART API
 * ------------------------------------------------------------------------- */
hal_status_t hal_uart_transmit(hal_uart_handle_t handle, const uint8_t* data, uint16_t size, uint32_t timeout);
hal_status_t hal_uart_receive(hal_uart_handle_t handle, uint8_t* data, uint16_t size, uint32_t timeout);

#endif /* __HAL_UART_H */
