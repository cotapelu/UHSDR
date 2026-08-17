# UHSDR Platform Architecture — Codebase-Grounded Baseline v5.0

**Version:** 5.0  
**Language:** C11/C17 (GNU extensions)  
**Build:** GNU Make + ARM GCC + `.mak` files  
**Execution:** Bare-metal audio DMA ISR → PendSV deferred → cooperative main loop  
**Memory Model:** Static allocation primary; no heap in product code; `malloc` unused  
**Target MCUs:** STM32F4 (Cortex-M4), STM32F7 (Cortex-M7), STM32H7 (Cortex-M7)  
**Target Boards:** mcHF UI (F4), OVI40 UI (F7/H7), LAPWING (RF only, WIP)  
**Safety Profile:** IEC 62304 Class B (practical)

---

## 1. Platform Vision

Tối ưu codebase UHSDR thành **platform chuẩn** cho STM32F4/F7/H7, dựa trên nền tảng hiện có.

**Mục tiêu:**
- 1 codebase, 7 firmware builds + 6 bootloader builds
- Product code chia sẻ 100% giữa các MCU (`files.mak` chung)
- MCU abstraction tập trung vào `uhsdr_mcu.h` + board config headers
- Loại bỏ scattered `#ifdef` không cần thiết
- Bổ sung: watchdog, hoàn thiện fault handlers, cache maintenance
- Tận dụng `basesw/` structure hiện có

---

## 2. Supported Hardware

### MCU Matrix

| MCU | Core | FPU | Flash | SRAM | CCM/SRAM1 | Cache | Build Flag |
|---|---|---|---|---|---|---|---|
| STM32F407xx | M4 | FPv4-SP | 1MB | 192KB | 64KB CCM | None | `BUILDFOR=F4` |
| STM32F4-512KB | M4 | FPv4-SP | 512KB | 192KB | 64KB CCM | None | `BUILDFOR=F4-512KB` |
| STM32F767xx | M7 | FPv5-D16 | 1MB+ | 320KB+ | 64KB CCM | I-cache | `BUILDFOR=F7` |
| STM32H743xx | M7 | FPv5-D16 | 1MB+ | 512KB+ | 128KB SRAM1 | I+D-cache | `BUILDFOR=H7` |

### Board Matrix

| Board | UI | RF | MCU | Config Header | Status |
|---|---|---|---|---|---|
| mcHF | ✅ | ✅ | **F4 only** | `UHSDR_UI_mchf_config.h` | Production |
| OVI40 | ✅ | ✅ | **F7/H7 only** | `UHSDR_UI_ovi40_config.h` | Production |
| LAPWING | ❌ | ⚠️ | Unspecified | `RF_BRD_LAPWING` | WIP, no UI |

### Valid Build Matrix (7 firmware + 6 bootloader)

| MCU | Board | Firmware | Bootloader | Status |
|---|---|---|---|---|
| F4 | mcHF | `fw-mchf_f4-mchf.bin` | `bl-mchf_f4-mchf.bin` | ✅ Default |
| F4-512KB | mcHF | `fw-mchf_f4-small.bin` | shared | ✅ Small build |
| F4 | OVI40 | `fw-mchf_f4-ovi40.bin` | `bl-mchf_f4-ovi40.bin` | ❌ Compile error: `#error McHF boards support only CORTEX_M4` |
| F7 | mcHF | `fw-mchf_f7-mchf.bin` | `bl-mchf_f7-mchf.bin` | ❌ Compile error: `#error OVI40 boards support only CORTEX_M7` |
| F7 | OVI40 | `fw-mchf_f7-ovi40.bin` | `bl-mchf_f7-ovi40.bin` | ✅ |
| H7 | mcHF | `fw-mchf_h7-mchf.bin` | `bl-mchf_h7-mchf.bin` | ❌ Compile error: `#error McHF boards support only CORTEX_M4` |
| H7 | OVI40 | `fw-mchf_h7-ovi40.bin` | `bl-mchf_h7-ovi40.bin` | ✅ |

> **Note:** `make all` builds ALL 7 firmware combinations. Invalid combos fail at compile time due to `#error` in board config headers.

---

## 3. Core Architecture

```text
mchf-eclipse/
├── basesw/
│   ├── mcHF/     ← F4 HAL/startup/middleware (CubeMX)
│   ├── ovi40/    ← F7 HAL/startup/middleware (CubeMX)
│   └── ovi40-h7/ ← H7 HAL/startup/middleware (CubeMX)
├── drivers/
│   ├── audio/    ← Audio pipeline (shared, all MCUs)
│   │   └── codec/ ← I2S (F4) / SAI (F7/H7) callbacks
│   ├── ui/       ← LCD, spectrum, waterfall, menu, encoder, radio (shared)
│   │   └── lcd/  ← Display controllers, touchscreen, spectrum
│   ├── usb/      ← USB device + host (shared)
│   ├── freedv/   ← FreeDV codec (shared)
│   └── cat/      ← CAT control (shared)
├── hardware/
│   ├── board_configs/  ← mcHF/ovi40 config headers with #error guards
│   ├── uhsdr_board.c   ← Board init, LED, RTC, RAM detect, BusFault handler
│   ├── uhsdr_mcu.h     ← MCU abstraction (GPIO, CPU type, flash size)
│   ├── uhsdr_keypad.c  ← Keypad scanning
│   └── uhsdr_hw_i2c.c  ← I2C bus abstraction
├── misc/
│   ├── config_storage.c ← Flash/EEPROM config
│   ├── serial_eeprom.c  ← EEPROM driver
│   ├── uhsdr_canary.c   ← Stack canary
│   └── v_eprom/         ← Flash driver
├── src/
│   ├── uhsdr_main.c     ← Main loop, state machine initialization
│   └── bootloader/       ← DFU via USB Host + FatFs
└── linker/               ← 9 linker scripts (firmware + bootloader × 3 MCUs)
```

### Execution Model (all MCUs)
- **Audio DMA ISR** @ 1.5kHz — RX/TX processing, AGC, filters, NR, FreeDV encode/decode
- **PendSV** — deferred high-priority tasks (FreeDV, NR, PTT), lowest priority
- **Main loop** — UI, spectrum, menus, encoder, keyboard, meters
- **No RTOS. No preemptive scheduler.**

### Execution Flow
```
Audio DMA ISR (1.5kHz)
    ├── HAL_DMA_IRQHandler() [basesw]
    ├── AudioDriver_I2SCallback() [drivers/audio/audio_driver.c]
    ├── AGC, filters, CW, NR
    ├── ts.sysclock++ (100Hz timebase)
    └── PendSV trigger: SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk
         └── PendSV_Handler() [basesw/*/Src/stm32fxx_it.c]
              └── UiDriver_TaskHandler_HighPrioTasks() [drivers/ui/ui_driver.c]
                   ├── FreeDv_HandleFreeDv()
                   ├── AudioNr_HandleNoiseReduction()
                   └── RadioManagement_HandlePttOnOff()

Main Loop (cooperative)
    ├── UiDriver_TaskHandler_MainTasks()
    ├── Encoder/keys @ 100Hz
    ├── Spectrum/waterfall @ 40-80ms
    ├── Meters @ 25Hz
    └── Menu/UI rendering
```

---

## 4. Build System

### Structure
```
Makefile (root)
├── mchf-eclipse/Makefile (inner, includes .mak files)
├── files.mak              ← Shared product code (ALL MCUs)
├── include.mak            ← Shared include dirs
├── f4-files.mak           ← F4 HAL sources
├── f4-include.mak         ← F4 include dirs
├── f4-bootloader.mak      ← F4 bootloader sources
├── (same for F7, H7)
└── bootloader.mak         ← Shared bootloader sources
```

### Key Points
- **Product code is 100% shared** across MCUs (`files.mak` is identical)
- **Only HAL/startup/middleware differs** per MCU (`f4-files.mak`, `f7-files.mak`, `h7-files.mak`)
- **Board selection via `-DUI_BRD_MCHF` / `-DUI_BRD_OVI40`** in CONFIGFLAGS
- **MCU selection via `-DCORTEX_M4` / `-DCORTEX_M7`** in machine flags
- **Compile flags:** `-O2` for firmware, `-Os` for bootloader, `-flto`, `-Wall -Wextra`

### Build Commands
```bash
make                    # Build firmware + bootloader for default (F4, mcHF)
make BUILDFOR=F7 BOARD=ovi40 firmware   # Build F7 firmware for OVI40
make BUILDFOR=H7 BOARD=ovi40 both       # Build H7 firmware + bootloader
make all-firmware       # Build ALL 7 firmware combinations
make all-bootloader     # Build ALL 6 bootloader combinations
```

---

## 5. Layer Contracts

### 5.1 basesw/ (Per-MCU HAL)
- **mcHF:** STM32F4xx HAL, CMSIS, USB Host/Device, FatFs, I2S
- **ovi40:** STM32F7xx HAL, CMSIS, USB Host/Device, FatFs, SAI, FMC
- **ovi40-h7:** STM32H7xx HAL, CMSIS, USB Host/Device, FatFs, SAI, FMC, MDMA

**Owns:** startup, HAL drivers, CubeMX-generated `main.c`, `stm32fxx_it.c`, `stm32fxx_hal_msp.c`, fault handlers

### 5.2 drivers/ (Product Drivers — Shared)
- **audio:** RX/TX pipeline, AGC, filters, CW, RTTY, PSK, FreeDV
- **ui:** LCD, spectrum, waterfall, menu, encoder, keypad, touch, radio state machine
- **usb:** Device (Audio+CDC), Host (HID/MSC) — **Host compiled but never initialized**
- **freedv:** FreeDV digital voice codec
- **cat:** Computer Aided Transceiver control

### 5.3 hardware/ (Board Abstraction — Shared)
- **board_configs:** Per-board config headers with `#error` MCU guards
- **uhsdr_board.c:** Board init, LED, RTC, RAM detect (BusFault-based on F4/F7, hardcoded on H7)
- **uhsdr_mcu.h:** MCU type detection, GPIO abstraction, flash size
- **uhsdr_keypad.c:** Keypad scanning with board-specific maps
- **uhsdr_hw_i2c.c:** I2C bus abstraction (Si5351A, codecs, EEPROM)

### 5.4 misc/ (Utilities — Shared)
- **config_storage.c:** Flash/EEPROM config with RAM cache
- **serial_eeprom.c:** I2C EEPROM driver
- **uhsdr_canary.c:** Stack canary
- **uhsdr_math.c:** Math utilities

### 5.5 src/ (Application Entry)
- **uhsdr_main.c:** Main loop, state machine initialization
- **bootloader/:** DFU via USB Host + FatFs

---

## 6. Existing MCU Abstraction (Accurate)

### 6.1 uhsdr_mcu.h
```c
// mchf-eclipse/hardware/uhsdr_mcu.h

typedef enum {
    CPU_NONE = 0,
    CPU_STM32F4 = 1,
    CPU_STM32F7 = 2,
    CPU_STM32H7 = 3,
} mchf_cpu_t;

inline static mchf_cpu_t MchfHW_Cpu(); // Returns current MCU type

// GPIO abstraction (H7 uses BSRRL/BSRRH, F4/F7 use BSRR)
inline static void GPIO_SetBits(GPIO_TypeDef *PORT, uint32_t PINS);
inline static void GPIO_ResetBits(GPIO_TypeDef *PORT, uint32_t PINS);
inline static void GPIO_ToggleBits(GPIO_TypeDef *PORT, uint32_t PINS);

// Flash size abstraction
#define STM32_GetFlashSize()    // F4/F7: read flash size register, H7: FLASH_SIZE/1024
#define STM32_GetRevision()     // Read device revision
#define STM32_UUID              // Unique device ID
```

### 6.2 Board Config Headers
```c
// mchf-eclipse/hardware/board_configs/UHSDR_UI_mchf_config.h (F4)
#define __MCHF_SPECIALMEM __attribute__ ((section (".ccm")))  // 64KB CCM
#define __UHSDR_DMAMEM    // no-op

// mchf-eclipse/hardware/board_configs/UHSDR_UI_ovi40_config.h (F7/H7)
#define __MCHF_SPECIALMEM    // no-op
#if defined(STM32H7)
    #define __UHSDR_DMAMEM __attribute__ ((section (".dmamem")))  // SRAM1
#else
    #define __UHSDR_DMAMEM    // no-op
#endif

#define USE_TWO_CHANNEL_AUDIO  // OVI40 only
#define USE_HMC1023            // OVI40 only
```

### 6.3 Memory Sections (Actual Usage)

| Section | F4 | F7 | H7 | Used For |
|---|---|---|---|---|
| `.ccm` | 64KB @ 0x10000000 | 64KB @ 0x10000000 | — | Audio filters, spectrum, NR states (`__MCHF_SPECIALMEM`) |
| `.dmamem` | — | — | 128KB @ 0x30000000 | LCD pixelbuffer only (`__UHSDR_DMAMEM`) |
| `.ram` | 128KB @ 0x20000000 | 256KB @ 0x20000000 | 512KB @ 0x24000000 | Default data, stack |

> **Note:** `.ccm` is no-op on F7/H7. `.dmamem` is H7-only. F7 has no dedicated DMA section.

---

## 7. Fault Handlers (Actual State)

### 7.1 F4 (mcHF)
| Handler | Location | Behavior |
|---|---|---|
| HardFault_Handler | `stm32f4xx_it.c` | Naked, extracts registers, calls `Debug_FaultGetRegistersFromStack()` |
| MemManage_Handler | `stm32f4xx_it.c` | Empty while(1) loop |
| BusFault_Handler | `uhsdr_board.c` | Naked, used for RAM size detection (192/256/512KB) |
| UsageFault_Handler | `stm32f4xx_it.c` | Empty while(1) loop |

### 7.2 F7 (ovi40)
| Handler | Location | Behavior |
|---|---|---|
| HardFault_Handler | `stm32f7xx_it.c` | Empty while(1) loop |
| MemManage_Handler | `stm32f7xx_it.c` | Empty while(1) loop |
| BusFault_Handler | **MISSING** | Falls back to `Default_Handler` |
| UsageFault_Handler | `stm32f7xx_it.c` | Empty while(1) loop |

### 7.3 H7 (ovi40-h7)
| Handler | Location | Behavior |
|---|---|---|
| HardFault_Handler | `stm32h7xx_it.c` | Empty while(1) loop |
| MemManage_Handler | `stm32h7xx_it.c` | Empty while(1) loop |
| BusFault_Handler | **MISSING** | Falls back to `Default_Handler` |
| UsageFault_Handler | `stm32h7xx_it.c` | Empty while(1) loop |

### 7.4 Issues
- F7/H7 fault handlers are empty loops — no register dump, no diagnostics
- BusFault_Handler missing on F7/H7 — RAM detection not possible
- F4 BusFault_Handler is shared with RAM detection — not a general fault handler

---

## 8. Audio Pipeline

### 8.1 Signal Chain
```
RX: ADC → DMA → Audio ISR → IQ decimation → Hilbert → AGC → Filter → NR → DAC
TX: ADC → DMA → Audio ISR → Filter → ALC → Interpolation → DAC
```

### 8.2 Audio Interfaces

| MCU | Peripheral | Callbacks | Codec | Channels |
|---|---|---|---|---|
| F4 | I2S3 | `HAL_I2S_RxCpltCallback`, `HAL_I2S_RxHalfCpltCallback` | WM8731 | 1 |
| F7 | SAI1 | `HAL_SAI_RxCpltCallback`, `HAL_SAI_RxHalfCpltCallback` | WM8731 | 1 |
| H7 | SAI1+SAI2 | Same + SAI2 callbacks | WM8731 + external | 2 |

### 8.3 Audio ISR Contract
- **Frequency:** 1.5kHz (`IQ_INTERRUPT_FREQ`)
- **Block size:** 128-256 samples
- **ISR time budget:** < 100µs
- **Deferred work:** PendSV for FreeDV, NR, PTT
- **No blocking:** No HAL_Delay, no printf, no malloc

### 8.4 Audio Codec File
`drivers/audio/codec/uhsdr_hw_i2s.c` contains both I2S (F4) and SAI (F7/H7) callbacks, selected via `#ifdef UI_BRD_MCHF` / `#ifdef UI_BRD_OVI40`.

### 8.5 Audio State Placement
```c
// Filter states, decimation, interpolation — all in .ccm on F4
float32_t __MCHF_SPECIALMEM decimState_I[...];
float32_t __MCHF_SPECIALMEM decimState_Q[...];
float32_t __MCHF_SPECIALMEM interpState[...];

// On F7/H7: __MCHF_SPECIALMEM is no-op → placed in default .data/.bss
```

---

## 9. Display Subsystem

### 9.1 Supported Controllers

| Controller | Interface | Resolution | Detection |
|---|---|---|---|
| ILI9320/ILI9325 | SPI/Parallel | 320x240 | Read ID |
| ILI9486 | Parallel | 480x320 | Read ID 0x9486 |
| RA8875 | SPI/Parallel | 800x480 | Read ID 0x8875 |
| SSD1289 | Parallel | 320x240 | HY32D |
| RPi 3.5" | SPI | 480x320 | Force detect |

### 9.2 Display Abstraction
```c
// mchf-eclipse/drivers/ui/lcd/ui_lcd_hy28.h
typedef struct {
    uint16_t (*ReadDisplayId)(void);
    void (*SetActiveWindow)(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void (*SetCursorA)(uint16_t Xpos, uint16_t Ypos);
    void (*WriteRAM_Prepare)(void);
    void (*WriteReg)(uint16_t reg, uint16_t val);
    // ... drawing primitives
} uhsdr_display_info_t;

const uhsdr_display_info_t display_infos[]; // Detection table
```

### 9.3 SPI Prescalers (Per MCU)

| MCU | LCD Default | LCD High-Speed | Touch |
|---|---|---|---|
| F4 | SPI_BAUDRATEPRESCALER_4 | SPI_BAUDRATEPRESCALER_2 | SPI_BAUDRATEPRESCALER_64 |
| F7 | SPI_BAUDRATEPRESCALER_8 | SPI_BAUDRATEPRESCALER_4 | SPI_BAUDRATEPRESCALER_128 |
| H7 | SPI_BAUDRATEPRESCALER_8 | SPI_BAUDRATEPRESCALER_4 | SPI_BAUDRATEPRESCALER_32 |

### 9.4 DMA Buffer
```c
// mchf-eclipse/drivers/ui/lcd/ui_lcd_hy28.c
static __UHSDR_DMAMEM uint16_t pixelbuffer[PIXELBUFFERCOUNT][PIXELBUFFERSIZE];
```
- F4: `.ccm` (DMA-capable)
- F7: No-op → regular RAM (cache maintenance MISSING)
- H7: `.dmamem` (SRAM1, cache maintenance MISSING)

### 9.5 Touchscreen (XPT2046)
- **Controller:** XPT2046 (resistive, SPI)
- **IRQ:** Active low, debounced in main loop
- **State machine:** `TP_DATASETS_NONE → TP_DATASETS_WAIT → TP_DATASETS_PROCESSED`
- **SPI speed:** Dynamic prescaler change during touch read

---

## 10. Input Subsystem

### 10.1 Keypad Matrix
```c
// mchf-eclipse/hardware/uhsdr_keypad.c
typedef struct {
    GPIO_TypeDef *keyPort;
    uint16_t keyPin;
    uint16_t button_id;
} Keypad_KeyPhys_t;

// Board-specific map arrays: bm_set_normal_arr[], etc.
void Keypad_Init(void);
void Keypad_Scan(void);
bool Keypad_IsKeyPressed(uint16_t button_id);
```

### 10.2 Button Mapping

**mcHF (17 buttons + touch):**
- F1-F5, G1-G4, M1-M3, BNDM, BNDP, STEPM, STEPP, PWR
- Encoders: ENC_ONE, ENC_TWO, ENC_THREE, FREQ_ENC

**OVI40 (23 buttons + touch):**
- F1-F6, G1-G4, M1-M3, E1-E4, S18, S19
- BNDM, BNDP, STEPM, STEPP, PWR
- Encoders: same as mcHF

---

## 11. Spectrum/Waterfall

### 11.1 Implementation
- **FFT:** CMSIS DSP `arm_rfft_fast_f32`, `arm_cfft_f32`
- **Update rate:** 40-80ms (main loop, not ISR)
- **Ring buffer:** `FFT_RingBuffer[FFT_IQ_BUFF_LEN]` — accessed from ISR and main loop

### 11.2 Memory Budget

| MCU | Waterfall | Buffer Location |
|---|---|---|
| F4 | Half resolution | `.ccm` (static) |
| F7 | Full resolution | `.ram` (static in `sd` struct) |
| H7 | Full resolution | `.ram` (static in `sd` struct) |

### 11.3 Cache Considerations (F7/H7)
- `sd.FFT_RingBuffer` written by audio ISR, read by main loop
- **No cache maintenance** — potential corruption on F7/H7
- **No cache maintenance** for LCD pixelbuffer DMA on F7/H7

---

## 12. Safety Mechanisms

### 12.1 Watchdog
**STATUS: MISSING from product code**
- HAL driver exists in `basesw/`
- Product code never calls `HAL_IWDG_Init()` or `HAL_IWDG_Refresh()`
- **MANDATORY: Add to `uhsdr_main.c` main loop**

### 12.2 Stack Guard
**STATUS: PARTIAL**
- `uhsdr_canary.c` exists with canary word
- `malloc` for canary pointer in init (startup only)
- Not enforced in main loop or ISR

### 12.3 Cache Maintenance
**STATUS: MINIMAL**
- `SCB_CleanDCache()` called in `Board_Reboot()` (F7/H7)
- `SCB_CleanDCache()` called in bootloader `command.c` (F7)
- **MISSING for:** LCD DMA, touchscreen, FFT ring buffer

### 12.4 Bootloader Safety
- CRC32 or AES-128-GCM image validation
- Version check (anti-rollback)
- Boot accounting (failed boot counter in SRAM2/RTC backup)
- Recovery mode after N failed boots

---

## 13. Existing Scattered `#ifdef` (49 instances)

### In Product Code (excluding board_configs/)

| File | Count | Examples |
|---|---|---|
| `drivers/ui/lcd/ui_lcd_hy28.c` | 6 | SPI prescaler, DMA enable, LCD RAM addr, touch |
| `drivers/ui/lcd/ui_spectrum.c` | 4 | Cache clean, FFT buffer placement |
| `drivers/ui/menu/ui_menu.c` | 2 | Menu layout, feature flags |
| `drivers/audio/audio_driver.c` | 1 | 32-bit IQ bits |
| `drivers/audio/audio_filter.c` | 1 | LMS autonotch |
| `hardware/uhsdr_board.c` | 2 | Cache clean, flash wait states |
| `hardware/uhsdr_hw_i2c.c` | 2 | I2C speed change (F4-only), timing |
| `hardware/uhsdr_rtc.c` | 2 | RTC init (F4), LSE (H7 FIXME) |
| `hardware/uhsdr_board_config.h` | 3 | Flash size, I2C speed, RAM detect |
| `src/uhsdr_main.c` | 1 | CCM memory init |
| `src/bootloader/*.c` | 5 | Flash interface, cache, USB |
| `misc/v_eprom/uhsdr_flash.c` | 3 | Flash sector layout |
| `misc/profiling.h` | 1 | DWT enable (F7) |

---

## 14. What Already Exists (Keep)

✅ **Audio DMA ISR + PendSV pattern** — proven across all MCUs  
✅ **Shared product code** (`files.mak`) — single source of truth  
✅ **Board config system** — `#error` guards, `__MCHF_SPECIALMEM`, `__UHSDR_DMAMEM`  
✅ **MCU abstraction** (`uhsdr_mcu.h`) — GPIO, CPU type, flash size  
✅ **Display detection logic** (`display_infos[]`) — flexible, both boards  
✅ **Keypad scanning** (`uhsdr_keypad.c`) — board-specific maps, shared scanner  
✅ **Touchscreen state machine** — XPT2046, both boards  
✅ **PendSV for deferred work** — `UiDriver_TaskHandler_HighPrioTasks()`  
✅ **Static allocation in CCM** — F4 uses CCM effectively  
✅ **Full HAL per MCU** — `basesw/{mcHF,ovi40,ovi40-h7}/`  
✅ **F4 HardFault handler** — register dump via `Debug_FaultGetRegistersFromStack()`  
✅ **BusFault handler** — RAM size detection on F4/F7  
✅ **SPI prescaler abstraction** — per-MCU in `ui_lcd_hy28.c`  
✅ **Cache maintenance** — `Board_Reboot()` and bootloader  

---

## 15. What's Missing (Add)

### Priority 1: Critical
1. **Watchdog initialization** — IWDG HAL driver exists but never started
2. **F7/H7 fault handlers** — need register dump like F4
3. **BusFault_Handler on F7/H7** — missing entirely

### Priority 2: High
4. **H7 RAM detection** — hardcoded to 512KB, needs real detection
5. **I2C timing abstraction** — F4-only speed change, FIXME for F7/H7
6. **H7 RTC support** — FIXME in `uhsdr_rtc.c`
7. **Cache maintenance for DMA** — LCD pixelbuffer, FFT ring buffer
8. **SPI DMA on H7** — disabled, FIXME in `ui_lcd_hy28.c`

### Priority 3: Medium
9. **Audio vtable** — abstract I2S vs SAI
10. **Remove dead code** — USB Host compiled but never initialized
11. **Reduce newlib usage** — diag/trace code uses heavy newlib
12. **Split large files** — `ui_driver.c` (7653 lines), `audio_driver.c` (3051 lines)

---

## 16. What's Wrong (Fix)

| Issue | Location | Severity | Fix |
|---|---|---|---|
| No watchdog | Product code | Critical | Add `HAL_IWDG_Init()` + `HAL_IWDG_Refresh()` in main loop |
| F7/H7 fault handlers empty | `stm32f7xx_it.c`, `stm32h7xx_it.c` | High | Add register dump like F4 |
| BusFault missing on F7/H7 | `stm32f7xx_it.c`, `stm32h7xx_it.c` | High | Add BusFault_Handler |
| H7 RAM hardcoded 512KB | `uhsdr_board.c:469` | High | Implement real RAM detection |
| I2C timing F4-only | `uhsdr_hw_i2c.c:75` | High | Add F7/H7 timing calculation |
| H7 RTC FIXME | `uhsdr_rtc.c:128` | Medium | Implement H7 RTC init |
| Cache unmaintained | LCD, FFT buffers | Medium | Add `SCB_CleanDCache()` / `SCB_InvalidateDCache()` |
| SPI DMA disabled on H7 | `ui_lcd_hy28.c:29` | Medium | Fix H7 SPI DMA |
| USB Host dead code | `files.mak` | Low | Remove or `#ifdef USE_USBHOST` |
| newlib in diag | `drivers/diag/` | Low | Rewrite without newlib |
| `ui_driver.c` too large | `ui_driver.c` (7653 lines) | Low | Split into modules |

---

## 17. Non-Negotiable Rules

1. No `malloc`/`free` in ISR, audio, or RF paths (all MCUs) — **currently satisfied**
2. Watchdog always running in production builds (all MCUs) — **MISSING, must add**
3. HardFault/MemManage/BusFault/UsageFault handlers must have register dump (all MCUs) — **partial on F4, missing on F7/H7**
4. All blocking calls have bounded timeout (all MCUs)
5. ISR must be non-blocking and deterministic (all MCUs)
6. Vendor headers never in public API (all MCUs)
7. Static allocation is default (all MCUs) — **currently satisfied, heap = 0**
8. All string operations are bounded (all MCUs)
9. DMA buffers must be cache-line aligned + maintained (F7/H7) — **partial, LCD only**
10. Board-specific code stays in `board_configs/` or `hardware/`
11. Product code stays MCU-agnostic — use `uhsdr_mcu.h` abstractions where possible
12. All new code must compile on F4, F7, and H7 without modification

---

## 18. Migration Strategy

### Phase 1: Safety Critical (1-2 weeks)
1. Add watchdog init + kick in `uhsdr_main.c`
2. Add F7/H7 fault handlers with register dump (match F4 pattern)
3. Add BusFault_Handler on F7/H7
4. Add always-on stack guard enforcement

### Phase 2: Hardware Support (2-3 weeks)
5. Fix H7 RAM detection (replace hardcoded 512KB)
6. Add F7/H7 I2C timing abstraction
7. Implement H7 RTC support
8. Fix H7 SPI DMA

### Phase 3: Cache & Memory (1-2 weeks)
9. Add cache maintenance to LCD DMA buffers
10. Add cache maintenance to FFT ring buffer
11. Add audio interface vtable (I2S vs SAI)

### Phase 4: Cleanup (1-2 weeks)
12. Remove USB Host dead code or gate behind `USE_USBHOST`
13. Remove scattered `#ifdef` where possible (target: <20 remaining)
14. Split `ui_driver.c` and `audio_driver.c`
15. Update documentation

---

## 19. Current Codebase Health

### Strengths
- ✅ Shared product code across all MCUs — single `files.mak`
- ✅ Audio DMA ISR + PendSV pattern works across all MCUs
- ✅ Board config system with compile-time `#error` guards
- ✅ MCU abstraction in `uhsdr_mcu.h` (GPIO, CPU type, flash)
- ✅ Display detection logic works for both boards
- ✅ F4 HardFault handler has register dump
- ✅ BusFault handler for RAM detection on F4/F7
- ✅ SPI prescaler abstraction per MCU
- ✅ Static allocation — no heap usage in product code

### Weaknesses
- ❌ No watchdog (all MCUs)
- ❌ F7/H7 fault handlers are empty while(1) loops
- ❌ BusFault_Handler missing on F7/H7
- ❌ H7 RAM detection hardcoded to 512KB
- ❌ I2C timing F4-only (FIXME for F7/H7)
- ❌ H7 RTC not implemented (FIXME)
- ❌ Cache not maintained for LCD/FFT DMA (F7/H7)
- ❌ SPI DMA disabled on H7
- ❌ 49 scattered `#ifdef` in product code
- ❌ USB Host compiled but never initialized (dead code)
- ❌ diag/trace uses newlib (heavy)
- ❌ Large files: `ui_driver.c` (7653 lines), `audio_driver.c` (3051 lines)
