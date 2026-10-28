/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                                        UHSDR                                    **
**               a powerful firmware for STM32 based SDR transceivers              **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:     hal_rtc.h                                                       **
**  Description:   Abstract RTC/backup-domain API for STM32 HAL shim layer        **
**                 Product code includes this header; NO vendor HAL includes.      **
**  Last Modified: 2026-10-28                                                      **
**  Licence:       GNU GPLv3                                                       **
************************************************************************************/

#ifndef __HAL_RTC_H
#define __HAL_RTC_H

#include "hal_common.h"

/* -------------------------------------------------------------------------
 * Backup domain register index.
 *
 * The product code addresses the backup domain by register index (index 0 is
 * the first backup register, i.e. the vendor RTC_BKP_DR0). The index range is
 * 0..31 on every supported MCU family.
 * ------------------------------------------------------------------------- */
#define HAL_RTC_BKP_INDEX_MIN              0U
#define HAL_RTC_BKP_INDEX_MAX              31U

/* -------------------------------------------------------------------------
 * Smooth calibration pulse direction (vendor
 * RTC_SMOOTHCALIB_PLUSPULSES_SET / RTC_SMOOTHCALIB_PLUSPULSES_RESET).
 * ------------------------------------------------------------------------- */
typedef enum {
    HAL_RTC_CALIB_MINUS_PULSES = 0,
    HAL_RTC_CALIB_PLUS_PULSES  = 1,
} hal_rtc_calib_pulses_t;

/* -------------------------------------------------------------------------
 * Abstract RTC API
 * ------------------------------------------------------------------------- */

/* Initialises the RTC peripheral: 24 hour format, asynchronous prescaler 127,
 * synchronous prescaler 255, output disabled, open drain output polarity. The
 * RTC clock source must already be selected (hal_rtc_enable_lse_clock). */
hal_status_t hal_rtc_init(void);

/* Sets the calendar time (binary format). Hours: 0-23, minutes: 0-59,
 * seconds: 0-59. */
hal_status_t hal_rtc_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds);

/* Reads the calendar time in binary format.
 *
 * Note: the sub-second register is deliberately NOT read. On STM32F407
 * silicon reading the SSR stops the RTC shadow registers and causes further
 * trouble; the product does not use sub-seconds. Reading the date register
 * after the time register releases the shadow registers, as required by the
 * hardware. */
hal_status_t hal_rtc_get_time(uint8_t* hours, uint8_t* minutes, uint8_t* seconds);

/* Smooth calibration with a 32 second period: 'minus_pulses' is the number of
 * RTCCLK pulses to subtract over one calibration period. */
hal_status_t hal_rtc_set_smooth_calib(hal_rtc_calib_pulses_t pulses, uint32_t minus_pulses);

/* Configures the LSE as the RTC clock, selects it as the RTC clock source and
 * enables the RTC peripheral clock. */
hal_status_t hal_rtc_enable_lse_clock(void);

/* Reports whether the LSE is running. 'ready' is set to 1 when the low speed
 * external oscillator is stable. */
hal_status_t hal_rtc_lse_is_ready(uint8_t* ready);

/* Enables write access to the backup domain (power controller clock and
 * backup domain access enable). */
hal_status_t hal_rtc_enable_backup_access(void);

/* Forces a backup domain reset (clears all backup registers and resets the
 * RTC). */
hal_status_t hal_rtc_backup_reset_force(void);

/* Reads one backup domain register. */
hal_status_t hal_rtc_bkp_read(uint32_t index, uint32_t* value);

/* Writes one backup domain register. */
hal_status_t hal_rtc_bkp_write(uint32_t index, uint32_t value);

#endif /* __HAL_RTC_H */
