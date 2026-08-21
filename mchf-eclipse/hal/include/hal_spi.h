/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_spi.h                                                       **
 **  Description:   Abstract SPI API for STM32 HAL shim layer                       **
 **                 Product code includes this header; NO vendor HAL includes.      **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __HAL_SPI_H
#define __HAL_SPI_H

#include "hal_common.h"

/* -------------------------------------------------------------------------
 * Opaque SPI handle (maps to SPI_HandleTypeDef* in backend)
 * ------------------------------------------------------------------------- */
typedef void* hal_spi_handle_t;

/* -------------------------------------------------------------------------
 * Abstract SPI API
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

#endif /* __HAL_SPI_H */
