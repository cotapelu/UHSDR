/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     remote.c                                                        **
 **  Description:   Remote control module — mode detection, UART init,             **
 **                 state machine for 2x mcHF UI wired remote control              **
 **  Last Modified: 2026-08-28                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "remote.h"
#include "remote_protocol.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "uhsdr_board.h"
/* Board-agnostic: resolve board config from UI_BRD_* macro (AGENTS.md 5.3).
 * Must NOT name a specific board config directly. */
#include "board_configs/UHSDR_UI_board.h"

/* Board-agnostic UI / RF APIs used by Remote_InjectIntoUI() on every board. */
#include "drivers/ui/ui_driver.h"
#include "drivers/ui/radio_management.h"

/* mcHF UI board specific: remote unit input capture + encoder deltas. */
#ifdef UI_BRD_MCHF
#include "drivers/ui/ui_driver_remote.h"
#include "drivers/ui/encoder/ui_rotary.h"
#endif

#include <string.h>

/* -------------------------------------------------------------------------
 * UART instance (static allocation — no heap)
 * ------------------------------------------------------------------------- */
static hal_uart_handle_t g_remote_uart = NULL;
static uint8_t g_remote_rx_byte = 0;

/* -------------------------------------------------------------------------
 * RX packet parser state
 * ------------------------------------------------------------------------- */
typedef struct {
    uint8_t state;
    uint8_t idx;
    uint8_t len;
    uint8_t crc;
    uint8_t payload[256];
} remote_rx_t;

/* -------------------------------------------------------------------------
 * Command queue for main-unit injection
 * ------------------------------------------------------------------------- */
#define REMOTE_CMD_QUEUE_SIZE 8

typedef struct {
    uint8_t  type;
    int8_t   delta;
    uint8_t  button_id;
    uint8_t  pressed;
    uint32_t freq_hz;
} remote_cmd_t;

static remote_cmd_t remote_cmd_queue[REMOTE_CMD_QUEUE_SIZE];
static uint8_t remote_cmd_head = 0;
static uint8_t remote_cmd_tail = 0;
static uint8_t remote_cmd_count = 0;

static remote_mode_t g_mode = MODE_MAIN;
static remote_state_t g_remote_state = REMOTE_STATE_INIT;
static main_state_t g_main_state = MAIN_STATE_STANDALONE;
static uint32_t g_last_hello_time = 0;
static uint32_t g_last_heartbeat_time = 0;
static remote_rx_t g_rx;

/* -------------------------------------------------------------------------
 * Queue helpers (called from ISR-safe contexts where possible)
 * ------------------------------------------------------------------------- */
static void Remote_Dequeue(remote_cmd_t *cmd);

static hal_status_t Remote_Enqueue(const remote_cmd_t *cmd)
{
    if (remote_cmd_count >= REMOTE_CMD_QUEUE_SIZE) {
        /* queue full — drop oldest */
        remote_cmd_t dummy;
        Remote_Dequeue(&dummy);
    }
    remote_cmd_queue[remote_cmd_head] = *cmd;
    remote_cmd_head = (remote_cmd_head + 1) % REMOTE_CMD_QUEUE_SIZE;
    remote_cmd_count++;
    return HAL_STATUS_OK;
}

static void Remote_Dequeue(remote_cmd_t *cmd)
{
    if (remote_cmd_count == 0) {
        cmd->type = 0xFF; /* empty sentinel */
        return;
    }
    *cmd = remote_cmd_queue[remote_cmd_tail];
    remote_cmd_tail = (remote_cmd_tail + 1) % REMOTE_CMD_QUEUE_SIZE;
    remote_cmd_count--;
}

/* -------------------------------------------------------------------------
 * UART RX byte callback (interrupt context — non-blocking)
 * ------------------------------------------------------------------------- */
static void Remote_RxByte(uint8_t byte);

static void Remote_UartRxCallback(uint8_t byte)
{
    Remote_RxByte(byte);
    /* restart single-byte RX for next interrupt */
    if (g_remote_uart != NULL)
    {
        hal_uart_start_rx(g_remote_uart, &g_remote_rx_byte, 1);
    }
}

/* -------------------------------------------------------------------------
 * TX helpers
 * ------------------------------------------------------------------------- */
static hal_status_t Remote_SendPacket(const uint8_t *buf, uint16_t len)
{
    if (g_remote_uart == NULL)
    {
        return HAL_STATUS_ERR_HW_FAILURE;
    }
    return hal_uart_transmit(g_remote_uart, buf, len, 100);
}

static hal_status_t Remote_SendHello(void)
{
    uint8_t pkt[9];
    uint8_t payload[4] = {0x4D, 0x43, 0x48, 0x5F}; /* "mcHF_" magic */
    hal_status_t st = Remote_BuildPacket(pkt, 0xF0, payload, 4);
    if (st != HAL_STATUS_OK) return st;
    return Remote_SendPacket(pkt, 9);
}

static hal_status_t Remote_SendMainAck(void)
{
    uint8_t pkt[10];
    uint8_t payload[5] = {0x4D, 0x43, 0x48, 0x5F, 0x01}; /* magic + fw_version */
    hal_status_t st = Remote_BuildPacket(pkt, 0xF1, payload, 5);
    if (st != HAL_STATUS_OK) return st;
    return Remote_SendPacket(pkt, 10);
}

static hal_status_t Remote_SendHeartbeat(void)
{
    uint8_t pkt[5];
    hal_status_t st = Remote_BuildPacket(pkt, 0xF2, NULL, 0);
    if (st != HAL_STATUS_OK) return st;
    return Remote_SendPacket(pkt, 5);
}

hal_status_t Remote_SendEncoderDelta(int8_t delta)
{
    uint8_t pkt[6];
    hal_status_t st = Remote_BuildPacket(pkt, 0x01, (const uint8_t *)&delta, 1);
    if (st != HAL_STATUS_OK) return st;
    return Remote_SendPacket(pkt, 6);
}

hal_status_t Remote_SendButton(uint8_t button_id, uint8_t pressed)
{
    uint8_t pkt[7];
    uint8_t payload[2] = {button_id, pressed};
    /* ARCH.md: 0x03=BUTTON_PRESS, 0x04=BUTTON_RELEASE */
    uint8_t cmd = pressed ? 0x03 : 0x04;
    hal_status_t st = Remote_BuildPacket(pkt, cmd, payload, 2);
    if (st != HAL_STATUS_OK) return st;
    return Remote_SendPacket(pkt, 7);
}

/* -------------------------------------------------------------------------
 * Packet processing — enqueue commands for main-loop injection
 * ------------------------------------------------------------------------- */
static void Remote_ProcessPacket(const uint8_t *payload, uint8_t len)
{
    if (len == 0) return;

    uint8_t cmd = payload[0];
    const uint8_t *data = &payload[1];
    uint8_t data_len = len - 1;
    remote_cmd_t c;

    c.type = cmd;
    c.delta = 0;
    c.button_id = 0;
    c.pressed = 0;
    c.freq_hz = 0;

    switch (cmd) {
        case 0x01: /* ENCODER_DELTA */
            if (data_len == 1) {
                c.delta = (int8_t)data[0];
                Remote_Enqueue(&c);
            }
            break;
        case 0x03: /* BUTTON_PRESS */
            if (data_len == 2) {
                c.button_id = data[0];
                c.pressed = 1;
                Remote_Enqueue(&c);
            }
            break;
        case 0x04: /* BUTTON_RELEASE */
            if (data_len == 2) {
                c.button_id = data[0];
                c.pressed = 0;
                Remote_Enqueue(&c);
            }
            break;
        case 0x05: /* PTT_TOGGLE */
            if (data_len == 0) {
                Remote_Enqueue(&c);
            }
            break;
        case 0x06: /* FREQ_SET */
            if (data_len == 4) {
                c.freq_hz = ((uint32_t)data[0] << 24) |
                            ((uint32_t)data[1] << 16) |
                            ((uint32_t)data[2] << 8)  |
                            (uint32_t)data[3];
                Remote_Enqueue(&c);
            }
            break;
        default:
            break;
    }
}

/* -------------------------------------------------------------------------
 * Handshake packet processing (called from Remote_RxByte)
 * ------------------------------------------------------------------------- */
static void Remote_ProcessHandshake(const uint8_t *payload, uint8_t len)
{
    if (len == 0) return;

    uint8_t cmd = payload[0];

    switch (cmd) {
        case 0xF0: /* HELLO — remote unit announces presence */
            if (g_mode == MODE_MAIN && g_main_state == MAIN_STATE_STANDALONE) {
                Remote_SendMainAck();
                g_main_state = MAIN_STATE_REMOTE_ACTIVE;
                g_last_heartbeat_time = ts.sysclock;
            }
            break;
        case 0xF1: /* MAIN_ACK — main unit accepts remote */
            if (g_mode == MODE_REMOTE && g_remote_state == REMOTE_STATE_INIT) {
                g_remote_state = REMOTE_STATE_ACTIVE;
                g_last_heartbeat_time = ts.sysclock;
            }
            break;
        case 0xF2: /* HEARTBEAT — keep-alive from remote */
            if (g_mode == MODE_MAIN && g_main_state == MAIN_STATE_REMOTE_ACTIVE) {
                g_last_heartbeat_time = ts.sysclock;
            }
            break;
        default:
            break;
    }
}

/* -------------------------------------------------------------------------
 * UART RX byte callback (interrupt context — non-blocking)
 * ------------------------------------------------------------------------- */
static void Remote_RxByte(uint8_t byte)
{
    switch (g_rx.state) {
        case 0:
            if (byte == 0xAA) g_rx.state = 1;
            break;
        case 1:
            if (byte == 0xBB) {
                g_rx.state = 2;
            } else if (byte != 0xAA) {
                g_rx.state = 0;
            }
            break;
        case 2:
            g_rx.len = byte;
            g_rx.idx = 0;
            g_rx.crc = 0;
            if (g_rx.len == 0) {
                g_rx.state = 4; /* zero-length payload, jump to CRC */
            } else if (g_rx.len > 252) {
                g_rx.state = 0;
            } else {
                g_rx.state = 3;
            }
            break;
        case 3:
            g_rx.payload[g_rx.idx++] = byte;
            g_rx.crc ^= byte;
            for (uint8_t bit = 0; bit < 8; bit++) {
                if (g_rx.crc & 0x80) {
                    g_rx.crc = (g_rx.crc << 1) ^ 0x07;
                } else {
                    g_rx.crc = g_rx.crc << 1;
                }
            }
            if (g_rx.idx == g_rx.len) {
                g_rx.state = 4;
            }
            break;
        case 4:
            if (byte == g_rx.crc) {
                uint8_t cmd = g_rx.payload[0];
                if (cmd == 0xF0 || cmd == 0xF1 || cmd == 0xF2) {
                    Remote_ProcessHandshake(g_rx.payload, g_rx.len);
                } else {
                    Remote_ProcessPacket(g_rx.payload, g_rx.len);
                }
            }
            g_rx.state = 0;
            break;
    }
}

/* -------------------------------------------------------------------------
 * Mode detection via GPIO jumper
 *
 * REMOTE_MODE_DETECT / REMOTE_MODE_DETECT_PIO are defined per-board in
 * hardware/board_configs/UHSDR_UI_<board>_config.h (see ARCH.md 3.1/3.2).
 * Product code must not hardcode pin numbers (AGENTS.md 9.3).
 * ------------------------------------------------------------------------- */
#ifndef REMOTE_MODE_DETECT
    #error "REMOTE_MODE_DETECT is not defined for this UI board (see board_configs)"
#endif
#ifndef REMOTE_MODE_DETECT_PIO
    #error "REMOTE_MODE_DETECT_PIO is not defined for this UI board (see board_configs)"
#endif

remote_mode_t Remote_DetectMode(void)
{
    hal_gpio_config_t cfg = {
        .pin = REMOTE_MODE_DETECT,
        .mode = HAL_GPIO_MODE_INPUT,
        .pull = HAL_GPIO_PULL_DOWN,
        .speed = HAL_GPIO_SPEED_LOW,
    };
    hal_status_t st = hal_gpio_init(REMOTE_MODE_DETECT_PIO, &cfg);
    if (st != HAL_STATUS_OK) return MODE_MAIN;

    /* Settle delay for pin */
    for (volatile int i = 0; i < 10000; i++);

    if (hal_gpio_read_pin(REMOTE_MODE_DETECT_PIO, REMOTE_MODE_DETECT) == HAL_GPIO_PIN_SET) {
        return MODE_REMOTE;
    }
    return MODE_MAIN;
}

/* -------------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------------- */
remote_mode_t Remote_GetMode(void)
{
    return g_mode;
}

main_state_t Remote_GetMainState(void)
{
    return g_main_state;
}

hal_status_t Remote_Init(void)
{
    g_mode = Remote_DetectMode();
    g_remote_state = REMOTE_STATE_INIT;
    g_main_state = MAIN_STATE_STANDALONE;
    g_last_hello_time = 0;
    g_last_heartbeat_time = 0;

    g_remote_uart = hal_uart_create();
    if (g_remote_uart != NULL)
    {
        hal_uart_config_t cfg = {
            .baud_rate      = 115200,
            .word_length    = UART_WORDLENGTH_8B,
            .stop_bits      = UART_STOPBITS_1,
            .parity         = UART_PARITY_NONE,
            .hw_flow_ctrl   = UART_HWCONTROL_NONE,
        };
        hal_uart_init(g_remote_uart, &cfg);
        hal_uart_register_rx_callback(g_remote_uart, Remote_UartRxCallback);
        hal_uart_start_rx(g_remote_uart, &g_remote_rx_byte, 1);
    }

    return HAL_STATUS_OK;
}

void Remote_Process(void)
{
    if (g_mode == MODE_REMOTE) {
        /* Remote unit state machine */
        switch (g_remote_state) {
            case REMOTE_STATE_INIT:
                if (ts.sysclock - g_last_hello_time >= 10) { /* every 1s */
                    Remote_SendHello();
                    g_last_hello_time = ts.sysclock;
                }
                break;
            case REMOTE_STATE_ACTIVE:
                if (ts.sysclock - g_last_heartbeat_time >= 100) { /* every 1s */
                    Remote_SendHeartbeat();
                    g_last_heartbeat_time = ts.sysclock;
                }
#ifdef UI_BRD_MCHF
                UiDriver_Remote_Poll();
#endif
                break;
            case REMOTE_STATE_ERROR:
                Board_RedLed(LED_STATE_TOGGLE);
                if (ts.sysclock - g_last_hello_time >= 100) {
                    Remote_SendHello();
                    g_last_hello_time = ts.sysclock;
                }
                break;
        }
    } else {
        /* Main unit state machine */
        if (g_main_state == MAIN_STATE_REMOTE_ACTIVE) {
            /* Revert to standalone if heartbeat missing >5s */
            if (ts.sysclock - g_last_heartbeat_time > 500) {
                g_main_state = MAIN_STATE_STANDALONE;
            }
        }
        Remote_InjectIntoUI();
    }
}

/* -------------------------------------------------------------------------
 * Main-side injection: drain queue and apply to local UI/RF state
 * ------------------------------------------------------------------------- */
void Remote_InjectIntoUI(void)
{
    remote_cmd_t cmd;

    while (remote_cmd_tail != remote_cmd_head) {
        Remote_Dequeue(&cmd);

        switch (cmd.type) {
            case 0x01: /* ENCODER_DELTA */
#ifdef UI_BRD_MCHF
                UiDriverEncoderSetDelta(ENCFREQ, cmd.delta);
#endif
                break;
            case 0x03: /* BUTTON_PRESS */
#ifdef UI_BRD_MCHF
                UiDriver_InjectButton(cmd.button_id);
#endif
                break;
            case 0x04: /* BUTTON_RELEASE */
                /* Reset any pending press state from remote button */
#ifdef UI_BRD_MCHF
                ks.button_processed = 0;
                ks.button_just_pressed = 0;
                ks.button_pressed = 0;
#endif
                break;
            case 0x05: /* PTT_TOGGLE */
                RadioManagement_HandlePttOnOff();
                break;
            case 0x06: /* FREQ_SET */
                ts.tune_freq = cmd.freq_hz;
                UiDriver_UpdateFrequency(true, UFM_LARGE);
                break;
            default:
                break;
        }
    }
}
