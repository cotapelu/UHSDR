/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     ui_display_list.c                                               **
 **  Description:   Display list renderer — extracted from ui_driver.c             **
 **                 (T20.3 modularization)                                         **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

// Common hardware & layout
#include "uhsdr_board.h"
#include "ui_lcd_layouts.h"
#include "ui_lcd_hy28.h"
#include "ui_display_list.h"

/*
 * Render a single labeled value box into the left-hand side display area.
 *
 * Supports both horizontal and vertical layout modes.
 *
 * @param row            Row index (0-based) within the box strip
 * @param label          Static label text (always shown, top line)
 * @param encoder_active True if the associated encoder is being adjusted
 * @param text           Dynamic value text (right-aligned or centered, bottom line)
 * @param color          Default text color for label/value
 * @param clr_val        Color used for value text when text_is_value && encoder_active
 * @param text_is_value  If true, value text is right-aligned; otherwise centered
 */
void UiDriver_LeftBoxDisplay(const uint8_t row, const char *label, bool encoder_active,
		const char* text, uint32_t color, uint32_t clr_val, bool text_is_value)
{
	uint32_t label_color = encoder_active ? Black : color;
	uint32_t bg_color    = encoder_active ? Orange : Blue;
	uint32_t brdr_color  = encoder_active ? Orange : Blue;

	uint16_t posX, posY;

	if (ts.Layout->LEFTBOXES_MODE == MODE_HORIZONTAL)
	{
		posX = ts.Layout->LEFTBOXES_IND.x + (row * ts.Layout->LEFTBOXES_IND.w);
		posY = ts.Layout->LEFTBOXES_IND.y;
	}
	else
	{
		posX = ts.Layout->LEFTBOXES_IND.x;
		posY = ts.Layout->LEFTBOXES_IND.y + (row * ts.Layout->LEFTBOXES_IND.h);
	}

	// Outer border
	UiLcdHy28_DrawEmptyRect(posX, posY,
			ts.Layout->LEFTBOXES_IND.h - 2,
			ts.Layout->LEFTBOXES_IND.w - 2, brdr_color);

	// Label (top line)
	UiLcdHy28_PrintTextCentered(posX + 1, posY + 1,
			ts.Layout->LEFTBOXES_IND.w - 3,
			label, label_color, bg_color, 0);

	// Fill value area background
	UiLcdHy28_DrawFullRect(posX + 1,
			posY + 1 + 12,
			ts.Layout->LEFTBOXES_IND.h - 4 - 11,
			ts.Layout->LEFTBOXES_IND.w - 3,
			text_is_value ? Black : bg_color);

	if (text_is_value)
	{
		// Right-aligned value text
		UiLcdHy28_PrintTextRight(
				(posX + ts.Layout->LEFTBOXES_IND.w - 4),
				(posY + 1 + ts.Layout->LEFTBOXES_ROW_2ND_OFF),
				text, clr_val, Black, 0);
	}
	else
	{
		// Centered value text
		UiLcdHy28_PrintTextCentered(
				(posX + 1),
				(posY + 1 + ts.Layout->LEFTBOXES_ROW_2ND_OFF),
				ts.Layout->LEFTBOXES_IND.w - 3,
				text, color, bg_color, 0);
	}
}