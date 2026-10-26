# UHSDR Remote Control Architecture
# Kiến trúc Hệ thống Điều khiển Từ xa mcHF

**Version:** 1.0  
**Date:** 2026-08-28  
**Author:** UHSDR Project  
**Status:** Draft — Implementation Pending

---

## 1. Executive Summary

Tài liệu này mô tả kiến trức hệ thống biến **2 bo mcHF UI board (STM32F407)** thành một hệ thống radio có khả năng **điều khiển từ xa có dây**.

**Thiết kế cốt lõi:**
- Cùng 1 firmware binary, tự động phát hiện mode tại boot qua **GPIO jumper**
- **Main Unit:** mcHF UI + RF board, chạy firmware mode `MAIN`
- **Remote Unit:** mcHF UI standalone, chạy firmware mode `REMOTE`
- Giao tiếp: **UART (USART2/6) @ 115200 baud, 8N1**
- Protocol: **Packet-based, binary, CRC8 verified**
- Latency budget: **< 5ms** end-to-end cho control commands
- Không cần sửa phần firmware máy mẹ (RF side) — thứ ba lo

---

## 2. System Architecture

### 2.1 High-Level Block Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                                                                             │
│   ┌──────────────────────┐                    ┌──────────────────────────┐   │
│   │    REMOTE CONTROL     │                    │      MAIN RADIO UNIT     │   │
│   │       UNIT            │                    │                          │   │
│   │                       │                    │                          │   │
│   │  ┌─────────────────┐  │                    │  ┌──────────────────┐   │   │
│   │  │ mcHF UI Board   │  │    UART Cable      │  │  mcHF UI Board    │   │   │
│   │  │ STM32F407       │  │◄──────────────────►│  │  STM32F407        │   │   │
│   │  │                 │  │  115200 baud 8N1   │  │                   │   │   │
│   │  │ ┌─────────────┐ │  │                    │  │ ┌───────────────┐ │   │   │
│   │  │ │ LCD Display │ │  │                    │  │ │  LCD Display  │ │   │   │
│   │  │ └─────────────┘ │  │                    │  │ └───────────────┘ │   │   │
│   │  │ ┌─────────────┐ │  │                    │  │ ┌───────────────┐ │   │   │
│   │  │ │  Encoder    │ │  │                    │  │ │   Encoder     │ │   │   │
│   │  │ └─────────────┘ │  │                    │  │ └───────────────┘ │   │   │
│   │  │ ┌─────────────┐ │  │                    │  │ ┌───────────────┐ │   │   │
│   │  │ │  Buttons    │ │  │                    │  │ │   Buttons     │ │   │   │
│   │  │ └─────────────┘ │  │                    │  │ └───────────────┘ │   │   │
│   │  │                 │  │                    │  │                   │   │   │
│   │  │ Firmware:       │  │                    │  │ Firmware:         │   │   │
│   │  │ - Remote Mode   │  │                    │  │ - Main Mode       │   │   │
│   │  │ - Capture input │  │                    │  │ - Process remote  │   │   │
│   │  │ - Send cmds     │  │                    │  │   cmds as local   │   │   │
│   │  │ - RX display    │  │                    │  │ - Send display    │   │   │
│   │  │   updates        │  │                    │  │   updates         │   │   │
│   │  └─────────────────┘  │                    │  └──────────────────┘   │   │
│   │           │            │                    │            │            │   │
│   │   ┌───────▼────────┐  │                    │   ┌────────▼────────┐  │   │
│   │   │  Power (5V/3.3V)│  │                    │   │ RF Board (TX/RX) │  │   │
│   │   │  via USB/Barrel │  │                    │   │ - PA, Filter,    │  │   │
│   │   └────────────────┘  │                    │   │   PLL, etc.      │  │   │
│   │                       │                    │   └─────────────────┘  │   │
│   └───────────────────────┘                    └──────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 2.2 Mode Detection

Boot-time GPIO detection quyết định firmware mode:

```
PB5 (example) — Mode Detection Pin
     │
     ├──► Jumper to GND  →  ts.mode = MODE_MAIN  (Main Radio Unit)
     │
     └──► Jumper to 3.3V  →  ts.mode = MODE_REMOTE  (Remote Control Unit)
```

**Lý do chọn PB5:**
- Free pin trên mcHF UI header
- Không conflict với:
  - LCD (SPI2)
  - Touchscreen (SPI2 + TP_IRQ)
  - I2C (PB6/PB10 cho I2C1, PB7/PB11 cho I2C2)
  - Encoder (PA0-PA3)
  - Buttons (matrix)
  - LEDs (PA4/PA5)

---

## 3. Hardware Architecture

### 3.1 mcHF UI Board Pinout (Relevant Pins)

| Pin | Port | Function | Remote Mode | Main Mode |
|-----|------|----------|-------------|-----------|
| PB5 | GPIOB | **Mode Detect** | Input (PU/PD) | Input (PU/PD) |
| PA2 | GPIOA | **USART2_TX** | AF7 (UART TX) | AF7 (UART RX listener) |
| PA3 | GPIOA | **USART2_RX** | AF7 (UART RX) | AF7 (UART TX responder) |
| PA0 | GPIOA | Encoder A | Local input | Local input |
| PA1 | GPIOA | Encoder B | Local input | Local input |
| PA4 | GPIOA | Green LED | Status TX | Status RX |
| PA5 | GPIOA | Red LED | Error | Error |
| PB13 | GPIOB | SPI2_SCK (LCD) | Local LCD | Local LCD |
| PB14 | GPIOB | SPI2_MISO | Local LCD | Local LCD |
| PB15 | GPIOB | SPI2_MOSI | Local LCD | Local LCD |
| PA8 | GPIOA | Touch IRQ | Disabled | Normal |

### 3.2 UART Hardware Config

**USART2 Configuration:**

| Parameter | Value | Notes |
|-----------|-------|-------|
| Peripheral | USART2 | APB1 clock @ 42MHz (F4) |
| Baud Rate | 115200 | Sufficient for control latency |
| Word Length | 8 bits | Standard |
| Stop Bits | 1 | Standard |
| Parity | None | Error detection via CRC8 |
| Flow Control | None | Simple point-to-point |
| Direction | Full duplex | TX + RX simultaneously |
| Interrupt Priority | 7 (low) | Below audio ISR (1-3) |

**DMA Consideration:**
- UART TX: Không cần DMA (packet nhỏ, interrupt-driven đủ)
- UART RX: Có thể dùng IDLE line detection interrupt để detect packet start

---

## 4. Software Architecture

### 4.1 Firmware Modes

```
┌─────────────────────────────────────────────────────────────────────┐
│                        FIRMWARE MODE TREE                           │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   Boot                                                              │
│     │                                                               │
│     ├── Read GPIO PB5                                              │
│     │                                                               │
│     ├─ LOW (GND) ──────► MODE_MAIN                                 │
│     │                   - Full radio functionality                  │
│     │                   - UART: Remote Listener (RX commands)      │
│     │                   - Encoder/Buttons: LOCAL only              │
│     │                   - Display: Show local + remote state       │
│     │                                                              │
│     └─ HIGH (3.3V) ────► MODE_REMOTE                              │
│                          - RF disabled (no TX/RX)                  │
│                          - UART: Remote Controller (TX commands)  │
│                          - Encoder/Buttons: CAPTURE & SEND         │
│                          - Display: Show received main display     │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 4.2 Module Structure (Remote Control)

```
mchf-eclipse/
├── drivers/
│   ├── remote/                    ← NEW MODULE
│   │   ├── remote.h
│   │   ├── remote.c               ← Mode detection, UART init, state machine
│   │   ├── remote_protocol.c      ← Packet framing, CRC, encode/decode
│   │   ├── remote_cmd.c           ← Command handlers (encoder, buttons)
│   │   └── remote_display.c       ← Display update from main → remote LCD
│   │
│   ├── cat/                       ← Extended for UART CAT mode
│   │   └── cat_driver.c
│   │
│   └── ui/                        ← Modified for remote input hook
│       ├── ui_driver.c
│       └── ui_driver_remote.c     ← NEW: input capture for remote mode
│
├── hardware/
│   ├── uhsdr_board.c              ← Add Board_RemoteMode_Init()
│   └── board_configs/
│       └── UHSDR_UI_mchf_config.h ← Add REMOTE_MODE pin defines
│
└── src/
    └── uhsdr_main.c               ← Hook mode detection at boot
```

### 4.3 State Machine: Remote Controller

```
┌────────────┐
│   BOOT      │
│  (detect)   │
└─────┬──────┘
      │
      ├─ MODE_MAIN ──────► MAIN_IDLE
      │                     ├── Listen UART for remote commands
      │                     ├── Process as if local input
      │                     └── Broadcast display state
      │
      └─ MODE_REMOTE ────► REMOTE_INIT
                            ├── Init UART @ 115200
                            ├── Send HELLO packet
                            └── Wait MAIN_ACK
                                 │
                                 ├─ Timeout/No ACK ──► REMOTE_ERROR (blink LED)
                                 │
                                 └─ ACK received ────► REMOTE_ACTIVE
                                                       ├── Capture encoder → send ENCODER_CMD
                                                       ├── Capture buttons → send BUTTON_CMD
                                                       ├── RX display → update LCD
                                                       └── Heartbeat @ 1Hz
```

### 4.4 State Machine: Main Listener

```
┌────────────┐
│   BOOT      │
│  (detect)   │
└─────┬──────┘
      │
      └─ MODE_MAIN ──────► MAIN_INIT
                            ├── Init UART @ 115200
                            └── Wait HELLO from remote
                                 │
                                 ├─ Timeout/No HELLO ──► MAIN_STANDALONE (normal op)
                                 │
                                 └─ HELLO received ────► MAIN_REMOTE_ACTIVE
                                                       ├── Send MAIN_ACK
                                                       ├── Parse remote commands
                                                       ├── Inject into ui_driver.c
                                                       └── Periodic display sync
```

---

## 5. Communication Protocol

### 5.1 Packet Format

```
┌──────────┬──────────┬──────────┬──────────────┬────────┐
│  START1  │  START2  │  LENGTH  │   PAYLOAD    │  CRC8  │
│  0xAA    │  0xBB    │  1 byte  │  0-252 bytes │ 1 byte │
└──────────┴──────────┴──────────┴──────────────┴────────┘

- START1/START2: 0xAA, 0xBB (sync word)
- LENGTH: payload length (0 = heartbeat ack)
- PAYLOAD: [CMD][DATA...]
- CRC8: CRC-8-CCITT over PAYLOAD only

Maximum packet size: 256 bytes
Minimum packet size: 4 bytes (START1 + START2 + LENGTH + CRC8)
```

### 5.2 Command Set

#### Remote → Main (0x01–0x0F)

| CMD | Code | Payload | Description |
|-----|------|---------|-------------|
| ENCODER_DELTA | 0x01 | 1 byte: int8_t | Encoder rotation delta (+1/-1) |
| ENCODER_BTN | 0x02 | 1 byte: 0=release, 1=press | Encoder push button |
| BUTTON_PRESS | 0x03 | 1 byte: button_id | Momentary button press |
| BUTTON_RELEASE | 0x04 | 1 byte: button_id | Momentary button release |
| PTT_TOGGLE | 0x05 | 0 bytes | Toggle PTT state |
| FREQ_SET | 0x06 | 4 bytes: uint32_t Hz | Direct frequency set |
| POWER_SET | 0x07 | 1 byte: 0-100 | Power level % |
| MODE_SET | … | … | (reserved) |

#### Main → Remote (0x80–0x8F)

| CMD | Code | Payload | Description |
|-----|------|---------|-------------|
| DISPLAY_LINE | 0x80 | 1 byte: line_id + data | Update specific LCD line |
| METER_UPDATE | 0x81 | 1 byte: meter_id + 2 bytes: value | Update S-meter, power, etc. |
| STATUS_UPDATE | 0x82 | N bytes: packed status | VFO A/B, mode, band, PTT |
| DISPLAY_CLEAR | 0x83 | 0 bytes | Clear entire display |
| DISPLAY_FULL | 0x84 | N bytes: full framebuffer | Full display sync (initial) |
| HEARTBEAT | 0x85 | 0 bytes | Keep-alive |
| ERROR | 0x8F | 1 byte: error_code | Error notification |

#### Handshake Commands (Both)

| CMD | Code | Payload | Description |
|-----|------|---------|-------------|
| HELLO | 0xF0 | 4 bytes: magic 0x4D43485F (mcHF_) | Remote announces presence |
| MAIN_ACK | 0xF1 | 4 bytes: magic + 1 byte: fw_version | Main accepts remote |
| PING | 0xF2 | 0 bytes | Latency measurement |
| PONG | 0xF3 | 4 bytes: uint32_t timestamp_ms | Ping reply |

### 5.3 Payload Structures

```c
// Remote → Main
typedef struct __attribute__((packed)) {
    uint8_t seq;           // Sequence number
    int8_t encoder_delta;  // -128 to +127
} remote_encoder_cmd_t;

typedef struct __attribute__((packed)) {
    uint8_t button_id;
    uint8_t pressed;       // 0=release, 1=press
} remote_button_cmd_t;

// Main → Remote
typedef struct __attribute__((packed)) {
    uint8_t line_id;       // 0-7, LCD line number
    uint8_t length;        // Data length
    uint8_t data[32];      // UTF-8 or raw pixels
} remote_display_line_t;

typedef struct __attribute__((packed)) {
    uint8_t vfo_active;    // 0=VFO_A, 1=VFO_B
    uint32_t freq_hz;      // Current frequency
    uint8_t dmod_mode;     // DEMOD_LSB/USB/AM/FM/...
    uint8_t band;          // 160m/80m/40m/...
    uint8_t ptt;           // 0=RX, 1=TX
    uint8_t power_level;   // 0-100%
    int8_t s_meter;        // -80 to +10 dBm
} remote_status_t;
```

### 5.4 CRC8 Implementation

```c
// CRC-8-CCITT: poly 0x07, init 0x00
uint8_t crc8_ccitt(const uint8_t *data, uint16_t len) {
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
```

---

## 6. Detailed Design: Boot Sequence

### 6.1 Remote Unit Boot Flow

```
1. Reset → SystemInit → main()
2. Board_InitMinimal()
   ├── GPIO init (PB5 as mode detect)
   ├── Read PB5 → ts.remote_mode = (PB5 == HIGH)
   └── Clock, watchdog, cache init
3. Board_InitFull()
   ├── If REMOTE MODE:
   │   ├── Skip RF board init (DAC, ADC, SPI RF)
   │   ├── Skip PLL/tuning init
   │   ├── Init USART2 (115200, IRQ priority 7)
   │   ├── Init LCD (local display)
   │   ├── Init encoder + buttons
   │   └── Remote_Init()
   │       ├── Reset packet RX state machine
   │       ├── Start 3-second HELLO broadcast (100ms interval)
   │       └── Enter REMOTE_ACTIVE on MAIN_ACK
   └── If MAIN MODE:
       └── (Normal full boot, see 6.2)
4. Main Loop:
   ├── If REMOTE MODE:
   │   ├── Poll encoder → send ENCODER_CMD if delta
   │   ├── Poll buttons → send BUTTON_CMD on edge
   │   ├── RX UART → parse packets
   │   │   ├── DISPLAY_LINE → update LCD
   │   │   ├── METER_UPDATE → update meter bar
   │   │   └── STATUS_UPDATE → update VFO/mode display
   │   └── Send HEARTBEAT @ 1Hz (watchdog)
   └── If MAIN MODE:
       └── (Normal main loop + remote listener)
```

### 6.2 Main Unit Boot Flow

```
1. Reset → SystemInit → main()
2. Board_InitMinimal() + Board_InitFull()
   ├── Normal radio init (full stack)
   └── Init USART2 (115200, IRQ priority 7)
3. Main Loop normal execution
4. Concurrent:
   ├── Main loop (ui, audio, RF)
   └── UART RX ISR / main polling:
       ├── Detect HELLO packet
       ├── Send MAIN_ACK with fw_version
       ├── Enter MAIN_REMOTE_ACTIVE state
       └── Parse remote commands:
           ├── ENCODER_DELTA → inject as local encoder event
           ├── BUTTON_PRESS → inject as local button press
           ├── PTT_TOGGLE → call RadioManagement_PttOnOff()
           └── FREQ_SET → call AudioDriver_SetFreq()
```

---

## 7. Detailed Design: Command Injection

### 7.1 From UART to UI (Main Side)

**Problem:** `ui_driver.c` đọc encoder/buttons trực tiếp từ GPIO, không có abstraction layer để inject external events.

**Solution:** Hook vào điểm xử lý input:

```c
// Trong ui_driver.c — main loop, trước khi UiDriver_TaskHandler_MainTasks()

// Remote input queue (ring buffer)
#define REMOTE_QUEUE_SIZE 8
typedef struct {
    uint8_t type;      // REMOTE_CMD_ENCODER / REMOTE_CMD_BUTTON
    int8_t delta;      // Encoder: +1/-1
    uint8_t button_id; // Button: ID
    uint8_t pressed;   // Button: 0/1
} remote_cmd_queue_t;

remote_cmd_queue_t remote_queue[REMOTE_QUEUE_SIZE];
uint8_t remote_queue_head = 0;
uint8_t remote_queue_tail = 0;

// Function to inject from UART parser
void Remote_InjectCommand(uint8_t type, int8_t delta, uint8_t btn, uint8_t pressed) {
    uint8_t next = (remote_queue_head + 1) % REMOTE_QUEUE_SIZE;
    if (next != remote_queue_tail) {  // Not full
        remote_queue[next].type = type;
        remote_queue[next].delta = delta;
        remote_queue[next].button_id = btn;
        remote_queue[next].pressed = pressed;
        remote_queue_head = next;
    }
}

// In main loop processing:
void UiDriver_ProcessRemoteCommands(void) {
    while (remote_queue_tail != remote_queue_head) {
        remote_queue_tail = (remote_queue_tail + 1) % REMOTE_QUEUE_SIZE;
        remote_cmd_queue_t *cmd = &remote_queue[remote_queue_tail];
        
        if (cmd->type == REMOTE_CMD_ENCODER) {
            // Inject as if local encoder moved
            UiDriver_InjectEncoderDelta(cmd->delta);
        } else if (cmd->type == REMOTE_CMD_BUTTON) {
            // Inject as if local button pressed
            UiDriver_InjectButtonEvent(cmd->button_id, cmd->pressed);
        }
    }
}
```

### 7.2 From Encoder to UART (Remote Side)

```c
// Trong ui_driver_remote.c hoặc ui_driver.c (remote mode only)

#define ENCODER_POLL_INTERVAL_MS 5
#define BUTTON_DEBOUNCE_MS 20

void UiDriver_RemoteMode_Poll(void) {
    static uint32_t last_encoder_poll = 0;
    static uint8_t last_encoder_state = 0;
    
    // Poll encoder at 200Hz (every 5ms)
    uint32_t now = ts.sysclock;
    if (now - last_encoder_poll >= ENCODER_POLL_INTERVAL_MS) {
        last_encoder_poll = now;
        
        uint8_t enc_a = hal_gpio_read_pin(ENCODER_A_PIO, ENCODER_A);
        uint8_t enc_b = hal_gpio_read_pin(ENCODER_B_PIO, ENCODER_B);
        uint8_t enc_state = (enc_a << 1) | enc_b;
        
        if (enc_state != last_encoder_state) {
            // Quadrature decode
            int8_t delta = 0;
            if ((last_encoder_state == 0x00 && enc_state == 0x01) ||
                (last_encoder_state == 0x01 && enc_state == 0x03) ||
                (last_encoder_state == 0x03 && enc_state == 0x02) ||
                (last_encoder_state == 0x02 && enc_state == 0x00)) {
                delta = +1;  // Clockwise
            } else {
                delta = -1;  // Counter-clockwise
            }
            
            if (delta != 0) {
                Remote_SendEncoderDelta(delta);
            }
            last_encoder_state = enc_state;
        }
    }
    
    // Poll buttons (matrix scan, debounced)
    UiDriver_RemoteMode_PollButtons();
}
```

---

## 8. Detailed Design: Display Synchronization

### 8.1 Strategy: Full Sync + Delta Updates

**Problem:** Remote LCD cần hiển thị giống main LCD, nhưng:
- Main LCD: 320x240 pixels (ILI9341/RA8875)
- Remote LCD: Cũng 320x240 (cùng mcHF UI board)
- Full framebuffer sync mỗi frame = ~150KB @ 60fps = quá lớn cho UART 115200

**Solution:**

```
Phase 1: Full Display Sync (on connect)
  - Main gửi toàn bộ pixelbuffer (PIXELBUFFERCOUNT x PIXELBUFFERSIZE)
  - Remote vẽ full frame lên LCD
  - ~200ms cho full sync (acceptable at boot)

Phase 2: Delta Updates (real-time)
  - Chỉ gửi các line/region đã thay đổi
  - Main so sánh framebuffer trước/sau, gửi dirty regions
  - Remote vẽ dirty region lên LCD
  - ~1-5ms cho mỗi update (real-time)
```

### 8.2 Dirty Rectangle Detection (Main Side)

```c
// Trong ui_lcd_hy28.c hoặc module mới remote_display.c

typedef struct {
    uint16_t x, y, width, height;
    uint8_t dirty;
} dirty_rect_t;

#define MAX_DIRTY_RECTS 16
dirty_rect_t dirty_rects[MAX_DIRTY_RECTS];

// Gọi sau mỗi lần vẽ xong frame
void Remote_Display_CommitDirty(void) {
    for (int i = 0; i < PIXELBUFFERCOUNT; i++) {
        uint16_t *buf = pixelbuffer[i];
        // Scan buffer, tìm changed regions
        // Gửi qua UART: DISPLAY_DIRTY_RECT + coordinates + pixel data
    }
}
```

### 8.3 Display Protocol Messages

```c
// Full sync (initial)
#define MSG_DISPLAY_FULL   0x84
// Payload: [width:2][height:2][pixelcount:2][pixeldata:N]

// Dirty rect update
#define MSG_DISPLAY_DIRTY  0x85
// Payload: [count:1][rect1:x:2,y:2,w:2,h:2,data:N][rect2...]

// Simple line update (fallback, low bandwidth)
#define MSG_DISPLAY_LINE   0x80
// Payload: [line_num:1][length:1][data:length]
```

---

## 9. Memory & Performance Budget

### 9.1 RAM Usage (Remote Unit)

| Component | Size | Notes |
|-----------|------|-------|
| Remote context | ~200 bytes | State machine, buffers |
| RX packet buffer | 256 bytes | UART DMA/IRQ buffer |
| TX packet buffer | 64 bytes | Max command size |
| Display framebuffer | 150KB | Same as main unit |
| Encoder queue | 32 bytes | Ring buffer |

**Total additional RAM:** ~200 bytes + small buffers (fits in STM32F4 192KB SRAM)

### 9.2 Latency Budget

| Component | Target | Budget |
|-----------|--------|--------|
| Encoder poll interval | 5ms | 1 encoder tick @ 1000 PPR = enough |
| UART TX latency | <1ms | 3 bytes @ 115200 |
| UART RX latency | <2ms | Interrupt-driven |
| Packet parsing | <0.5ms | Simple state machine |
| Display update | <10ms | For delta regions |
| End-to-end button press | <20ms | Acceptable for UI control |

**Worst case latency:** ~30ms (encoder hold + 3 packets queued) — still imperceptible.

---

## 10. Error Handling & Recovery

### 10.1 Error Codes

```c
typedef enum {
    REMOTE_ERR_NONE = 0,
    REMOTE_ERR_UART_INIT,        // UART init failed
    REMOTE_ERR_HELLO_TIMEOUT,    // No ACK from main within 3s
    REMOTE_ERR_CRC_MISMATCH,     // Packet CRC check failed
    REMOTE_ERR_PACKET_TOO_LONG,  // Overflow
    REMOTE_ERR_QUEUE_FULL,       // Command queue full
    REMOTE_ERR_DISPLAY_SYNC,     // Display sync failed
    REMOTE_ERR_MAIN_DISCONNECT,  // No heartbeat for >5s
} remote_error_t;
```

### 10.2 Recovery Strategies

| Error | Recovery | User Feedback |
|-------|----------|---------------|
| HELLO timeout | Blink red LED, retry every 1s | Red LED fast blink |
| CRC mismatch | Drop packet, don't ACK | No effect (statistical) |
| Queue full | Drop oldest command | Green LED flicker |
| Main disconnect | Stop sending, show "NO LINK" | LCD: "REMOTE LINK LOST" |
| UART framing | Reset UART peripheral | Red LED on + retry |

---

## 11. Build System Changes

### 11.1 New Compile-Time Flags

```makefile
# In Makefile / include.mak

# Remote control mode selection
REMOTE_MODE ?= 0  # 0=Main, 1=Remote

ifeq ($(REMOTE_MODE),1)
    CONFIGFLAGS += -DREMOTE_MODE=1 -DMODE_DETECT_PIN=GPIO_PIN_5 -DMODE_DETECT_PORT=GPIOB
endif
```

### 11.2 Build Targets

```makefile
# Dual firmware builds for remote pair

f4-mchf-remote-pair: f4-mchf f4-mchf-remote
    @echo "Built main firmware: fw-mchf.bin (flash to main unit)"
    @echo "Built remote firmware: fw-mchf-remote.bin (flash to remote unit)"

f4-mchf-remote:
    @$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) \
        BUILDFOR=F4 BOARD=mchf CONFIGFLAGS="-DREMOTE_MODE=1" firmware
    @cp $(BUILD_DIR)/fw-mchf.bin $(BUILD_DIR)/fw-mchf-remote.bin
```

---

## 12. API Reference

### 12.1 Remote Module API

```c
#ifndef __REMOTE_H
#define __REMOTE_H

#include <stdint.h>
#include "uhsdr_types.h"

/* Mode detection */
typedef enum {
    MODE_MAIN = 0,
    MODE_REMOTE = 1
} remote_mode_t;

remote_mode_t Remote_DetectMode(void);

/* Initialization */
hal_status_t Remote_Init(void);
void Remote_DeInit(void);

/* Command transmission (remote side) */
hal_status_t Remote_SendEncoderDelta(int8_t delta);
hal_status_t Remote_SendButton(uint8_t button_id, uint8_t pressed);

/* Command reception (main side) */
void Remote_ProcessIncoming(void);
void Remote_InjectIntoUI(void);  /* Call from main loop */

/* Display (remote side) */
hal_status_t Remote_DisplayUpdateLine(uint8_t line_id, const uint8_t *data, uint8_t len);
hal_status_t Remote_DisplayFullSync(const uint8_t *framebuffer, uint16_t width, uint16_t height);

/* Status */
uint8_t Remote_IsConnected(void);
remote_error_t Remote_GetLastError(void);

#endif /* __REMOTE_H */
```

---

## 13. Testing Strategy

### 13.1 Unit Tests (Host-Based)

| Test | Description | Framework |
|------|-------------|-----------|
| CRC8 | Verify CRC calculation | test_remote_crc.c |
| Packet encode/decode | Round-trip packet serialization | test_remote_protocol.c |
| Queue overflow | Verify drop-oldest behavior | test_remote_queue.c |
| Mode detection | GPIO read mock | test_remote_mode.c |

### 13.2 Integration Tests (Hardware)

| Test | Setup | Pass Criteria |
|------|-------|---------------|
| UART loopback | TX↔RX shorted on single board | All commands round-trip <5ms |
| Encoder latency | Remote encoder → Main response | <20ms visual update |
| Display sync | Full + delta sync | Pixel-identical within 100ms |
| Disconnect/reconnect | Unplug cable mid-session | Auto-reconnect within 2s |
| Button debounce | Rapid button press/release | No double-trigger |

### 13.3 Stress Tests

| Test | Duration | Pass Criteria |
|------|----------|---------------|
| Encoder rotate @ 1000Hz | 60s | No command lost, queue never full |
| Button mash | 10s @ 50Hz | All presses registered |
| UART error injection | 1000 packets | CRC catches all corrupted |

---

## 14. Security & Safety

### 14.1 Security Considerations

| Threat | Mitigation |
|--------|------------|
| Unauthorized control | Physical cable only (no wireless), mode jumper required |
| Command injection | CRC8 verification, reject corrupted packets |
| Replay attack | Sequence number + monotonic check (future enhancement) |
| UART noise | Hardware: twisted pair, 120Ω termination if long cable |

### 14.2 Safety Constraints

| Constraint | Implementation |
|------------|----------------|
| PTT interlock | Remote PTT command gated by watchdog kick on main |
| Emergency stop | Local button always overrides remote |
| Display timeout | If remote heartbeats stop >5s, main reverts to standalone |

---

## 15. Future Enhancements (Out of Scope v1.0)

1. **Wireless variant:** BLE/WiFi module nối UART, bỏ cable
2. **Multi-remote:** 1 main + N remotes (addressing in protocol)
3. **Voice channel:** Audio stream từ remote về main (I2S over UART/USB)
4. **Configuration sync:** Remote nhận full config từ main lần đầu connect
5. **OTA update:** Remote firmware update qua UART từ main
6. **Encryption:** AES-128 cho protocol nếu cần bảo mật cao

---

## 16. Open Issues & Decisions Needed

| Issue | Options | Recommendation |
|--------|---------|----------------|
| UART pins | USART2 (PA2/PA3) vs USART6 (PC6/PC7) | **USART2** — quen thuộc, có sẵn trên header |
| Mode detect pin | PB5 vs PB4 vs PA15 | **PB14** — PB5 = ENC_ONE_CH2 on mcHF UI; PB14 free since USB HOST removed in Phase 17 |
| Display sync method | Full framebuffer vs dirty rect vs line-based | **Full once + dirty rect** — balance bandwidth vs complexity |
| Packet buffer size | 64 vs 128 vs 256 bytes | **256 bytes** — accommodate full status structs |
| Protocol binary vs ASCII | Binary (compact) vs ASCII (debug-friendly) | **Binary** — compact, faster, CRC đơn giản |
| Watchdog behavior on disconnect | Revert to standalone vs halt | **Revert to standalone** — safer for field operation |
| Firmware mode selection | Compile-time `REMOTE_MODE=1` vs runtime GPIO detection | **Runtime GPIO detection** — single binary for both main and remote units; PB14 jumper at boot |
| Button command encoding | Separate 0x03/0x04 press/release vs single cmd with flag | **Separate 0x03/0x04** — matches ARCH.md Section 5.2 command table |
| Encoder injection | Direct GPIO quadrature vs TIM counter manipulation | **TIM counter via `UiDriverEncoderSetDelta()`** — reuses existing encoder interface |

> **⚠ Inconsistency warning.** The pinout table in 3.1 lists **PB5** as "Mode Detect",
> while the decision table above (16) selects **PB14** and records that
> PB5 is `ENC_ONE_CH2` (frequency encoder channel B on mcHF UI). These two sections
> contradict each other. **Section 16 is authoritative** — the code uses PB14, and
> PB5 must not be used because it would break the main tuning knob.
> See `docs/TODO.md` Phase 66 for the note about this being wrongly "fixed" to PB5
> and then reverted.

---

## 17. Implementation status (verified 2026-10-24)

| Item | Status |
|---|---|
| Protocol, framing, CRC8 | implemented and host-tested (`remote_protocol.c`) |
| State machines, command queue | implemented (`remote.c`) |
| Board config / mode detect pin | **PB14**, now defined per-board in `hardware/board_configs/` (was hard-coded in product code) |
| UART transport | ⚠ **was never compiled**: every `basesw` `hal_conf.h` ships `HAL_UART_MODULE_ENABLED` commented out, so `stm32f*xx_hal_uart.c` built to an empty object. Now enabled from the Makefile (`HALUARTFLAGS`) — basesw untouched |
| F4 UART binding | bound to **USART2 on PA2/PA3, AF7, NVIC priority 7** in `hal/src/uart/hal_uart_stm32_f4.c`; `USART2_IRQHandler` supplied there because `basesw` has no USART2 handler for this use |
| Module invocation | `Remote_Init()` + `Remote_Process()` now called from `uhsdr_main.c`, gated by `REMOTE_UI_PAIR_ENABLE` (1 on mcHF, 0 on OVI40) |
| Symbols in `fw-mchf.elf` (F4) | 6 live — was 0 before the wiring above |
| Hardware validation | ❌ **none** — never exercised on two real boards |

Until that last row is closed, treat remote control as "wired but unproven".

## 17. Implementation Notes (Phase 43-45)

### 17.1 Mode Detection
- Pin: **PB14** (corrected from ARCH.md Section 3.2 proposal of PB5)
- Reason: PB5 = ENC_ONE_CH2 on mcHF UI; PB14 free since USB HOST removed in Phase 17
- Detection: `Remote_DetectMode()` in `drivers/remote/remote.c` reads GPIO at boot
- Pull-down config; HIGH (3.3V) = REMOTE mode, LOW (GND) = MAIN mode

### 17.2 Single Binary Architecture
- **No compile-time `REMOTE_MODE` flag** — same `fw-mchf.bin` flashes to both units
- Runtime detection via PB14 jumper selects mode at boot
- Main unit: `Remote_GetMode() == MODE_MAIN`
- Remote unit: `Remote_GetMode() == MODE_REMOTE`

### 17.3 State Machines
- **Remote unit:** `REMOTE_STATE_INIT` → send HELLO (0xF0) @ 1Hz → `REMOTE_STATE_ACTIVE` on MAIN_ACK → send HEARTBEAT (0xF2) @ 1Hz
- **Main unit:** `MAIN_STATE_STANDALONE` → wait HELLO → `MAIN_STATE_REMOTE_ACTIVE` → revert to standalone if heartbeat missing >5s
- Handshake packets: HELLO (0xF0), MAIN_ACK (0xF1), HEARTBEAT (0xF2)

### 17.4 Command Injection (Main Side)
- Queue: 8-entry ring buffer in `remote.c`; `Remote_Enqueue()` / `Remote_Dequeue()`
- Injection functions:
  - `UiDriverEncoderSetDelta(ENCFREQ, delta)` in `drivers/ui/encoder/ui_rotary.c`
  - `UiDriver_InjectButton(button_id)` in `drivers/ui/ui_driver.c`
  - `RadioManagement_HandlePttOnOff()` for PTT_TOGGLE
  - `ts.tune_freq = freq` + `UiDriver_UpdateFrequency()` for FREQ_SET
- Guards: `UiDriver_KeyboardProcessOldClicks()` and `UiDriver_TaskHandler_MainTasks()` skip local processing in remote mode

### 17.5 Input Capture (Remote Side)
- Encoder: `UiDriver_Remote_Poll()` calls `UiDriverEncoderRead(ENCFREQ)` @ 100Hz; sends delta via UART
- Buttons: `Keypad_Scan()` + `Keypad_ButtonStates()` edge detection; sends press/release via `Remote_SendButton()`
- Commands: ENCODER_DELTA (0x01), BUTTON_PRESS (0x03), BUTTON_RELEASE (0x04)

### 17.6 Protocol Compliance
- Button commands use separate 0x03 (press) and 0x04 (release) per ARCH.md Section 5.2
- `Remote_SendButton()` dynamically selects cmd based on `pressed` flag
- Packet buffer sizes corrected: 7 bytes for button (was 6), 6 bytes for encoder

---

## 19. SDR Extension Analysis: 700 MHz / 2.3 GHz Video + Voice over COFDM/Mesh

**Version:** 0.1  
**Date:** 2026-09-10  
**Status:** Analysis — Design Pending

---

### 19.1 Executive Summary

Tài liệu này bổ sung phân tích khả năng mở rộng UHSDR từ **HF/UHF SSB/FM transceiver** hiện tại thành hệ thống **SDR một kênh video + voice** hoạt động tại **700 MHz** và **2.3 GHz**, sử dụng **COFDM** làm lớp vật lý và **mesh** làm lớp mac/routing. Phân tích dựa trên toàn bộ codebase hiện tại và các ràng buộc phần cứng/phần mềm đã biết.

**Kết luật tóm tắt:**
- **Voice:** khả thi ở cả 700 MHz và 2.3 GHz.
- **Video:** khả thi nhưng chặt; cần H7 là minimum hợp lý.
- **COFDM:** khả thi nhưng cần viết lại modem layer gần như từ đầu.
- **Mesh:** khả thi ở layer 2, không cần IP.
- **Không cần IP stack:** đúng, có thể làm custom PHY/MAC hoàn toàn độc lập.

---

### 19.2 Current Capability Assessment

#### 19.2.1 UHSDR hiện tại là gì

UHSDR hiện là firmware **bare-metal cooperative** cho STM32F4/F7/H7:
- Audio DMA ISR 1.5 kHz → PendSV → main loop.
- DSP audio 48 kHz: SSB/AM/FM/CW/RTTY/PSK/FreeDV.
- LO: Si570 hoặc Si5351.
- Codec audio: WM8731/WM8776.
- UI: LCD + encoder + keypad + menu + spectrum/waterfall.
- USB CDC, USB Audio, USB Host DFU.

UHSDR đã có board **LAPWING** (`RF_BRD_LAPWING`) chạy băng **1240–1300 MHz**, chứng minh codebase không bị khóa ở HF.

#### 19.2.2 Frequency coverage limits

| Component | Current range | Gap to target |
|---|---|---|
| Si5351A | ~2.3–290 MHz LO | 700 MHz cần converter; 2.3 GHz cần ngoại PLL |
| Si570 CMOS | 10–160 MHz LO | 700 MHz cần up-conversion |
| Si570 C-grade | 10–280 MHz LO | 2.3 GHz cần ngoại PLL |
| mcHF RF | HF/6m/2m/70cm | không có 700 MHz / 2.3 GHz RF frontend |
| OVI40 RF | HF/2m/70cm | không có 700 MHz / 2.3 GHz RF frontend |
| Lapwing RF | 1240–1300 MHz | đã có 1.2–1.3 GHz, nhưng không có 700 MHz / 2.3 GHz |

---

### 19.3 Service Layer: Voice + Video

#### 19.3.1 Voice

- **Khả thi:** cao.
- Codec đã có sẵn: **Codec2** (1300/3200 bit/s), **FreeDV**.
- Có thể thay thế bằng **AMBE+** nếu cần, nhưng cần codec phần cứng hoặc port software.
- Voice có thể được đóng gói trực tiếp vào khung custom mà không cần IP.

#### 19.3.2 Video

- **Khả thi:** có, nhưng chặt.
- Codebase hiện **không có video pipeline**.
- Cần thêm:
  - **Video source:** camera MIPI/CSI, USB webcam, hoặc overlay.
  - **Video encoder:** H.264 baseline hoặc MJPEG.
  - **Video decoder** nếu cần receive.
- H7 có **HAL_JPEG** cho JPEG image, nhưng không phải video stream.

#### 19.3.3 Codec recommendation

| Use case | Codec | Bitrate | Notes |
|---|---|---|---|
| Voice only | Codec2 1300 | 1300 bit/s | Đã có sẵn |
| Voice quality | Codec2 3200 | 3200 bit/s | Đã có sẵn |
| Digital voice narrow | AMBE+ | 3600 bit/s | Cần hardware/port |
| Video low res | MJPEG | 1–4 Mbps | Dễ implement |
| Video standard | H.264 baseline | 512 kbps–2 Mbps | Cần encoder |
| Video high quality | H.265 | 256 kbps–1 Mbps | Cần encoder, phức tạp |

---

### 19.4 Transmission Layer: COFDM

#### 19.4.1 COFDM không cần IP

**Có.** COFDM là **physical layer** hoàn toàn độc lập với IP.

- COFDM chỉ cần: **IFFT/FFT + pilot + cyclic prefix + equalizer**.
- Có mang trực tiếp **voice codec stream** hoặc **video elementary stream**.
- Có thể dùng **custom frame format** thay vì IP.

#### 19.4.2 Simplified COFDM design

Để triển khai trên STM32, nên bắt đầu với **COFDM đơn giản**:

| Parameter | Recommendation |
|---|---|
| FFT size | 32–64 subcarriers |
| Modulation | QPSK / 16QAM |
| Pilot pattern | Scattered + continuous |
| CP length | 1/4–1/8 of FFT |
| Equalizer | 1-tap per subcarrier |
| Synchronization | Schmidl-Cox |
| PAPR reduction | Clipping + tone reservation |
| Channel coding | LDPC hoặc Turbo code (quan trọng cho video) |

#### 19.4.3 COFDM vs Single-carrier

| Feature | COFDM | Single-carrier QPSK/16QAM + FHSS |
|---|---|---|
| Độ phức tạp | Cao | Trung bình–Thấp |
| Chống fading | Rất tốt | Tốt với equalizer |
| PAPR | Cao | Thấp |
| Synchronization | Phức tạp | Đơn giản |
| Mesh nhiều hop | Khó | Dễ |
| Point-to-point | Rất tốt | Tốt |

**Khuyến nghị:**
- Nếu cần **mesh nhiều hop** → **Single-carrier + DSSS/FHSS**.
- Nếu cần **chống fading/multipath** nặng → **COFDM**.

---

### 19.5 Mesh Layer (No IP Required)

#### 19.5.1 Mesh không cần IP

**Có.** Mesh hoạt động ở **layer 2 (MAC)**, không cần IP stack.

- Định tuyến theo **Node ID**.
- Frame format: `[Header][Payload][CRC]`.
- Routing protocol: **AODV-like** hoặc **BATMAN-like**.

#### 19.5.2 Custom frame format

```
┌──────────┬──────────┬──────────┬──────────────┬────────┐
│  START1  │  START2  │  LENGTH  │   PAYLOAD    │  CRC8  │
│  0xAA    │  0xBB    │  1 byte  │  0-252 bytes │ 1 byte │
└──────────┴──────────┴──────────┴──────────────┴────────┘

Payload:
  [NodeID_SRC: 1 byte]
  [NodeID_DST: 1 byte]
  [Next_Hop: 1 byte]
  [Seq: 1 byte]
  [Type: 1 byte]  // VOICE / VIDEO / CTRL
  [Priority: 1 byte]
  [Payload data...]
```

#### 19.5.3 MAC features needed

| Feature | Description |
|---|---|
| Node addressing | 1-byte node ID, 0 = broadcast |
| Route discovery | AODV-like RREQ/RREP |
| Forwarding | Store-and-forward, next-hop lookup |
| Loop prevention | TTL + hop count + sequence number |
| Priority scheduling | Voice > Video > Data |
| ARQ | Simple ACK/NACK per frame |
| Channel access | CSMA/CA hoặc scheduled TDMA |

---

### 19.6 Hardware Requirements

#### 19.6.1 RF Frontend

| Band | LO/VCO | Mixer | PA | LNA | Filter |
|---|---|---|---|---|---|
| 700 MHz | ADF4351/ADF4360, LMX2582 | ADL5801, SIM7600 | 500 mW–1 W | 2–3 dB NF | SAW/ceramic |
| 2.3 GHz | LMX2582, HMC1031 | HMC219, ADL5375 | 100–500 mW | 2–3 dB NF | Microstrip/SAW |

#### 19.6.2 ADC/DAC requirements

| Band | Sample rate | Resolution | Notes |
|---|---|---|---|
| 700 MHz | 16–20 MSPS | 12–14 bit | Down-convert trước khi ADC |
| 2.3 GHz | 50–100 MSPS | 12–14 bit | Cần mixer + ADC ngoại |

#### 19.6.3 MCU selection

| MCU | Voice | Simple video | COFDM | Mesh | Notes |
|---|---|---|---|---|---|
| STM32F4 | ✅ | ❌ | ⚠️ Cực chật | ⚠️ 3 nodes | mcHF hiện tại |
| STM32F7 | ✅ | ⚠️ MJPEG | ✅ Cơ bản | ✅ 3–5 nodes | OVI40 F7 |
| STM32H7 | ✅ | ✅ H.264 baseline | ✅ Khả thi | ✅ 5–10 nodes | OVI40 H7 |

**Kết luận:** STM32H7 là **minimum hợp lý** cho video + COFDM + mesh. STM32F4 chỉ đủ cho voice single-carrier.

---

### 19.7 Software Requirements

#### 19.7.1 New oscillator backends

```
osc_si570.c          ← Đã có, extend cho 700 MHz
osc_si5351a.c        ← Đã có, extend
osc_adf4351.c        ← NEW: PLL 35–4400 MHz
osc_lmx2582.c        ← NEW: PLL 20–8000 MHz
```

#### 19.7.2 New RF board config

```
UHSDR_RF_700m_config.h    ← NEW
UHSDR_RF_2300m_config.h   ← NEW

- Band filter table (700 MHz / 2.3 GHz)
- Coupling table
- Power level table
- PA bias/max power limits
```

#### 19.7.3 Modem layer (PHY)

```
drivers/
├── modem/
│   ├── modem.h                 ← Modem abstraction
│   ├── modem_qpsk.c            ← QPSK/16QAM modulator/demodulator
│   ├── modem_sc_fec.c          ← Single-carrier FEC
│   ├── modem_cofdm.c           ← COFDM PHY
│   │   ├── ofdm_fft.c          ← IFFT/FFT wrapper
│   │   ├── ofdm_pilot.c        ← Pilot insertion/removal
│   │   ├── ofdm_equalizer.c    ← 1-tap per subcarrier
│   │   ├── ofdm_sync.c         ← Schmidl-Cox
│   │   └── ofdm_papr.c         ← Clipping + tone reservation
│   └── modem_fec.c             ← LDPC / Turbo code
```

#### 19.7.4 Mesh layer (MAC + routing)

```
drivers/
├── mesh/
│   ├── mesh.h                  ← Mesh API
│   ├── mesh_mac.c              ← Frame format, addressing
│   ├── mesh_routing.c          ← AODV-like route discovery
│   ├── mesh_forward.c          ← Forwarding, loop prevention
│   ├── mesh_arq.c              ← ARQ, ACK/NACK
│   └── mesh_scheduler.c        ← Priority: Voice > Video > Data
```

#### 19.7.5 Video pipeline

```
drivers/
├── video/
│   ├── video.h                 ← Video API
│   ├── video_capture.c         ← Camera input (MIPI/USB)
│   ├── video_encode.c          ← H.264 baseline / MJPEG
│   ├── video_decode.c          ← Receive side
│   └── video_mux.c             ← Mux with voice
```

#### 19.7.6 Audio enhancements

```
drivers/audio/
├── audio_voicemode.c          ← Voice-specific processing
├── audio_codec2.c              ← Codec2 encode/decode (đã có)
├── audio_ambe.c                ← AMBE+ (nếu cần)
```

---

### 19.8 Layer Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                        │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────────┐   │
│  │    Voice     │ │    Video     │ │      Data        │   │
│  │  Codec2/     │ │  H.264/      │ │  Custom frames   │   │
│  │  AMBE+       │ │  MJPEG       │ │                  │   │
│  └──────┬───────┘ └──────┬───────┘ └───────┬──────────┘   │
│         │                │                │               │
│  ┌──────▼────────────────▼────────────────▼──────┐        │
│  │              PACKETIZER / MUXER               │        │
│  │   [NodeID][Seq][Type][Priority][Payload][CRC] │        │
│  └──────────────────────┬────────────────────────┘        │
│                         │                                 │
│  ┌──────────────────────▼────────────────────────┐        │
│  │           MESH ROUTING LAYER                  │        │
│  │   [Route Table][Next Hop][TTL][Hop Count]     │        │
│  └──────────────────────┬────────────────────────┘        │
│                         │                                 │
│  ┌──────────────────────▼────────────────────────┐        │
│  │              MODEM LAYER                      │        │
│  │   Single-carrier QPSK/16QAM DSSS/FHSS        │        │
│  │   hoặc Simplified COFDM                       │        │
│  │   [Scramble][FEC][Mod][Shaping]               │        │
│  └──────────────────────┬────────────────────────┘        │
│                         │                                 │
│  ┌──────────────────────▼────────────────────────┐        │
│  │              BASEBAND / PHY                   │        │
│  │   [Upsample][Interpolate][DAC][RF Frontend]   │        │
│  └───────────────────────────────────────────────┘        │
└─────────────────────────────────────────────────────────────┘
```

---

### 19.9 Implementation Roadmap

#### Phase 1: Foundation (STM32F7 minimum)
1. New oscillator backend cho ADF4351/LMX2582 (700 MHz / 2.3 GHz).
2. New RF board config (`UHSDR_RF_700m_config.h`, `UHSDR_RF_2300m_config.h`).
3. Voice codec integration: Codec2 encode/decode vào pipeline.
4. Simple single-carrier QPSK modem.
5. Custom packetizer: `[NodeID][Seq][Type][Payload][CRC]`.

#### Phase 2: Mesh Layer
6. MAC layer: addressing, frame format.
7. AODV-like route discovery.
8. Forwarding + loop prevention.
9. Priority scheduling: Voice > Video > Data.
10. ARQ (ACK/NACK).

#### Phase 3: COFDM PHY
11. IFFT/FFT wrapper (CMSIS-DSP đã có sẵn).
12. Pilot insertion + channel estimation.
13. Equalizer 1-tap per subcarrier.
14. Synchronization (Schmidl-Cox).
15. PAPR reduction.
16. LDPC/Turbo code.

#### Phase 4: Video
17. Video capture (camera interface).
18. MJPEG encoder (đơn giản nhất).
19. H.264 baseline encoder (nếu cần chất lượng cao).
20. Video mux with voice.

---

### 19.10 Technology Comparison

| Technology | Độ phức tạp | Chống fading | PAPR | Mesh fit | Best for |
|---|---|---|---|---|---|
| **COFDM** | Cao | Rất tốt | Cao | Trung bình | Broadcast, mobile, multipath nặng |
| **Single-carrier + FHSS** | Thấp | Tốt | Thấp | Rất tốt | Mesh nhiều hop, anti-jam |
| **DSSS + QPSK** | Trung bình | Rất tốt | Thấp | Rất tốt | Tactical mesh, low SNR |
| **GMSK** | Thấp | Trung bình | Thấp | Tốt | Voice, low-rate data |

**Khuyến nghị cho use-case video + voice + mesh:**
- **Phase 1–2:** Single-carrier QPSK/16QAM + DSSS/FHSS mesh (đi nhanh, robust).
- **Phase 3+:** Simplified COFDM nếu multipath là vấn đề lớn.

---

### 19.11 Feasibility Summary

| Question | Answer | Notes |
|---|---|---|
| COFDM không cần IP? | ✅ Có | Là PHY layer, independent |
| Mesh không cần IP? | ✅ Có | Là MAC/layer 2, independent |
| Voice @ 700 MHz? | ✅ Khả thi | Codec2 đã có |
| Voice @ 2.3 GHz? | ✅ Khả thi | Cần LO/frontend mới |
| Video @ 700 MHz? | ⚠️ Khả thi | Cần H7 minimum |
| Video @ 2.3 GHz? | ⚠️ Khả thi | Cần H7 + frontend mới |
| COFDM trên STM32F4? | ❌ Không | Quá chật |
| COFDM trên STM32F7? | ⚠️ Cơ bản | Voice + simple video |
| COFDM trên STM32H7? | ✅ Khả thi | Voice + basic video + mesh |
| Single-carrier mesh trên F4? | ⚠️ Voice only | Không đủ cho video |
| Single-carrier mesh trên F7? | ✅ Khả thi | Voice + small mesh |
| Single-carrier mesh trên H7? | ✅ Khả thi | Voice + video + mesh |
| Nên dùng UHSDR trực tiếp? | ⚠️ Cần nhiều viết lại | Dùng làm nền hoặc module |

---

### 19.12 Recommendations

1. **Nếu cần đi nhanh:**
   - Bắt đầu với **Voice + Single-carrier QPSK/16QAM + FHSS mesh** trên STM32F7/H7.
   - COFDM để phase sau.

2. **Nếu cần chống fading:**
   - Simplified COFDM (32–64 subcarriers) trên STM32H7.
   - Single-carrier cho mesh, COFDM cho point-to-point.

3. **Nếu cần production-grade:**
   - STM32H7 là minimum.
   - Cân nhắc **FPGA/SoC** (Zynq, RFSoC) nếu cần video+COFDM+mesh thực sự.
   - Hoặc dùng **LimeSDR / USRP / bladeRF** + GNU Radio + custom stack.

4. **Architecture reuse:**
   - Có thể tái sử dụng **HAL shim layer**, **cooperative execution model**, **watchdog/fault handler**, **config storage**, **UI framework** của UHSDR.
   - Cần viết lại: **modem PHY**, **mesh MAC**, **video pipeline**, **RF frontend config**.

---

### 19.13 Risk Analysis

| Risk | Severity | Mitigation |
|---|---|---|
| STM32 compute insufficient | Cao | Dùng H7, hoặc FPGA/SoC |
| RF frontend complexity | Cao | Thuê design nếu không có kinh nghiệm RF |
| COFDM synchronization | Trung bình | Bắt đầu single-carrier, thêm COFDM sau |
| Video bandwidth | Trung bình | Giới hạn resolution/fps, dùng MJPEG |
| Regulatory (700 MHz / 2.3 GHz) | Cao | Kiểm tra license, power limit, band plan |
| Thermal (PA 700 MHz / 2.3 GHz) | Trung bình | Heat sink, power control |

---

### 19.14 References

- `docs/AGENTS.md` — Platform architecture v6.0
- `mchf-eclipse/drivers/ui/oscillator/osc_si570.c` — Si570 oscillator backend
- `mchf-eclipse/drivers/ui/oscillator/osc_si5351a.c` — Si5351 oscillator backend
- `mchf-eclipse/drivers/audio/audio_driver.c` — Audio pipeline
- `mchf-eclipse/drivers/audio/codec/codec.c` — Audio codec driver
- `mchf-eclipse/drivers/ui/radio_management.c` — Band/power/frequency management
- `mchf-eclipse/hardware/board_configs/UHSDR_UI_mchf_config.h` — mcHF board config
- `mchf-eclipse/hardware/board_configs/UHSDR_UI_ovi40_config.h` — OVI40 board config
- `mchf-eclipse/basesw/mcHF/Drivers/CMSIS/DSP_Lib/Source/` — CMSIS-DSP (FFT/IFFT available)

---

## 20. References (Original Remote Control)

- `docs/AGENTS.md` — Platform architecture v6.0
- `docs/TODO.md` — Task tracking for Phase 43+
- `mchf-eclipse/drivers/cat/cat_driver.c` — Existing CAT protocol implementation
- `mchf-eclipse/hardware/uhsdr_board.c` — Board init code
- `mchf-eclipse/drivers/ui/ui_driver.c` — UI input handling
- `mchf-eclipse/drivers/remote/remote.c` — Remote control module implementation
- `mchf-eclipse/drivers/remote/remote_protocol.c` — Packet framing & CRC8-CCITT

---

*This document is the single source of truth for architecture decisions. All implementation must align with this design.*