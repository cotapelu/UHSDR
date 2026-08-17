/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     ui_driver_touch.h                                               **
 **  Description:   Touchscreen handling extracted from ui_driver.c                 **
 **  Last Modified:                                                                 **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __UI_DRIVER_TOUCH_H
#define __UI_DRIVER_TOUCH_H

#include "uhsdr_board.h"

/* Touchscreen calibration */
void UiDriver_TouchscreenCalibrationRun(void);
bool UiDriver_TouchscreenCalibration(void);
void UiDriver_DoCrossCheck(int16_t cross[]);

/* Touch processing */
bool UiDriver_CheckTouchRegion(const UiArea_t* tr_p);
bool is_touchscreen_pressed(void);
bool UiDriver_ProcessTouchActions(const touchaction_list_descr_t* tld, bool is_long_press);

/* Touch handlers */
void UiDriver_HandleTouchScreen(bool is_long_press);
void UiDriver_HandleTouchScreenShortPress(bool is_long_press);
void UiDriver_HandleTouchScreenLongPress(bool is_long_press);

#endif /* __UI_DRIVER_TOUCH_H */
