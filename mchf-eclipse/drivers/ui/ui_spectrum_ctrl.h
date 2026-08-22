/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:       ui_spectrum_ctrl.h                                             **
 **  Description:     Spectrum display control API (extracted from ui_driver.c)      **
 **  Last Modified:   2026-08-22                                                     **
 **  Licence:         GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __UI_SPECTRUM_CTRL_H
#define __UI_SPECTRUM_CTRL_H

#include "uhsdr_board.h"

#include "ui_driver.h"  /* T31.1: SpectrumMode_t typedef */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reconfigure spectrum/waterfall layout after size/zoom change.
 *
 * Clears waterfall data and reinitializes spectrum scope if not in menu mode.
 */
void UiDriver_SpectrumChangeLayoutParameters(void);

/**
 * @brief Set spectrum display mode.
 * @param mode Spectrum mode (SPECTRUM_BLANK, SPECTRUM_SCOPE, SPECTRUM_DUAL, etc.)
 */
void UiDriver_SetSpectrumMode(SpectrumMode_t mode);

/**
 * @brief Get current spectrum display mode.
 * @return Current spectrum mode
 */
SpectrumMode_t UiDriver_GetSpectrumMode(void);

#ifdef __cplusplus
}
#endif

#endif /* __UI_SPECTRUM_CTRL_H */
