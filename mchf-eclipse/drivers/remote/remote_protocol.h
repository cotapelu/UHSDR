/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     remote_protocol.h                                               **
 **  Description:   Remote control packet framing and CRC8-CCITT                    **
 **  Last Modified: 2026-08-28                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __REMOTE_PROTOCOL_H
#define __REMOTE_PROTOCOL_H

#include <stdint.h>
#include "hal_common.h"

/* -------------------------------------------------------------------------
 * Packet constants
 * ------------------------------------------------------------------------- */
#define REMOTE_START1        0xAA
#define REMOTE_START2        0xBB
#define REMOTE_MAX_PAYLOAD   252

/* -------------------------------------------------------------------------
 * CRC8-CCITT: poly 0x07, init 0x00
 * ------------------------------------------------------------------------- */
uint8_t Remote_CRC8(const uint8_t *data, uint16_t len);

/* -------------------------------------------------------------------------
 * Build packet: [START1][START2][LENGTH][CMD][DATA...][CRC8]
 * LENGTH includes CMD byte only (payload_len = 1 + data_len)
 * ------------------------------------------------------------------------- */
hal_status_t Remote_BuildPacket(uint8_t *buf, uint8_t cmd, const uint8_t *payload, uint8_t len);

#endif /* __REMOTE_PROTOCOL_H */