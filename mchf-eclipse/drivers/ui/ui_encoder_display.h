/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     ui_encoder_display.h                                             **
 **  Description:   Encoder display renderer — extracted from ui_driver.c           **
 **                 (T24.1 modularization)                                         **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef _UI_ENCODER_DISPLAY_H_
#define _UI_ENCODER_DISPLAY_H_

#include <stdint.h>
#include <stdbool.h>

#include "uhsdr_board.h"
#include "ui_lcd_layouts.h"
#include "ui_lcd_hy28.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Render a single encoder value box in the layout-defined encoder strip area.
 *
 * Supports both horizontal and vertical encoder layout modes.
 *
 * @param row            Row index within the encoder grid
 * @param column         Column index within the encoder grid
 * @param label          Static label text (top line of the box)
 * @param encoder_active True if this encoder is being adjusted (highlighted)
 * @param temp           Dynamic value string (right-aligned, bottom line)
 * @param color          Default text color for value
 */
void UiDriver_EncoderDisplay(const uint8_t row, const uint8_t column, const char *label, bool encoder_active,
                             const char temp[5], uint32_t color);

#ifdef __cplusplus
}
#endif

#endif /* _UI_ENCODER_DISPLAY_H_ */
