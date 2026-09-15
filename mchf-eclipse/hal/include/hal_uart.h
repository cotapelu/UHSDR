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

/* -------------------------------------------------------------------------
 * UART configuration constants (cross-platform)
 * ------------------------------------------------------------------------- */
#ifndef UART_WORDLENGTH_8B
#define UART_WORDLENGTH_8B                  ((uint32_t)0x00000000U)
#endif
#ifndef UART_STOPBITS_1
#define UART_STOPBITS_1                     ((uint32_t)0x00000000U)
#endif
#ifndef UART_PARITY_NONE
#define UART_PARITY_NONE                    ((uint32_t)0x00000000U)
#endif
#ifndef UART_HWCONTROL_NONE
#define UART_HWCONTROL_NONE                 ((uint32_t)0x00000000U)
#endif

/* -------------------------------------------------------------------------
 * UART configuration (matches STM32 HAL UART_InitTypeDef essentials)
 * ------------------------------------------------------------------------- */
typedef struct {
    uint32_t baud_rate;
    uint32_t word_length;
    uint32_t stop_bits;
    uint32_t parity;
    uint32_t hw_flow_ctrl;
} hal_uart_config_t;

/* -------------------------------------------------------------------------
 * Interrupt-driven RX support
 * ------------------------------------------------------------------------- */
typedef void (*hal_uart_rx_callback_t)(uint8_t byte);

hal_status_t hal_uart_init(hal_uart_handle_t handle, const hal_uart_config_t* config);
hal_status_t hal_uart_register_rx_callback(hal_uart_handle_t handle, hal_uart_rx_callback_t cb);
hal_status_t hal_uart_start_rx(hal_uart_handle_t handle, uint8_t* buf, uint16_t size);
hal_status_t hal_uart_stop_rx(hal_uart_handle_t handle);

/* -------------------------------------------------------------------------
 * Handle management (static allocation — no heap)
 * ------------------------------------------------------------------------- */
hal_uart_handle_t hal_uart_create(void);

#endif /* __HAL_UART_H */
