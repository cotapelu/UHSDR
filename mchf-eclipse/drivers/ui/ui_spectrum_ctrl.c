/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:       ui_spectrum_ctrl.c                                             **
 **  Description:     Spectrum display control (extracted from ui_driver.c, T31.1)   **
 **  Last Modified:   2026-08-22                                                     **
 **  Licence:         GNU GPLv3                                                      **
 ************************************************************************************/

#include "ui_spectrum_ctrl.h"
#include "ui_spectrum.h"
#include "audio_driver.h"
#include "uhsdr_board.h"

void UiDriver_SpectrumChangeLayoutParameters()
{
	UiSpectrum_WaterfallClearData();
	AudioDriver_SetProcessingChain(ts.dmod_mode, false);

	if (ts.menu_mode == false)
	{
		UiSpectrum_Init();      // init spectrum scope
	}
}

void UiDriver_SetSpectrumMode(SpectrumMode_t mode)
{
    ts.flags1 = (ts.flags1 & ~(FLAGS1_SCOPE_ENABLED | FLAGS1_WFALL_ENABLED)) |(mode << 7);
}

SpectrumMode_t UiDriver_GetSpectrumMode()
{
    return (ts.flags1 & (FLAGS1_SCOPE_ENABLED | FLAGS1_WFALL_ENABLED))  >> 7;
}
