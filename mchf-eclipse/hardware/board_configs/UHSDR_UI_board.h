/*  -*-  mode: c; tab-width: 4; indent-tabs: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **  File name:     UHSDR_UI_board.h                                                **
 **  Description:   Central board-config selector.                                 **
 **                                                                                 **
 **                 Product code MUST NOT include a specific board config header  **
 **                 directly (AGENTS.md 5.3). Instead include this selector, which**
 **                 resolves to the correct board based on the UI_BRD_* macro     **
 **                 passed by the build system.                                    **
 **                                                                                 **
 **                 If no board macro is defined, a hard #error is raised so that  **
 **                 misconfigured builds fail at compile time rather than silently **
 **                 building a firmware with an undefined pin map.                 **
 **                                                                                 **
 **                 This also stops a board config whose MCU guard does not match **
 **                 (e.g. mcHF config pulled into an F7/H7 build) from being       **
 **                 compiled at all.                                                **
 **                                                                                 **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __UHSDR_UI_BOARD_CONFIG_H
#define __UHSDR_UI_BOARD_CONFIG_H

#if defined(UI_BRD_MCHF)
    #include "UHSDR_UI_mchf_config.h"
#elif defined(UI_BRD_OVI40)
    #include "UHSDR_UI_ovi40_config.h"
#else
    #error "No UI board selected. Define UI_BRD_MCHF or UI_BRD_OVI40 (see Makefile CONFIGFLAGS)."
#endif

#endif /* __UHSDR_UI_BOARD_CONFIG_H */
