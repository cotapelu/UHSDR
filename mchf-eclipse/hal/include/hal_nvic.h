/*  -*-  mode: c; tab-width: 4; indent-tabs mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                                        UHSDR                                    **
**               a powerful firmware for STM32 based SDR transceivers              **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:     hal_nvic.h                                                      **
**  Description:   Abstract NVIC/interrupt API for STM32 HAL shim layer           **
**                 Product code includes this header; NO vendor HAL includes.      **
**  Last Modified: 2026-10-28                                                      **
**  Licence:       GNU GPLv3                                                       **
************************************************************************************/

#ifndef __HAL_NVIC_H
#define __HAL_NVIC_H

#include "hal_common.h"

/* -------------------------------------------------------------------------
 * Abstract NVIC API.
 *
 * The interrupt numbers are the Cortex-M IRQ numbers; the vendor IRQn_Type
 * enum has identical numeric values on every supported family, so the shim
 * passes the number straight through to the vendor call.
 * ------------------------------------------------------------------------- */

/* Enables an interrupt in the NVIC. */
hal_status_t hal_nvic_enable_irq(uint32_t irq);

/* Sets the preemption priority and sub priority of an interrupt. */
hal_status_t hal_nvic_set_priority(uint32_t irq, uint32_t preempt_prio, uint32_t sub_prio);

#endif /* __HAL_NVIC_H */
