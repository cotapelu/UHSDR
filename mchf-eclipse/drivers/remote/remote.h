/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     remote.h                                                        **
 **  Description:   Remote control module header (mcHF UI pair over UART)           **
 **  Last Modified: 2026-08-28                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __REMOTE_H
#define __REMOTE_H

#include <stdint.h>
#include "hal_common.h"
#include "hal_uart.h"

/* -------------------------------------------------------------------------
 * Remote mode enumeration
 * ------------------------------------------------------------------------- */
typedef enum {
    MODE_MAIN = 0,    /* Main radio unit — listens for remote commands */
    MODE_REMOTE = 1,  /* Remote control unit — sends commands to main */
} remote_mode_t;

/* -------------------------------------------------------------------------
 * Remote unit state machine
 * ------------------------------------------------------------------------- */
typedef enum {
    REMOTE_STATE_INIT = 0,
    REMOTE_STATE_ACTIVE,
    REMOTE_STATE_ERROR
} remote_state_t;

/* -------------------------------------------------------------------------
 * Main unit state machine
 * ------------------------------------------------------------------------- */
typedef enum {
    MAIN_STATE_STANDALONE = 0,
    MAIN_STATE_REMOTE_ACTIVE
} main_state_t;

/* -------------------------------------------------------------------------
 * Mode detection (GPIO jumper, see board_configs)
 * ------------------------------------------------------------------------- */
remote_mode_t Remote_DetectMode(void);
remote_mode_t Remote_GetMode(void);
main_state_t Remote_GetMainState(void);

/* -------------------------------------------------------------------------
 * Lifecycle
 * ------------------------------------------------------------------------- */
hal_status_t Remote_Init(void);
void Remote_Process(void);
void Remote_InjectIntoUI(void);

/* -------------------------------------------------------------------------
 * Command transmission (remote side)
 * ------------------------------------------------------------------------- */
hal_status_t Remote_SendEncoderDelta(int8_t delta);
hal_status_t Remote_SendButton(uint8_t button_id, uint8_t pressed);

#endif /* __REMOTE_H */