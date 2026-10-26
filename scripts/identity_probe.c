/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                **
 **                                        UHSDR                                   **
 **               a powerful firmware for STM32 based SDR transceivers             **
 **--------------------------------------------------------------------------------**
 **                                                                                **
 **  Description:   Build system identity probe.                                  **
 **                                                                                **
 **                 The board configuration headers own the transceiver identity   **
 **                 (TRX_NAME / TRX_ID). This translation unit is preprocessed   **
 **                 with the real compiler flags of a build configuration so that  **
 **                 scripts/check_identity.sh can inspect the resolved macros      **
 **                 without producing a full firmware image.                       **
 **                                                                                **
 **                 It is never linked into a firmware or bootloader.              **
 **                                                                                **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "uhsdr_board_config.h"
