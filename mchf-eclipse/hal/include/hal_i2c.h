/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_i2c.h                                                       **
 **  Description:   Abstract I2C API for STM32 HAL shim layer                       **
 **                 Product code includes this header; NO vendor HAL includes.      **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __HAL_I2C_H
#define __HAL_I2C_H

#include "hal_common.h"

/* -------------------------------------------------------------------------
 * Opaque I2C handle (maps to I2C_HandleTypeDef* in backend)
 * ------------------------------------------------------------------------- */
typedef void* hal_i2c_handle_t;

/* -------------------------------------------------------------------------
 * I2C timing multiplier (matches project legacy value)
 * ------------------------------------------------------------------------- */
#define HAL_I2C_BUS_SPEED_MULT   20000

/* -------------------------------------------------------------------------
 * Abstract I2C API
 * ------------------------------------------------------------------------- */
hal_status_t hal_i2c_is_device_ready(hal_i2c_handle_t handle, uint16_t addr, uint32_t trials, uint32_t timeout);
hal_status_t hal_i2c_mem_write(hal_i2c_handle_t handle, uint16_t addr, uint16_t mem_addr, uint16_t mem_addr_size, const uint8_t* data, uint16_t size, uint32_t timeout);
hal_status_t hal_i2c_mem_read(hal_i2c_handle_t handle, uint16_t addr, uint16_t mem_addr, uint16_t mem_addr_size, uint8_t* data, uint16_t size, uint32_t timeout);
hal_status_t hal_i2c_deinit(hal_i2c_handle_t handle);
hal_status_t hal_i2c_init(hal_i2c_handle_t handle);
hal_status_t hal_i2c_change_speed(hal_i2c_handle_t handle, uint16_t speed);

#endif /* __HAL_I2C_H */
