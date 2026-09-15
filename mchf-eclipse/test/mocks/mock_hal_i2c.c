/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     mock_hal_i2c.c                                                  **
 **  Description:   Mock HAL I2C implementation for host unit testing               **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "mock_hal_i2c.h"
#include <stdbool.h>

/* Global mock state */
mock_i2c_device_t mock_i2c_devices[MOCK_I2C_MAX_DEVICES];
uint32_t mock_i2c_current_handle;

void mock_i2c_reset(void)
{
    memset(mock_i2c_devices, 0, sizeof(mock_i2c_devices));
    mock_i2c_current_handle = 0;
}

void mock_i2c_add_device(uint32_t handle_idx, uint8_t addr, uint16_t mem_size)
{
    if (handle_idx < MOCK_I2C_MAX_DEVICES)
    {
        memset(mock_i2c_devices[handle_idx].memory, 0, MOCK_I2C_MAX_MEMORY);
        mock_i2c_devices[handle_idx].mem_size = mem_size;
        mock_i2c_devices[handle_idx].device_addr = addr;
        mock_i2c_devices[handle_idx].device_ready = true;
    }
}

void mock_i2c_set_memory(uint32_t handle_idx, uint16_t offset, const uint8_t* data, uint16_t size)
{
    if (handle_idx < MOCK_I2C_MAX_DEVICES && offset + size <= mock_i2c_devices[handle_idx].mem_size)
    {
        memcpy(&mock_i2c_devices[handle_idx].memory[offset], data, size);
    }
}

void mock_i2c_set_device_ready(uint32_t handle_idx, bool ready)
{
    if (handle_idx < MOCK_I2C_MAX_DEVICES)
    {
        mock_i2c_devices[handle_idx].device_ready = ready;
    }
}

const uint8_t* mock_i2c_get_memory(uint32_t handle_idx, uint16_t* mem_size)
{
    if (handle_idx < MOCK_I2C_MAX_DEVICES)
    {
        if (mem_size) *mem_size = mock_i2c_devices[handle_idx].mem_size;
        return mock_i2c_devices[handle_idx].memory;
    }
    if (mem_size) *mem_size = 0;
    return NULL;
}

hal_status_t hal_i2c_is_device_ready(hal_i2c_handle_t handle, uint16_t addr, uint32_t trials, uint32_t timeout)
{
    (void)addr;
    (void)trials;
    (void)timeout;
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle;
    if (handle_idx >= MOCK_I2C_MAX_DEVICES)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_i2c_devices[handle_idx].read_count++;
    return mock_i2c_devices[handle_idx].device_ready ? HAL_STATUS_OK : HAL_STATUS_ERR_HW_FAILURE;
}

hal_status_t hal_i2c_mem_write(hal_i2c_handle_t handle, uint16_t addr, uint16_t mem_addr, uint16_t mem_addr_size, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    (void)addr;
    (void)mem_addr_size;
    (void)timeout;
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle;
    if (handle_idx >= MOCK_I2C_MAX_DEVICES || data == NULL || mem_addr + size > mock_i2c_devices[handle_idx].mem_size)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_i2c_devices[handle_idx].write_count++;
    memcpy(&mock_i2c_devices[handle_idx].memory[mem_addr], data, size);
    return HAL_STATUS_OK;
}

hal_status_t hal_i2c_mem_read(hal_i2c_handle_t handle, uint16_t addr, uint16_t mem_addr, uint16_t mem_addr_size, uint8_t* data, uint16_t size, uint32_t timeout)
{
    (void)addr;
    (void)mem_addr_size;
    (void)timeout;
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle;
    if (handle_idx >= MOCK_I2C_MAX_DEVICES || data == NULL || mem_addr + size > mock_i2c_devices[handle_idx].mem_size)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_i2c_devices[handle_idx].read_count++;
    memcpy(data, &mock_i2c_devices[handle_idx].memory[mem_addr], size);
    return HAL_STATUS_OK;
}

hal_status_t hal_i2c_deinit(hal_i2c_handle_t handle)
{
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle;
    if (handle_idx >= MOCK_I2C_MAX_DEVICES)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    return HAL_STATUS_OK;
}

hal_status_t hal_i2c_init(hal_i2c_handle_t handle)
{
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle;
    if (handle_idx >= MOCK_I2C_MAX_DEVICES)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_i2c_devices[handle_idx].init_count++;
    return HAL_STATUS_OK;
}

hal_status_t hal_i2c_change_speed(hal_i2c_handle_t handle, uint16_t speed)
{
    (void)speed;
    uint32_t handle_idx = (uint32_t)(uintptr_t)handle;
    if (handle_idx >= MOCK_I2C_MAX_DEVICES)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    return HAL_STATUS_OK;
}
