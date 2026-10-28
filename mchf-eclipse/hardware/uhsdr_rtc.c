/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassiev - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/
#include "uhsdr_board.h"
#include "uhsdr_rtc.h"

#include "hal_rtc.h"

/* Private macros */
/* Internal status registers for RTC */
/* previously we used DR0 which is also used by the HAL Layer, so we moved to DR1;
 * the current API addresses the backup domain by register index, see
 * RTC_PRESENCE_BKP_INDEX in uhsdr_rtc.h */

#define RTC_PRESENCE_INIT_VAL              0x0001       // if we find this value after power on, we assume battery is available
#define RTC_PRESENCE_OK_VAL                0x0002       // then we set this value to remember a clock is present
//#define RTC_PRESENCE_ACK_VAL               0x0003       // if we find this value after power on, we assume user enabled RTC
//#define RTC_PRESENCE_NACK_VAL              0x0004       // if we find this value after power on, we assume user decided against using RTC
#ifdef USE_RTC_LSE

void Rtc_FullReset() {
    (void)hal_rtc_backup_reset_force();
}

static void Rtc_StartInternal(bool doClock)
{
    if (doClock == true)
    {
        // ok, there is a battery, so let us now start the oscillator
        // and remember that a clock is present
        (void)hal_rtc_enable_lse_clock();
        (void)hal_rtc_bkp_write(RTC_PRESENCE_BKP_INDEX, RTC_PRESENCE_OK_VAL);
    }
    // very first start of rtc
    (void)hal_rtc_init();
}


void Rtc_Start()
{
    Rtc_StartInternal(true);
}

#endif
bool Rtc_isEnabled()
{
    bool retval = false;
#ifdef USE_RTC_LSE

    uint8_t lse_ready = 0;
    uint32_t presence = 0;

    (void)hal_rtc_enable_backup_access();

    (void)hal_rtc_lse_is_ready(&lse_ready);

    if (lse_ready != 0)
    {
        // we detected most likely a running RTC or a system capable of running an RTC since the external oscillator is running;
        retval = true;
    }

    (void)hal_rtc_bkp_read(RTC_PRESENCE_BKP_INDEX, &presence);

    switch(presence)
    {
    case 0:
        // 0 -> cleared backup ram -> no battery or reset
        // if we find the RTC_PRESENCE_INIT_VAL in the backup register next time we boot
        // we know there is a battery present.
        (void)hal_rtc_bkp_write(RTC_PRESENCE_BKP_INDEX, RTC_PRESENCE_INIT_VAL);
        break;
    case RTC_PRESENCE_OK_VAL:
        retval = true;
        ts.vbat_present = true;
        break;
    case RTC_PRESENCE_INIT_VAL:
        ts.vbat_present = true;
        break;
    default:
        // TODO: Anything else is a problem, since who wrote a different value? Not this code!
        break;
    }
#endif
    return retval;
}

bool Rtc_SetPpm(int16_t ppm)
{
    bool retval = false;
    if (ppm >= RTC_CALIB_PPM_MIN && ppm <= RTC_CALIB_PPM_MAX)
    {
        uint32_t calm;
        hal_rtc_calib_pulses_t calp;
        float64_t ppm2pulses = rint((float64_t)ppm * 1.048576); //  = (32 * 32768) / 1000.0000
        if (ppm2pulses <= 0.0) // important, we must make sure to not set calp if 0ppm
        {
            calm = - ppm2pulses;
            calp = HAL_RTC_CALIB_MINUS_PULSES;
        }
        else
        {
            calm = 512 - ppm2pulses;
            if (calm > 512)
            {
                calm = 0;
            }
            calp = HAL_RTC_CALIB_PLUS_PULSES;
        }
        (void)hal_rtc_set_smooth_calib(calp, calm);

        retval = true;
    }
    return retval;
}

/**
  * @brief  Reads the current calendar time in binary format.
  * @param  time: pointer to a uhsdr_rtc_time_t structure filled with the
  *               hours, minutes and seconds of the RTC
  * @note   The sub second register is deliberately not read (see the note in
  *         hal_rtc.h): on STM32F407 silicon reading the SSR stops the RTC
  *         shadow registers and causes also other troubles. The product does
  *         not need sub seconds.
  */
void Rtc_GetTime(uhsdr_rtc_time_t* time)
{
    uint8_t hours = 0, minutes = 0, seconds = 0;

    if (time == NULL)
    {
        return;
    }

    (void)hal_rtc_get_time(&hours, &minutes, &seconds);

    time->hours   = hours;
    time->minutes = minutes;
    time->seconds = seconds;
}

/**
  * @brief  Writes the calendar time.
  * @param  time: pointer to a uhsdr_rtc_time_t structure with the new time in
  *               binary format (hours 0-23, minutes 0-59, seconds 0-59)
  */
void Rtc_SetTime(const uhsdr_rtc_time_t* time)
{
    if (time == NULL)
    {
        return;
    }

    (void)hal_rtc_set_time(time->hours, time->minutes, time->seconds);
}
