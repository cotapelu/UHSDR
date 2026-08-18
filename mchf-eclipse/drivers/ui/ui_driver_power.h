/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     ui_driver_power.h                                               **
 **  Description:   Power management and configuration handling for UI driver        **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/
#ifndef __UI_DRIVER_POWER_H
#define __UI_DRIVER_POWER_H

#include "uhsdr_board.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COL_PWR_IND                 		White
#define TEMP_DATA 43

typedef enum
{
	CONFIG_DEFAULTS_KEEP = 0,
	CONFIG_DEFAULTS_LOAD_FREQ,
	CONFIG_DEFAULTS_LOAD_ALL
} CONFIG_DEFAULTS;

bool UiDriver_LoadSavedConfigurationAtStartup(void);
void UiDriver_PowerDownCleanup(bool saveConfiguration);
bool UiDriver_SaveConfiguration(void);
void UiDriver_CreateVoltageDisplay(void);
void UiDriver_DisplayVoltage(void);
bool UiDriver_HandleVoltage(void);
void UiDriver_DisplayTemperature(int temp);
void UiDriver_HandleLoTemperature(void);

#ifdef __cplusplus
}
#endif

#endif /* __UI_DRIVER_POWER_H */
