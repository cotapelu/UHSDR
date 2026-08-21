/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     ui_encoder_display.c                                            **
 **  Description:   Encoder display renderer — extracted from ui_driver.c           **
 **                 (T24.1 modularization)                                         **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

// Layout and LCD primitives
#include "uhsdr_board.h"
#include "ui_lcd_layouts.h"
#include "ui_lcd_hy28.h"
#include "ui_encoder_display.h"

void UiDriver_EncoderDisplay(const uint8_t row, const uint8_t column, const char *label, bool encoder_active,
		const char temp[5], uint32_t color)
{
	uint32_t label_color = encoder_active ? Black : Grey1;
	uint32_t bg_color    = encoder_active ? Orange : Grey;
	uint32_t brdr_color  = encoder_active ? Orange : Grey;

	if (ts.Layout->ENCODER_MODE == MODE_HORIZONTAL)
	{
		UiLcdHy28_DrawEmptyRect(
				ts.Layout->ENCODER_IND.x + ENC_COL_W * 2 * column + row * ENC_COL_W + column * Xspacing,
				ts.Layout->ENCODER_IND.y,
				ENC_ROW_H - 2, ENC_COL_W - 2, brdr_color);
		UiLcdHy28_PrintTextCentered(
				(ts.Layout->ENCODER_IND.x + 1 + ENC_COL_W * 2 * column + row * ENC_COL_W + column * Xspacing),
				(ts.Layout->ENCODER_IND.y + 1),
				ENC_COL_W - 3, label,
				label_color, bg_color, 0);
		UiLcdHy28_PrintTextRight(
				(ts.Layout->ENCODER_IND.x + ENC_COL_W - 4 + ENC_COL_W * 2 * column + row * ENC_COL_W + column * Xspacing),
				(ts.Layout->ENCODER_IND.y + 1 + ENC_ROW_2ND_OFF),
				temp, color, Black, 0);
	}
	else
	{
		UiLcdHy28_DrawEmptyRect(
				ts.Layout->ENCODER_IND.x + ENC_COL_W * column,
				ts.Layout->ENCODER_IND.y + row * ENC_ROW_H,
				ENC_ROW_H - 2, ENC_COL_W - 2, brdr_color);
		UiLcdHy28_PrintTextCentered(
				(ts.Layout->ENCODER_IND.x + 1 + ENC_COL_W * column),
				(ts.Layout->ENCODER_IND.y + 1 + row * ENC_ROW_H),
				ENC_COL_W - 3, label,
				label_color, bg_color, 0);
		UiLcdHy28_PrintTextRight(
				(ts.Layout->ENCODER_IND.x + ENC_COL_W - 4 + ENC_COL_W * column),
				(ts.Layout->ENCODER_IND.y + 1 + row * ENC_ROW_H + ENC_ROW_2ND_OFF),
				temp, color, Black, 0);
	}
}
