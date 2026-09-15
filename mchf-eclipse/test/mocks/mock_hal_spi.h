/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     mock_hal_spi.h                                                  **
 **  Description:   Mock HAL SPI for host unit testing                              **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __MOCK_HAL_SPI_H
#define __MOCK_HAL_SPI_H

#include "hal_spi.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Mock SPI device configuration
 * ------------------------------------------------------------------------- */
#define MOCK_SPI_MAX_DEVICES 4
#define MOCK_SPI_MAX_TRANSFER 1024

typedef struct {
    uint8_t tx_buffer[MOCK_SPI_MAX_TRANSFER];
    uint8_t rx_buffer[MOCK_SPI_MAX_TRANSFER];
    uint16_t tx_len;
    uint16_t rx_len;
    uint32_t init_count;
    uint32_t transmit_count;
    uint32_t receive_count;
    bool enabled;
} mock_spi_device_t;

/* Global mock state */
extern mock_spi_device_t mock_spi_devices[];
extern uint32_t mock_spi_current_handle;

/* -------------------------------------------------------------------------
 * Mock helper functions for tests
 * ------------------------------------------------------------------------- */
void mock_spi_reset(void);
void mock_spi_set_rx_response(uint32_t handle_idx, const uint8_t* data, uint16_t size);
const uint8_t* mock_spi_get_tx_buffer(uint32_t handle_idx, uint16_t* size);

/* -------------------------------------------------------------------------
 * Mock HAL SPI API (mirrors hal_spi.h)
 * ------------------------------------------------------------------------- */
hal_status_t hal_spi_init(hal_spi_handle_t handle);
hal_status_t hal_spi_deinit(hal_spi_handle_t handle);
hal_status_t hal_spi_enable(hal_spi_handle_t handle);
hal_status_t hal_spi_disable(hal_spi_handle_t handle);
hal_status_t hal_spi_transmit(hal_spi_handle_t handle, const uint8_t* data, uint16_t size, uint32_t timeout);
hal_status_t hal_spi_receive(hal_spi_handle_t handle, uint8_t* data, uint16_t size, uint32_t timeout);
hal_status_t hal_spi_transmit_receive(hal_spi_handle_t handle, const uint8_t* tx, uint8_t* rx, uint16_t size, uint32_t timeout);
hal_status_t hal_spi_transmit_dma(hal_spi_handle_t handle, const uint8_t* data, uint16_t size);
hal_status_t hal_spi_receive_dma(hal_spi_handle_t handle, uint8_t* data, uint16_t size);

#endif /* __MOCK_HAL_SPI_H */
