/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     ui_driver_remote.c                                              **
 **  Description:   Remote input capture (encoder + buttons) for mcHF remote unit   **
 **                 F4/mcHF only; guarded by UI_BRD_MCHF                           **
 **  Last Modified: 2026-08-28                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "ui_driver_remote.h"
#include "remote.h"
#include "uhsdr_mcu.h"
#include "hal_gpio.h"
#include "board_configs/UHSDR_UI_board.h"
#include "drivers/ui/encoder/ui_rotary.h"
#include "uhsdr_keypad.h"

/* -------------------------------------------------------------------------
 * Phase 44.1: remote input capture for remote unit.
 *
 * Strategy:
 *   - Primary encoder (ENCFREQ) read via UiDriverEncoderRead() TIM interface
 *   - Buttons read via Keypad_Scan() + Keypad_ButtonStates() edge detection
 *   - Events sent via Remote_SendEncoderDelta() / Remote_SendButton()
 * ------------------------------------------------------------------------- */

/* Encoder tracking */
static int32_t last_enc_freq = 0;

/* Button edge tracking */
static uint32_t last_button_states = 0;

/* -------------------------------------------------------------------------
 * Lifecycle
 * ------------------------------------------------------------------------- */
hal_status_t UiDriver_Remote_Init(void)
{
    /* Encoder TIM interface is initialized by UiRotaryFreqEncoderInit() */
    /* in uhsdr_board.c during Board_InitFull(). Just reset tracking. */
    last_enc_freq = 0;

    /* Button states */
    Keypad_Scan();
    last_button_states = Keypad_ButtonStates();

    return HAL_STATUS_OK;
}

/* -------------------------------------------------------------------------
 * Poll: called from Remote_Process() in main loop
 * ------------------------------------------------------------------------- */
void UiDriver_Remote_Poll(void)
{
    int32_t delta;
    uint32_t cur_button_states;
    uint32_t changed;
    uint32_t pressed_now;
    uint32_t released_now;
    int i;

    /* ---- Encoder capture ---- */
    delta = UiDriverEncoderRead(ENCFREQ);
    if (delta != 0)
    {
        /* UiDriverEncoderRead returns -ENCODER_RANGE..+ENCODER_RANGE */
        int8_t sdelta = (int8_t)delta;
        if (sdelta != 0)
        {
            Remote_SendEncoderDelta(sdelta);
        }
    }

    /* ---- Button capture ---- */
    Keypad_Scan();
    cur_button_states = Keypad_ButtonStates();

    changed = cur_button_states ^ last_button_states;
    pressed_now = changed & cur_button_states;
    released_now = changed & last_button_states;

    /* Send press events */
    for (i = 0; i < BUTTON_NUM; i++)
    {
        if (pressed_now & (1U << i))
        {
            Remote_SendButton(i, 1);
        }
    }

    /* Send release events */
    for (i = 0; i < BUTTON_NUM; i++)
    {
        if (released_now & (1U << i))
        {
            Remote_SendButton(i, 0);
        }
    }

    last_button_states = cur_button_states;
}
