/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     mock_hal_i2c.h                                                  **
 **  Description:   Mock HAL I2C for host unit testing                              **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __MOCK_HAL_I2C_H
#define __MOCK_HAL_I2C_H

#include "hal_i2c.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Mock I2C memory configuration
 * ------------------------------------------------------------------------- */
#define MOCK_I2C_MAX_DEVICES 8
#define MOCK_I2C_MAX_MEMORY 4096

typedef struct {
    uint8_t memory[MOCK_I2C_MAX_MEMORY];
    uint16_t mem_size;
    uint8_t device_addr;
    uint32_t init_count;
    uint32_t read_count;
    uint32_t write_count;
    bool device_ready;
} mock_i2c_device_t;

/* Global mock state */
extern mock_i2c_device_t mock_i2c_devices[];
extern uint32_t mock_i2c_current_handle;

/* -------------------------------------------------------------------------
 * Mock helper functions for tests
 * ------------------------------------------------------------------------- */
void mock_i2c_reset(void);
void mock_i2c_add_device(uint32_t handle_idx, uint8_t addr, uint16_t mem_size);
void mock_i2c_set_memory(uint32_t handle_idx, uint16_t offset, const uint8_t* data, uint16_t size);
void mock_i2c_set_device_ready(uint32_t handle_idx, bool ready);
const uint8_t* mock_i2c_get_memory(uint32_t handle_idx, uint16_t* mem_size);

/* -------------------------------------------------------------------------
 * Mock HAL I2C API (mirrors hal_i2c.h)
 * ------------------------------------------------------------------------- */
hal_status_t hal_i2c_is_device_ready(hal_i2c_handle_t handle, uint16_t addr, uint32_t trials, uint32_t timeout);
hal_status_t hal_i2c_mem_write(hal_i2c_handle_t handle, uint16_t addr, uint16_t mem_addr, uint16_t mem_addr_size, const uint8_t* data, uint16_t size, uint32_t timeout);
hal_status_t hal_i2c_mem_read(hal_i2c_handle_t handle, uint16_t addr, uint16_t mem_addr, uint16_t mem_addr_size, uint8_t* data, uint16_t size, uint32_t timeout);
hal_status_t hal_i2c_deinit(hal_i2c_handle_t handle);
hal_status_t hal_i2c_init(hal_i2c_handle_t handle);
hal_status_t hal_i2c_change_speed(hal_i2c_handle_t handle, uint16_t speed);

#endif /* __MOCK_HAL_I2C_H */
