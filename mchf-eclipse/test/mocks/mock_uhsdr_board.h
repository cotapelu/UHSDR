/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     mock_uhsdr_board.h                                              **
 **  Description:   Minimal board / transceiver state stubs for host unit testing    **
 **  Last Modified: 2026-08-24                                                      **
 **  Licence:       GNU GPLv3                                                       **
 **  TODO: Expand TransceiverState stub only when a new test requires additional     **
 **        fields to avoid macro bloat and keep the mock surface minimal.           **
 ************************************************************************************/

#ifndef __MOCK_UHSDR_BOARD_H
#define __MOCK_UHSDR_BOARD_H

#include <stdint.h>

/* Board configuration */
#ifndef UI_BRD_MCHF
#define UI_BRD_MCHF 1
#endif
#ifndef RF_BRD_MCHF
#define RF_BRD_MCHF 1
#endif

/* IQ conversion modes (mirrors ui_configuration.h) */
typedef enum {
    FREQ_IQ_CONV_OFF = 0,
    FREQ_IQ_CONV_P6KHZ = 1,
    FREQ_IQ_CONV_M6KHZ = 2,
    FREQ_IQ_CONV_P12KHZ = 3,
    FREQ_IQ_CONV_M12KHZ = 4,
    FREQ_IQ_CONV_MODE_MAX = 4
} freq_iq_conv_mode_t;

/* Minimal TransceiverState stub — add fields intentionally per test need */
typedef struct {
    uint8_t iq_freq_mode;
    uint32_t flags1;
    uint8_t menu_mode;
    uint8_t dmod_mode;
} TransceiverState;

/* Global transceiver state declaration */
extern TransceiverState ts;

#endif /* __MOCK_UHSDR_BOARD_H */