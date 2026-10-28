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

#ifndef HARDWARE_MCHF_RTC_H_
#define HARDWARE_MCHF_RTC_H_

#include <stdint.h>

#define RTC_CALIB_PPM_MAX 488
#define RTC_CALIB_PPM_MIN -487
#define RTC_CALIB_PPM_DEFAULT 0

/* -------------------------------------------------------------------------
 * Backup domain register index used for the "an RTC with battery is
 * present" marker. Index 1 is the second backup register on every supported
 * MCU family (the vendor constant used to be RTC_BKP_DR1).
 * ------------------------------------------------------------------------- */
#define RTC_PRESENCE_BKP_INDEX             1U

/* -------------------------------------------------------------------------
 * Calendar time in the product's own representation.
 *
 * The vendor RTC_TimeTypeDef (with its sub-second, daylight-saving and
 * store-operation fields) is not used in the public API any more; every
 * access goes through hal_rtc.h.
 * ------------------------------------------------------------------------- */
typedef struct {
    uint8_t hours;      /* 0 - 23 */
    uint8_t minutes;    /* 0 - 59 */
    uint8_t seconds;    /* 0 - 59 */
} uhsdr_rtc_time_t;

bool Rtc_isEnabled(void);
void Rtc_FullReset(void);
void Rtc_Start(void);
bool Rtc_SetPpm(int16_t ppm);

/* Reads the calendar time in binary format. */
void Rtc_GetTime(uhsdr_rtc_time_t* time);

/* Writes the calendar time in binary format (all fields are taken from the
 * struct, so the caller fills in a freshly read time and edits one field). */
void Rtc_SetTime(const uhsdr_rtc_time_t* time);

#endif /* HARDWARE_MCHF_RTC_H_ */
