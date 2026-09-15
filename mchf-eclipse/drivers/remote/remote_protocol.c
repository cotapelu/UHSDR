/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     remote_protocol.c                                               **
 **  Description:   Remote control packet framing and CRC8-CCITT                    **
 **  Last Modified: 2026-08-28                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "remote_protocol.h"

#include <stddef.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * CRC8-CCITT: poly 0x07, init 0x00
 * ------------------------------------------------------------------------- */
uint8_t Remote_CRC8(const uint8_t *data, uint16_t len)
{
    uint8_t crc = 0x00;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc = crc << 1;
            }
        }
    }
    return crc;
}

/* -------------------------------------------------------------------------
 * Build packet: [START1][START2][LENGTH][CMD][DATA...][CRC8]
 * ------------------------------------------------------------------------- */
hal_status_t Remote_BuildPacket(uint8_t *buf, uint8_t cmd, const uint8_t *payload, uint8_t len)
{
    if (buf == NULL || len > 252) {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    uint8_t payload_len = 1 + len; /* cmd + data */

    buf[0] = 0xAA;
    buf[1] = 0xBB;
    buf[2] = payload_len;
    buf[3] = cmd;

    if (payload != NULL && len > 0) {
        memcpy(&buf[4], payload, len);
    }

    buf[4 + len] = Remote_CRC8(&buf[3], payload_len);

    return HAL_STATUS_OK;
}