/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     ui_driver_remote.h                                               **
 **  Description:   Remote input capture header (mcHF UI pair, F4-only)             **
 **  Last Modified: 2026-08-28                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __UI_DRIVER_REMOTE_H
#define __UI_DRIVER_REMOTE_H

#include <stdint.h>
#include "hal_common.h"

/* -------------------------------------------------------------------------
 * Lifecycle
 * ------------------------------------------------------------------------- */
hal_status_t UiDriver_Remote_Init(void);
void UiDriver_Remote_Poll(void);

#endif /* __UI_DRIVER_REMOTE_H */