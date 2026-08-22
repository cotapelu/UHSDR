/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/

// Common
#include "uhsdr_board.h"
#include "uhsdr_hw_i2c.h"
#include "hal_i2c.h"

uint16_t UhsdrHw_I2C_DeviceReady(hal_i2c_handle_t handle, uchar I2CAddr)
{
    return hal_i2c_is_device_ready(handle, I2CAddr, 100, 100) == HAL_STATUS_OK ? 0 : 0xFF00;
}

uint16_t UhsdrHw_I2C_WriteRegister(hal_i2c_handle_t handle, uchar I2CAddr, uint16_t addr, uint16_t addr_size, uchar RegisterValue)
{
    return hal_i2c_mem_write(handle, I2CAddr, addr, addr_size, &RegisterValue, 1, 100) == HAL_STATUS_OK ? 0 : 0xFF00;
}

uint16_t UhsdrHw_I2C_WriteBlock(hal_i2c_handle_t handle, uchar I2CAddr, uint16_t addr, uint16_t addr_size, const uint8_t* data, uint32_t size)
{
    return hal_i2c_mem_write(handle, I2CAddr, addr, addr_size, data, size, 100) == HAL_STATUS_OK ? 0 : 0xFF00;
}

uint16_t UhsdrHw_I2C_ReadRegister(hal_i2c_handle_t handle, uchar I2CAddr, uint16_t addr, uint16_t addr_size, uint8_t *RegisterValue)
{
    return hal_i2c_mem_read(handle, I2CAddr, addr, addr_size, RegisterValue, 1, 100) == HAL_STATUS_OK ? 0 : 0xFF00;
}

uint16_t UhsdrHw_I2C_ReadBlock(hal_i2c_handle_t handle, uchar I2CAddr, uint16_t addr, uint16_t addr_size, uint8_t *data, uint32_t size)
{
    return hal_i2c_mem_read(handle, I2CAddr, addr, addr_size, data, size, 100) == HAL_STATUS_OK ? 0 : 0xFF00;
}
/**
 * @brief init I2C
 * @param speed in Hertz !!!
 */
void UhsdrHw_I2C_ChangeSpeed(hal_i2c_handle_t handle)
{
    hal_i2c_change_speed(handle, ts.i2c_speed[0]);
}



