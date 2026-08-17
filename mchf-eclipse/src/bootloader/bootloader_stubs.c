/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     bootloader_stubs.c                                              **
 **  Description:   Stubs for product-code functions referenced by bootloader UI    **
 **  Last Modified:                                                                 **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "uhsdr_board.h"

/* Stubs for touch-action functions referenced in ui_lcd_layouts.c */
void UiAction_ChangeDebugInfoDisplay(void) {}
void UiAction_ChangeLowerMeterUp(void) {}
void UiAction_ToggleWaterfallScopeDisplay(void) {}
void UiAction_ChangeSpectrumSize(void) {}
void UiAction_ChangeSpectrumZoomLevelDown(void) {}
void UiAction_CheckSpectrumTouchActions(void) {}
void UiAction_ChangeSpectrumZoomLevelUp(void) {}
void UiAction_ChangeFrequencyToNextKhz(void) {}
void UiVk_BndSelVirtualKeys(void) {}
void UiVk_BndFreqSetVirtualKeys(void) {}
void UiVk_ModSelVirtualKeys(void) {}
void UiAction_ChangeDemodMode(void) {}
void UiAction_ChangePowerLevel(void) {}
void UiAction_ChangeAudioSource(void) {}
void UiAction_ChangeBandDownOrUp(void) {}
void UiAction_ChangeBandUpOrDown(void) {}
void UiVk_DSPVirtualKeys(void) {}
void Codec_RestartI2S(void) {}
void UiAction_ChangeFrequencyByTouch(void) {}
void UiAction_ChangeDigitalMode(void) {}
void UiAction_ChangeDynamicTuning(void) {}
