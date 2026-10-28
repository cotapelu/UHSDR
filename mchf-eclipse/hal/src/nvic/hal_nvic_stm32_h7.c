/*  -*-  mode: c; tab-width: 4; indent-tabs mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                                        UHSDR                                    **
**               a powerful firmware for STM32 based SDR transceivers              **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:     hal_nvic_stm32_h7.c                                            **
**  Description:   STM32 HAL backend for the abstract NVIC API                    **
**                 This is the only file allowed to include the vendor HAL.       **
**  Last Modified: 2026-10-28                                                      **
**  Licence:       GNU GPLv3                                                       **
************************************************************************************/

#include "hal_nvic.h"

#include <stddef.h>

#include "stm32h7xx_hal.h"

/* -------------------------------------------------------------------------
 * API implementation
 * ------------------------------------------------------------------------- */
hal_status_t hal_nvic_enable_irq(uint32_t irq)
{
    HAL_NVIC_EnableIRQ((IRQn_Type)irq);

    return HAL_STATUS_OK;
}

hal_status_t hal_nvic_set_priority(uint32_t irq, uint32_t preempt_prio, uint32_t sub_prio)
{
    HAL_NVIC_SetPriority((IRQn_Type)irq, preempt_prio, sub_prio);

    return HAL_STATUS_OK;
}
