/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     ui_display_list.h                                               **
 **  Description:   Display list renderer — extracted from ui_driver.c             **
 **                 (T20.3 modularization)                                         **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef _UI_DISPLAY_LIST_H_
#define _UI_DISPLAY_LIST_H_

#include <stdint.h>
#include <stdbool.h>

#include "uhsdr_board.h"
#include "ui_lcd_hy28.h"
#include "ui_lcd_layouts.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Render a single labeled value box into the left-hand side display area.
 *
 * Supports both horizontal and vertical layout modes.
 * Does NOT flicker on update (unlike ad-hoc text redraw in main loop).
 *
 * @param row            Row index (0-based) within the box strip
 * @param label          Static label text (always shown)
 * @param encoder_active True if the associated encoder is being adjusted
 * @param text           Dynamic value text (right-aligned or centered)
 * @param color          Default text color for label/value
 * @param clr_val        Highlight color used when encoder_active and text_is_value
 * @param text_is_value  If true, text is right-aligned and colored on encoder_active
 */
void UiDriver_LeftBoxDisplay(const uint8_t row, const char *label, bool encoder_active,
                             const char* text, uint32_t color, uint32_t clr_val,
                             bool text_is_value);

#ifdef __cplusplus
}
#endif

#endif /* _UI_DISPLAY_LIST_H_ */