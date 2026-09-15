/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     mock_hal_uart.h                                                  **
 **  Description:   Mock HAL UART for host unit testing                              **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __MOCK_HAL_UART_H
#define __MOCK_HAL_UART_H

#include "hal_uart.h"
#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Mock UART device configuration
 * ------------------------------------------------------------------------- */
#define MOCK_UART_MAX_DEVICES 4
#define MOCK_UART_MAX_TRANSFER 256

typedef struct {
    uint8_t tx_buffer[MOCK_UART_MAX_TRANSFER];
    uint8_t rx_buffer[MOCK_UART_MAX_TRANSFER];
    uint16_t tx_len;
    uint16_t rx_len;
    uint32_t init_count;
    uint32_t transmit_count;
    uint32_t receive_count;
    bool rx_in_progress;
    hal_uart_rx_callback_t rx_callback;
    hal_uart_config_t config;
} mock_uart_device_t;

/* Global mock state */
extern mock_uart_device_t mock_uart_devices[];
extern uint32_t mock_uart_current_handle;

/* -------------------------------------------------------------------------
 * Mock helper functions for tests
 * ------------------------------------------------------------------------- */
void mock_uart_reset(void);
void mock_uart_set_rx_response(uint32_t handle_idx, const uint8_t* data, uint16_t size);
const uint8_t* mock_uart_get_tx_buffer(uint32_t handle_idx, uint16_t* size);
void mock_uart_inject_rx_byte(uint32_t handle_idx, uint8_t byte);
hal_status_t mock_uart_get_init_status(uint32_t handle_idx);

/* -------------------------------------------------------------------------
 * Mock HAL UART API (mirrors hal_uart.h)
 * ------------------------------------------------------------------------- */
hal_uart_handle_t hal_uart_create(void);
hal_status_t hal_uart_init(hal_uart_handle_t handle, const hal_uart_config_t* config);
hal_status_t hal_uart_register_rx_callback(hal_uart_handle_t handle, hal_uart_rx_callback_t cb);
hal_status_t hal_uart_start_rx(hal_uart_handle_t handle, uint8_t* buf, uint16_t size);
hal_status_t hal_uart_stop_rx(hal_uart_handle_t handle);
hal_status_t hal_uart_transmit(hal_uart_handle_t handle, const uint8_t* data, uint16_t size, uint32_t timeout);
hal_status_t hal_uart_receive(hal_uart_handle_t handle, uint8_t* data, uint16_t size, uint32_t timeout);

#endif /* __MOCK_HAL_UART_H */
