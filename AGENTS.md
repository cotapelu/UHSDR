# UHSDR Platform Architecture — STM32 Optimization Baseline v6.0

**Version:** 6.0  
**Language:** C11/C17 (GNU extensions)  
**Build:** GNU Make + ARM GCC + `.mak` fragments  
**Execution:** Bare-metal cooperative — Audio DMA ISR → PendSV → main loop  
**Memory Model:** Static allocation only; zero heap in product code  
**Target MCUs:** STM32F4 (Cortex-M4), STM32F7 (Cortex-M7), STM32H7 (Cortex-M7)  
**Target Boards:** mcHF UI (F4), OVI40 UI (F7/H7)  
**Safety Profile:** IEC 62304 Class B (practical)

---

## 1. Platform Vision

Tối ưu UHSDR thành **platform chuẩn cho STM32 Cortex-M4/M7**:

- 1 codebase, shared product code 100% giữa các MCU
- MCU abstraction tập trung vào `uhsdr_mcu.h` + `board_configs/`
- HAL shim layer tách product code khỏi vendor HAL (STM32)
- Zero `#ifdef` platform scatter trong product code
- Zero direct vendor HAL includes trong `drivers/` và `hardware/`
- Full safety: watchdog, fault handlers, stack guard, cache maintenance

---

## 2. Supported Hardware

### MCU Matrix

| MCU | Core | FPU | Flash | SRAM | CCM | Cache | Build |
|---|---|---|---|---|---|---|---|
| STM32F407xx | M4 | FPv4-SP | 1MB | 192KB | 64KB | None | `BUILDFOR=F4` |
| STM32F4-512KB | M4 | FPv4-SP | 512KB | 192KB | 64KB | None | `BUILDFOR=F4-512KB` |
| STM32F767xx | M7 | FPv5-D16 | 1MB+ | 320KB+ | 64KB | I-cache | `BUILDFOR=F7` |
| STM32H743xx | M7 | FPv5-D16 | 1MB+ | 512KB+ | — | I+D-cache | `BUILDFOR=H7` |

### Board Matrix

| Board | UI | RF | MCU | Config Header | Status |
|---|---|---|---|---|---|
| mcHF | ✅ | ✅ | **F4 only** | `UHSDR_UI_mchf_config.h` | Production |
| OVI40 | ✅ | ✅ | **F7/H7 only** | `UHSDR_UI_ovi40_config.h` | Production |

### Valid Build Matrix

| MCU | Board | Firmware | Bootloader | Status |
|---|---|---|---|---|
| F4 | mcHF | `fw-mchf_f4-mchf.bin` | `bl-mchf_f4-mchf.bin` | ✅ Default |
| F4-512KB | mcHF | `fw-mchf_f4-small.bin` | shared | ✅ Small |
| F7 | OVI40 | `fw-mchf_f7-ovi40.bin` | `bl-mchf_f7-ovi40.bin` | ✅ |
| H7 | OVI40 | `fw-mchf_h7-ovi40.bin` | `bl-mchf_h7-ovi40.bin` | ✅ |

---

## 3. Architecture Overview

```
mchf-eclipse/
├── hal/                           ← HAL SHIM LAYER (NEW — single HAL abstraction)
│   ├── include/                   ← Abstract API headers (no vendor includes)
│   │   ├── hal_gpio.h
│   │   ├── hal_spi.h
│   │   ├── hal_i2s.h              ← audio interface abstraction
│   │   ├── hal_i2c.h
│   │   ├── hal_dma.h
│   │   ├── hal_flash.h
│   │   ├── hal_watchdog.h
│   │   ├── hal_clock.h
│   │   └── hal_uart.h
│   └── src/                       ← STM32 backends (ONLY files with vendor HAL includes)
│       ├── gpio/hal_gpio_stm32.c
│       ├── spi/hal_spi_stm32.c
│       ├── i2s/hal_i2s_stm32.c
│       ├── i2c/hal_i2c_stm32.c
│       └── ...
│
├── vendors/                       ← VENDOR HAL (untouched, git submodule)
│   ├── stm32/                     ← STM32Cube HAL per MCU family
│   │   ├── Drivers/               ← HAL drivers (HAL v1.x per family)
│   │   ├── Middlewares/           ← USB, FatFs, etc.
│   │   └── Src/                   ← startup, main.c, IRQ handlers
│   │   ├── STM32F4xx/             ← basesw/mcHF/ (F4)
│   │   ├── STM32F7xx/             ← basesw/ovi40/ (F7)
│   │   └── STM32H7xx/             ← basesw/ovi40-h7/ (H7)
│
├── drivers/                       ← PRODUCT CODE (vendor-agnostic)
│   ├── audio/                     ← Audio pipeline (calls hal_i2s.h)
│   │   ├── audio_driver.c
│   │   ├── audio_filter.c
│   │   ├── audio_agc.c
│   │   ├── audio_nr.c
│   │   ├── audio_convolution.c
│   │   ├── audio_management.c
│   │   ├── cw/
│   │   ├── freedv/
│   │   ├── filters/
│   │   └── softdds/
│   ├── ui/                        ← UI (calls hal_spi.h, hal_gpio.h)
│   │   ├── ui_driver.c
│   │   ├── ui_driver_utils.c
│   │   ├── ui_display_list.c
│   │   ├── ui_encoder_display.c
│   │   ├── lcd/                   ← Display (calls hal_spi.h)
│   │   │   ├── ui_lcd_hy28.c
│   │   │   └── ui_lcd_layouts.c
│   │   ├── menu/
│   │   ├── encoder/
│   │   └── oscillator/
│   ├── cat/                       ← CAT (calls hal_uart.h)
│   ├── usb/                       ← USB Device (calls hal_usb.h)
│   └── diag/                      ← Debug trace (calls hal_uart.h)
│
├── hardware/                      ← BOARD ABSTRACTION (shared)
│   ├── uhsdr_mcu.h                ← MCU abstraction (GPIO, cache, clock, CPU type)
│   ├── uhsdr_board.c              ← Board init, LED, RTC, RAM detect
│   ├── uhsdr_fault.c              ← Shared fault handler (FaultHandler_Common)
│   ├── uhsdr_keypad.c             ← Keypad scanning
│   ├── uhsdr_hw_i2c.c             ← I2C bus (calls hal_i2c.h)
│   └── board_configs/             ← Compile-time board config
│       ├── UHSDR_UI_mchf_config.h
│       └── UHSDR_UI_ovi40_config.h
│
├── misc/                          ← UTILITIES (shared)
│   ├── config_storage.c
│   ├── serial_eeprom.c
│   ├── uhsdr_canary.c             ← Stack canary (static buffer, no heap)
│   └── v_eprom/                   ← Flash driver
│
├── src/
│   ├── uhsdr_main.c               ← Main loop, init order
│   └── bootloader/                ← DFU via USB Host + FatFs
│
├── test/                          ← HOST-BASED TESTS
│   ├── test.h                     ← Test framework
│   ├── test.c                     ← Runner
│   ├── test_audio_filters.c
│   ├── audio_driver_filters_standalone.c
│   └── test_canary.c
│
├── docs/
│   ├── architecture.md            ← This file (canonical)
│   ├── TODO.md                    ← Task tracking
│   └── reproducible_builds.md     ← Build reproducibility guide
│
├── files.mak                      ← Shared product code
├── include.mak                    ← Shared include paths
├── stm32-files.mak                ← STM32 HAL sources per MCU
├── Makefile                       ← Root build orchestrator
└── .gitignore
```

---

## 4. Execution Model

All MCUs follow identical execution model:

```
Audio DMA ISR (1.5 kHz)
    ├── HAL_DMA_IRQHandler()           [basesw]
    ├── AudioDriver_I2SCallback()      [audio_driver.c]
    ├── IQ decimation / interpolation
    ├── AGC, filters, CW, NR
    ├── ts.sysclock++                  (100 Hz timebase)
    └── PendSV trigger
         └── PendSV_Handler()          [basesw/Src/stm32fxx_it.c]
              └── UiDriver_TaskHandler_HighPrioTasks()
                   ├── FreeDv_HandleFreeDv()
                   ├── AudioNr_HandleNoiseReduction()
                   └── RadioManagement_HandlePttOnOff()

Main Loop (cooperative)
    ├── UiDriver_TaskHandler_MainTasks()
    ├── Encoder/keys @ 100 Hz
    ├── Spectrum/waterfall @ 40-80 ms
    ├── Meters @ 25 Hz
    ├── Menu/UI rendering
    ├── Canary_IsIntact()             (stack guard)
    └── HAL_IWDG_Refresh()            (watchdog kick)
    └── __WFI()                        (low-power idle)
```

**Contract:**
- ISR: non-blocking, < 100 µs, no printf, no malloc
- PendSV: deferred low-priority work
- Main loop: cooperative, bounded timeout on all blocking calls

---

## 5. Layer Contracts

### 5.1 HAL Shim Layer (`hal/`)
**Purpose:** Single point of vendor abstraction. Product code calls `hal_*` APIs; STM32 backends call HAL functions.

**Rules:**
- Headers in `hal/include/` — zero vendor includes, zero STM32 types
- Sources in `hal/src/*/` — ONLY these files include `stm32f4xx_hal.h` / `stm32f7xx_hal.h` / `stm32h7xx_hal.h`
- Every interface has: config struct + vtable + factory function
- Error handling: unified `hal_status_t` (`HAL_OK`, `HAL_ERR_*`)

```
drivers/audio/audio_driver.c
    │  #include "hal_i2s.h"          ← abstract API
    ▼
hal/src/i2s/hal_i2s_stm32.c
    │  #include "stm32f4xx_hal.h"    ← vendor HAL ONLY here
    │  audio_iface_t *iface = hal_i2s_create(CPU_STM32F4)
    │  iface->start_rx(iface, rx_cb, user_data)
    ▼
basesw/mcHF/Drivers/STM32F4xx_HAL_Driver/
    HAL_I2S_Receive_DMA()
```

### 5.2 Vendor Layer (`basesw/`)
**Purpose:** STM32Cube HAL + startup + linker scripts.

**Rules:**
- NEVER modify files in `basesw/` directly
- Updates via git submodule pin (tagged release)
- Owns: `main.c`, `stm32fxx_it.c`, `stm32fxx_hal_msp.c`, startup files

### 5.3 Product Layer (`drivers/`, `hardware/`, `misc/`, `src/`)
**Purpose:** Application logic — 100% shared across MCUs.

**Rules:**
- NEVER include `stm32f4xx_hal.h`, `stm32f7xx_hal.h`, `stm32h7xx_hal.h`
- NEVER call `HAL_*`, `HAL_RCC_*`, `GPIOx->` directly
- Use `hal_*` APIs or `uhsdr_mcu.h` abstractions
- Board-specific code ONLY in `board_configs/`

### 5.4 `uhsdr_mcu.h` Abstractions

| Abstraction | Implementation |
|---|---|
| `CPU_GetType()` | Read CPU ID register, return `CPU_STM32F4/F7/H7` |
| `GPIO_SetBits(port, pin)` | H7: BSRRL/BSRRH; F4/F7: BSRR |
| `CPU_Cache_CleanDCache(addr, len)` | F4: no-op; F7/H7: `SCB_CleanDCache_by_Addr()` |
| `CPU_Cache_InvalidateDCache(addr, len)` | F4: no-op; F7/H7: `SCB_InvalidateDCache_by_Addr()` |
| `DMA_BUFFER_ALIGN` | 32 (F7 cache line); 1 (F4 no-cache); 32 (H7) |
| `DMA_BUFFER_SECTION` | `.ccm` (F4); `.ram2` (F7); `.dma_mem` (H7) |

---

## 6. Memory Model

### Sections

| Section | F4 | F7 | H7 | Content |
|---|---|---|---|---|
| `.ccm` | 64KB @ 0x10000000 | 64KB @ 0x10000000 | — | Audio filters, spectrum, NR states |
| `.ram2` | — | 128KB @ 0x20000000 | — | DMA buffers (F7) |
| `.dma_mem` | — | — | 128KB @ 0x30000000 | DMA buffers (H7) |
| `.ram` | 128KB @ 0x20000000 | 256KB @ 0x20000000 | 512KB @ 0x24000000 | Default data, stack |

### Allocation Rules

```
1. Static arrays only — NO malloc/free in ISR, audio, RF paths
2. DMA buffers: __ALIGN(CPU_DMA_BUFFER_ALIGN) + __UHSDR_DMAMEM
3. Filter states: __MCHF_SPECIALMEM → .ccm on F4, .ram on F7/H7
4. Stack guard: static buffer (uhsdr_canary.c)
```

---

## 7. Safety Mechanisms

### 7.1 Watchdog
- `HAL_IWDG_Init()` in `Board_WatchdogInit()` — IWDG1 on H7, IWDG on F4/F7
- `HAL_IWDG_Refresh()` every 1s in main loop (`WATCHDOG_KICK_TICKS = 100`)

### 7.2 Fault Handlers
All MCUs: HardFault, MemManage, BusFault, UsageFault call `FaultHandler_Common()` — register dump + infinite loop.

### 7.3 Stack Guard
- `uhsdr_canary.c`: static buffer, checked every main loop iteration
- LED indication on corruption (red + green)

### 7.4 Cache Maintenance
- `DMA_BUFFER_CLEAN()` / `DMA_BUFFER_INVALIDATE()` macros
- Applied to: LCD pixelbuffer, FFT ring buffer
- `Board_Reboot()`: `SCB_CleanDCache()` on F7/H7

### 7.5 Bootloader Safety
- CRC32 firmware validation
- Anti-rollback version string
- 3-strike boot counter (SRAM2/RTC backup)
- Recovery mode after N failed boots

---

## 8. Build System

### Key Files

| File | Purpose |
|---|---|
| `Makefile` | Root orchestrator — `make f4-mchf`, `make all-firmware` |
| `files.mak` | Shared product code (all MCUs) |
| `include.mak` | Shared include paths |
| `stm32-files.mak` | STM32 vendor HAL sources |
| `*-bootloader.mak` | Bootloader-only sources |

### Commands

```bash
make f4-mchf              # Default build (F4 firmware + bootloader)
make BUILDFOR=F7 BOARD=ovi40 firmware   # F7 firmware only
make BUILDFOR=H7 BOARD=ovi40 both       # H7 firmware + bootloader
make all-firmware         # All 4 valid firmware combos
make all-bootloader       # All 3 valid bootloader combos
make doctor               # Pre-flight environment check
make check                # Repo sanity check
make info                 # Build matrix summary
make size-summary         # Flash/RAM usage report
make test                 # Host unit tests
```

---

## 9. Coding Standards

### 9.1 Naming

| Scope | Pattern | Example |
|---|---|---|
| Public API | `Module_Function()` | `AudioDriver_SetFreq()` |
| HAL API | `HAL_Module_Function()` | `HAL_GPIO_WritePin()` |
| Private | `static` | `static void encoder_update()` |
| Config struct | `module_config_t` | `audio_config_t` |
| Context struct | `module_context_t` | `audio_context_t` |
| Board config | `BOARD_*` | `BOARD_LCD_WIDTH` |
| MCU macro | `CPU_*` / `STM32*` | `CPU_GetType()` |

### 9.2 Error Handling

```c
typedef enum {
    HAL_OK = 0,
    HAL_ERR_TIMEOUT = -1,
    HAL_ERR_BUSY = -2,
    HAL_ERR_INVALID_PARAM = -3,
    HAL_ERR_NOT_SUPPORTED = -4,
    HAL_ERR_HW_FAILURE = -5,
} hal_status_t;
```

All HAL shims return `hal_status_t`. Product code checks return values.

### 9.3 State Management

```c
// ❌ FORBIDDEN: file-scope static in product code
static uint32_t hidden_state;  // BAD

// ✅ ALLOWED: explicit context struct
typedef struct { uint32_t state; } audio_context_t;
int AudioDriver_Init(audio_context_t *ctx, ...);

// ✅ ALLOWED: documented global (uhsdr_board.h)
extern __IO TransceiverState ts;
```

### 9.4 Include Rules

```
Product code (drivers/, hardware/, misc/, src/):
  ✅ #include "uhsdr_mcu.h"
  ✅ #include "hal_i2s.h"
  ❌ #include "stm32f4xx_hal.h"     ← FORBIDDEN
  ❌ #include "stm32f7xx_hal.h"     ← FORBIDDEN
  ❌ HAL_I2S_Receive_DMA()          ← FORBIDDEN

HAL backends (hal/src/*/):
  ✅ #include "stm32f4xx_hal.h"     ← ONLY here
  ✅ #include "hal_i2s.h"
```

---

## 10. Current State (Verified 2026-08-21)

### Implemented
- ✅ Watchdog: `HAL_IWDG_Init()` + refresh every 1s
- ✅ Fault handlers: F4/F7/H7 register dump via `FaultHandler_Common()`
- ✅ Stack guard: static buffer, main loop check
- ✅ Cache maintenance: LCD + FFT DMA buffers
- ✅ Audio vtable: I2S (F4) vs SAI (F7/H7)
- ✅ H7 RAM detection: 128KB/256KB/512KB/1024KB
- ✅ I2C timing: F4/F7/H7 abstraction
- ✅ H7 RTC: LSE/LSI init
- ✅ Bootloader safety: CRC32 + anti-rollback + 3-strike
- ✅ Low-power idle: `__WFI()` in main loop
- ✅ CI + unit tests + size regression detection

### Remaining (STM32-specific)
- 🔄 HAL shim layer: 15 abstract APIs for GPIO/SPI/I2S/DMA etc. (Phase 1)
- 🟡 Large file splits: `ui_driver.c` (6637L), `audio_driver.c` (2799L), `ui_lcd_hy28.c` (2683L)
- 🟡 Global state: encapsulate in context structs
- 🟡 `#ifdef` reduction: 730 total; ~27 platform guards remain in hardware abstraction

---

## 11. Success Metrics

| Metric | Current | Target |
|---|---|---|
| Vendor HAL includes in product code | 10+ files | 0 |
| Direct HAL calls in product code | 50+ | 0 |
| Platform `#ifdef` in product code | ~27 | <10 |
| Time to add new STM32 MCU | 2-3 weeks | 1 week (HAL shim only) |
| Time to add new board | 1-2 days | 1 day (config header only) |
| Host test coverage | 5 tests | 30+ tests |
| Binary size regression tolerance | — | < 1% |
| Build time (all targets) | ~5 min | <10 min |

---

## 12. How to Add a New STM32 MCU

1. Add `basesw/stm32/STM32<New>xx/` — CubeMX HAL + startup
2. Add `new-files.mak` — vendor HAL sources
3. Add `new-include.mak` — vendor include paths
4. Add `hal/src/*/hal_*_stm32.c` variant OR extend existing STM32 backend
5. Update `uhsdr_mcu.h` — add `CPU_STM32<NEW>` enum + cache/flash macros
6. Add `UHSDR_UI_<new>_config.h` board config OR reuse existing
7. `make BUILDFOR=<new> BOARD=<board> firmware`

---

## 13. How to Add a New Board

1. Copy existing `board_configs/UHSDR_UI_*_config.h`
2. Change pin assignments, LCD type, keypad map
3. Add `#ifdef` board-specific macros in new header
4. `make BOARD=<new> firmware`

---

## 14. Non-Negotiable Rules

1. No `malloc`/`free` in ISR, audio, or RF paths
2. Watchdog always running in production builds
3. Fault handlers must have register dump (all MCUs)
4. All blocking calls have bounded timeout
5. ISR is non-blocking and deterministic
6. Vendor headers NEVER in public API of `drivers/`, `hardware/`, `misc/`, `src/`
7. Static allocation is default — heap = 0
8. All string operations are bounded
9. DMA buffers are cache-line aligned + maintained (F7/H7)
10. Board-specific code ONLY in `board_configs/`
11. Product code is MCU-agnostic — use `hal_*` APIs
12. All new code compiles on F4, F7, and H7 without modification