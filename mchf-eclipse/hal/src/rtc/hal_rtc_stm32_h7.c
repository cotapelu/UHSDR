/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                                        UHSDR                                    **
**               a powerful firmware for STM32 based SDR transceivers              **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:     hal_rtc_stm32_h7.c                                           **
**  Description:   STM32 HAL backend for the abstract RTC API                     **
**                 This is the only file allowed to include the vendor HAL.       **
**  Last Modified: 2026-10-28                                                      **
**  Licence:       GNU GPLv3                                                       **
************************************************************************************/

#include "hal_rtc.h"

#include <stddef.h>

#include "stm32h7xx_hal.h"

/* The RTC handle is owned by the board support package (basesw .../Inc/rtc.h);
 * the backend fills it in exactly like the product code used to. */
#include "rtc.h"

/* -------------------------------------------------------------------------
 * Low level helpers (vendor registers/macros, kept inside the backend)
 * ------------------------------------------------------------------------- */

/* Reads the time register and releases the shadow registers afterwards.
 *
 * The sub second register is deliberately not read: on STM32F407 silicon
 * reading SSR stops the RTC shadow registers. The date register is read to
 * unlock the time/date shadow registers, as the hardware requires. */
static void hal_rtc_read_time_registers(uint8_t* hours, uint8_t* minutes, uint8_t* seconds)
{
    const uint32_t tmpreg = (uint32_t)(hrtc.Instance->TR & RTC_TR_RESERVED_MASK);

    (void)hrtc.Instance->DR;

    *hours   = (uint8_t)RTC_Bcd2ToByte((uint8_t)((tmpreg & (RTC_TR_HT | RTC_TR_HU)) >> 16U));
    *minutes = (uint8_t)RTC_Bcd2ToByte((uint8_t)((tmpreg & (RTC_TR_MNT | RTC_TR_MNU)) >> 8U));
    *seconds = (uint8_t)RTC_Bcd2ToByte((uint8_t)(tmpreg & (RTC_TR_ST | RTC_TR_SU)));
}

static hal_status_t hal_rtc_fill_init(void)
{
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat     = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv   = 127;
    hrtc.Init.SynchPrediv    = 255;
    hrtc.Init.OutPut         = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType     = RTC_OUTPUT_TYPE_OPENDRAIN;

    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

/* -------------------------------------------------------------------------
 * API implementation
 * ------------------------------------------------------------------------- */
hal_status_t hal_rtc_init(void)
{
    return hal_rtc_fill_init();
}

hal_status_t hal_rtc_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    RTC_TimeTypeDef setTime;

    memset(&setTime, 0, sizeof(setTime));

    /* Binary format, 24 hour mode (matches HAL_RTC_Init) and the store
     * operation the product menu always used. */
    setTime.Hours          = hours;
    setTime.Minutes        = minutes;
    setTime.Seconds        = seconds;
    setTime.TimeFormat     = RTC_HOURFORMAT12_AM;
    setTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    setTime.StoreOperation = RTC_STOREOPERATION_SET;

    if (HAL_RTC_SetTime(&hrtc, &setTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_rtc_get_time(uint8_t* hours, uint8_t* minutes, uint8_t* seconds)
{
    if (hours == NULL || minutes == NULL || seconds == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    hal_rtc_read_time_registers(hours, minutes, seconds);

    return HAL_STATUS_OK;
}

hal_status_t hal_rtc_set_smooth_calib(hal_rtc_calib_pulses_t pulses, uint32_t minus_pulses)
{
    const uint32_t vendor_pulses = (pulses == HAL_RTC_CALIB_PLUS_PULSES)
            ? RTC_SMOOTHCALIB_PLUSPULSES_SET
            : RTC_SMOOTHCALIB_PLUSPULSES_RESET;

    HAL_RTCEx_SetSmoothCalib(&hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, vendor_pulses, minus_pulses);

    return HAL_STATUS_OK;
}

hal_status_t hal_rtc_enable_lse_clock(void)
{
    RCC_OscInitTypeDef oscInit;
    RCC_PeriphCLKInitTypeDef periphClkInit;

    memset(&oscInit, 0, sizeof(oscInit));
    memset(&periphClkInit, 0, sizeof(periphClkInit));

    /* Do not touch the PLL, only enable the low speed external oscillator. */
    oscInit.PLL.PLLState    = RCC_PLL_NONE;
    oscInit.OscillatorType  = RCC_OSCILLATORTYPE_LSE;
    oscInit.LSEState        = RCC_LSE_ON;

    periphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    periphClkInit.RTCClockSelection    = RCC_RTCCLKSOURCE_LSE;

    if (HAL_RCC_OscConfig(&oscInit) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    if (HAL_RCCEx_PeriphCLKConfig(&periphClkInit) != HAL_OK)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }

    __HAL_RCC_RTC_ENABLE();

    return HAL_STATUS_OK;
}

hal_status_t hal_rtc_lse_is_ready(uint8_t* ready)
{
    if (ready == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    *ready = (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) != RESET) ? 1U : 0U;

    return HAL_STATUS_OK;
}

hal_status_t hal_rtc_enable_backup_access(void)
{
    /* H7: the power controller clock is already enabled by the HAL. */

    HAL_PWR_EnableBkUpAccess();

    __HAL_RCC_RTC_ENABLE();

    return HAL_STATUS_OK;
}

hal_status_t hal_rtc_backup_reset_force(void)
{
    __HAL_RCC_BACKUPRESET_FORCE();

    return HAL_STATUS_OK;
}

hal_status_t hal_rtc_bkp_read(uint32_t index, uint32_t* value)
{
    if (value == NULL || index > HAL_RTC_BKP_INDEX_MAX)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    *value = HAL_RTCEx_BKUPRead(&hrtc, index);

    return HAL_STATUS_OK;
}

hal_status_t hal_rtc_bkp_write(uint32_t index, uint32_t value)
{
    if (index > HAL_RTC_BKP_INDEX_MAX)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    HAL_RTCEx_BKUPWrite(&hrtc, index, value);

    return HAL_STATUS_OK;
}
