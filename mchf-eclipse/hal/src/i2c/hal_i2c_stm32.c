/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_i2c_stm32.c                                                 **
 **  Description:   STM32 backend for hal_i2c.h                                     **
 **                 ONLY this file includes vendor HAL I2C headers.                 **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "hal_i2c.h"

#include <stddef.h>

#if defined(STM32F407xx)
#include "stm32f4xx_hal.h"
#elif defined(STM32F767xx)
#include "stm32f7xx_hal.h"
#elif defined(STM32H743xx)
#include "stm32h7xx_hal.h"
#endif

/* -------------------------------------------------------------------------
 * API implementation
 * ------------------------------------------------------------------------- */
hal_status_t hal_i2c_is_device_ready(hal_i2c_handle_t handle, uint16_t addr, uint32_t trials, uint32_t timeout)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_I2C_IsDeviceReady((I2C_HandleTypeDef*)handle, addr, trials, timeout) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_i2c_mem_write(hal_i2c_handle_t handle, uint16_t addr, uint16_t mem_addr, uint16_t mem_addr_size, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || data == NULL || size == 0)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_I2C_Mem_Write((I2C_HandleTypeDef*)handle, addr, mem_addr, mem_addr_size, (uint8_t*)data, size, timeout) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_i2c_mem_read(hal_i2c_handle_t handle, uint16_t addr, uint16_t mem_addr, uint16_t mem_addr_size, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || data == NULL || size == 0)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_I2C_Mem_Read((I2C_HandleTypeDef*)handle, addr, mem_addr, mem_addr_size, data, size, timeout) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_i2c_deinit(hal_i2c_handle_t handle)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    HAL_I2C_DeInit((I2C_HandleTypeDef*)handle);

    return HAL_STATUS_OK;
}

hal_status_t hal_i2c_init(hal_i2c_handle_t handle)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    if (HAL_I2C_Init((I2C_HandleTypeDef*)handle) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_i2c_change_speed(hal_i2c_handle_t handle, uint16_t speed)
{
    if (handle == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    I2C_HandleTypeDef* hi2c = (I2C_HandleTypeDef*)handle;

    HAL_I2C_DeInit(hi2c);

#if defined(STM32F4)
    hi2c->Init.ClockSpeed = (uint32_t)speed * HAL_I2C_BUS_SPEED_MULT;
#else
    {
        uint32_t timing = 0;
        uint32_t speed_hz = (uint32_t)speed * HAL_I2C_BUS_SPEED_MULT;
        if (speed_hz >= 400000)
        {
            timing = 0x20404768;
        }
        else if (speed_hz >= 100000)
        {
#if defined(STM32H7)
            timing = 0x10C0ECFF;
#else
            timing = 0x00303D5B;
#endif
        }
        else
        {
            timing = 0x10C0ECFF;
        }
        hi2c->Init.Timing = timing;
    }
#endif

    if (HAL_I2C_Init(hi2c) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}
