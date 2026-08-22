/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_dma.h                                                       **
 **  Description:   Abstract DMA API for STM32 HAL shim layer                       **
 **                 Product code includes this header; NO vendor HAL includes.      **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __HAL_DMA_H
#define __HAL_DMA_H

#include "hal_common.h"

/* -------------------------------------------------------------------------
 * Opaque DMA handle (maps to DMA_HandleTypeDef* in backend)
 * ------------------------------------------------------------------------- */
typedef void* hal_dma_handle_t;

/* -------------------------------------------------------------------------
 * Abstract DMA API
 * ------------------------------------------------------------------------- */
hal_status_t hal_dma_init(hal_dma_handle_t handle);
hal_status_t hal_dma_deinit(hal_dma_handle_t handle);
hal_status_t hal_dma_start(hal_dma_handle_t handle, uint32_t src, uint32_t dst, uint32_t len);
hal_status_t hal_dma_stop(hal_dma_handle_t handle);
hal_status_t hal_dma_wait_complete(hal_dma_handle_t handle, uint32_t timeout);

#endif /* __HAL_DMA_H */
