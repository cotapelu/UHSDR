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
make coverage             # Host tests + gcov report (test/coverage.txt)
make quality              # Local pre-commit gate (switch test, tests, identity, doctor)
make check-identity       # Verify every target reports its own board identity
make build-switch-test    # Verify configuration switches on a dirty tree
make static-analysis      # cppcheck gate (bounded, ~5 min; see docs/TODO.md T78)
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

## 10. Current State (Verified 2026-10-24, ARM GCC 13.2.1)

> All figures below were measured by building every target on a clean tree.
> `docs/TODO.md` Phase 66 holds the commands and the defect list.

### Implemented — verified
- ✅ **Firmware builds:** F4, F4-512KB, F7, H7 — 0 errors each
  (F7/H7 previously failed to link entirely; fixed in Phase 66)
- ✅ **Bootloader builds:** F4, F7, H7 — 0 errors each
- ✅ **Host tests:** 671 passing / 0 failing (53 files, 11 475 LOC)
- ✅ *Vendor HAL `#include` in compiled product code:* **0 files** — architecture goal met
  (Phase 69 removed the last 5; only the CMSIS *device* headers in `board_configs/`
  remain, which is deliberate: they supply the register bases `uhsdr_mcu.h` dereferences)
- ✅ *`HAL_*` calls in compiled product code:* **0** — delay path was the first
  (`HAL_Delay` → `hal_delay_ms`), Phase 69 closed the remaining 57 sites in
  RTC, ADC, DAC, TIM, DEVID/REVID, bootloader GPIO/NVIC/clock and the oscillator
  lock fields. The only `HAL_*` name left is the vendor-mandated callback
  `HAL_GPIO_EXTI_Callback`; every other match is a comment
- ✅ *`__HAL_*` macros, vendor field access (`->Lock`) and vendor status constants
  (`HAL_OK`/`HAL_ERROR`)* in product code: **0** (Phase 69)
- ✅ **HAL shim layer:** 19 abstract APIs in `hal/include`, backends per MCU
  (`hal_rtc`, `hal_nvic`, `hal_tim` added in Phase 69)
- ✅ Board config selection is now centralised in
  `hardware/board_configs/UHSDR_UI_board.h` (no product code names a board config)
- ✅ Watchdog, fault dump, stack canary, cache maintenance, CRC32 bootloader
- ✅ UART HAL driver now actually compiled (was an empty object on every target)
- ✅ Remote control module wired into the main loop (`REMOTE_UI_PAIR_ENABLE`)

### Remaining — measured, not estimated
- ✅ **57 `-Wlto-type-mismatch` → 0** — root cause FOUND & fixed (Phase 67): `uhsdr_types.h`
  mixed `bool`(`int`) vs `_Bool` across TUs; see docs/TODO.md Phase 67.1
- ✅ **Test coverage wired** — `make -C test coverage` → `coverage.txt` (Phase 67.2, T68)
- ✅ **Static analysis gates CI** — `.github/workflows/ci.yml` + cppcheck `--error-exitcode`
  (Phase 67.3, T70); vendored dirs suppressed wholesale. The gate is bounded
  and parallel (T78, 4–5 min) and the **product-code baseline is empty**
  (T76): 0 findings, 0 debt suppressions
- 🟡 **~219 product-code warnings total** (was 260 → measured 206 `missing-prototypes`,
  8 VLA-parameter, 3 `absolute-value`, plus 2 minor — see docs/TODO.md)
- 🟡 **8 `-Wvla-parameter` in `drivers/freedv/`** — re-framed: these are VLA *parameters*
  (pointer decay, not stack VLAs); risk lower than documented (T71)
- 🟡 **542 `#ifdef`/`#if defined` in product code** (the earlier "730" figure was wrong)
- 🟡 **184 `TODO`/`FIXME`/`HACK`**
- 🟡 Large file splits still open: `ui_driver.c` 6 430 L, `ui_lcd_hy28.c` 2 671 L
- 🔴 **Remote control unproven on hardware.** It was dead code until Phase 66; it now
  compiles, links and runs every main-loop iteration, but has never touched a real board.
- 🔴 **License is GPL-3.0** — blocks commercial redistribution of firmware

---

## 11. Success Metrics

| Metric | Measured (2026-10-28) | Target |
|---|---|---|
| Vendor HAL includes in product code | **0** | 0 ✅ |
| Direct `HAL_*` calls in product code | **0** (was 57 across 12 files) | 0 ✅ |
| `__HAL_*` macros / `->Lock` / vendor status constants in product code | **0** | 0 ✅ |
| Platform `#ifdef` in product code | 542 | <50 feature flags |
| Firmware targets building | **4 / 4** (was 2 / 4) | 4 ✅ |
| Bootloader targets building | **3 / 3** | 3 ✅ |
| Host tests | **671 passing** | 30+ ✅ |
| Host test coverage | **unknown** | >90% measured |
| Product-code compiler warnings | 260 (→ ~219 after Phase 67) | <25 |
| `-Wlto-type-mismatch` | 57 (→ **0** after Phase 67) | 0 |
| F4 binary (text) | 426 961 B (→ 434 897 B, +0.055 % after Phase 67) → **357 857 B** after Phase 69.7 | < 1 % regression/step |
| Time to add new STM32 MCU | 1 week (HAL backend + `uhsdr_mcu.h`) | ≤ 1 week ✅ |
| Time to add new board | 1 config header + `UI_BRD_*` flag | ≤ 1 day ✅ |
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

## 15. SDR Extension Analysis: 700 MHz / 2.3 GHz Video + Voice over COFDM/Mesh

**Version:** 0.1  
**Date:** 2026-09-10  
**Status:** Analysis — Design Pending

---

### 15.1 Executive Summary

Tài liệu này bổ sung phân tích khả năng mở rộng UHSDR từ **HF/UHF SSB/FM transceiver** hiện tại thành hệ thống **SDR một kênh video + voice** hoạt động tại **700 MHz** và **2.3 GHz**, sử dụng **COFDM** làm lớp vật lý và **mesh** làm lớp mac/routing. Phân tích dựa trên toàn bộ codebase hiện tại và các ràng buộc phần cứng/phần mềm đã biết.

**Kết luật tóm tắt:**
- **Voice:** khả thi ở cả 700 MHz và 2.3 GHz.
- **Video:** khả thi nhưng chặt; cần H7 là minimum hợp lý.
- **COFDM:** khả thi nhưng cần viết lại modem layer gần như từ đầu.
- **Mesh:** khả thi ở layer 2, không cần IP.
- **Không cần IP stack:** đúng, có thể làm custom PHY/MAC hoàn toàn độc lập.

---

### 15.2 Current Capability Assessment

#### 15.2.1 UHSDR hiện tại là gì

UHSDR hiện là firmware **bare-metal cooperative** cho STM32F4/F7/H7:
- Audio DMA ISR 1.5 kHz → PendSV → main loop.
- DSP audio 48 kHz: SSB/AM/FM/CW/RTTY/PSK/FreeDV.
- LO: Si570 hoặc Si5351.
- Codec audio: WM8731/WM8776.
- UI: LCD + encoder + keypad + menu + spectrum/waterfall.
- USB CDC, USB Audio, USB Host DFU.

UHSDR đã có board **LAPWING** (`RF_BRD_LAPWING`) chạy băng **1240–1300 MHz**, chứng minh codebase không bị khóa ở HF.

#### 15.2.2 Frequency coverage limits

| Component | Current range | Gap to target |
|---|---|---|
| Si5351A | ~2.3–290 MHz LO | 700 MHz cần converter; 2.3 GHz cần ngoại PLL |
| Si570 CMOS | 10–160 MHz LO | 700 MHz cần up-conversion |
| Si570 C-grade | 10–280 MHz LO | 2.3 GHz cần ngoại PLL |
| mcHF RF | HF/6m/2m/70cm | không có 700 MHz / 2.3 GHz RF frontend |
| OVI40 RF | HF/2m/70cm | không có 700 MHz / 2.3 GHz RF frontend |
| Lapwing RF | 1240–1300 MHz | đã có 1.2–1.3 GHz, nhưng không có 700 MHz / 2.3 GHz |

---

### 15.3 Service Layer: Voice + Video

#### 15.3.1 Voice

- **Khả thi:** cao.
- Codec đã có sẵn: **Codec2** (1300/3200 bit/s), **FreeDV**.
- Có thể thay thế bằng **AMBE+** nếu cần, nhưng cần codec phần cứng hoặc port software.
- Voice có thể được đóng gói trực tiếp vào khung custom mà không cần IP.

#### 15.3.2 Video

- **Khả thi:** có, nhưng chặt.
- Codebase hiện **không có video pipeline**.
- Cần thêm:
  - **Video source:** camera MIPI/CSI, USB webcam, hoặc overlay.
  - **Video encoder:** H.264 baseline hoặc MJPEG.
  - **Video decoder** nếu cần receive.
- H7 có **HAL_JPEG** cho JPEG image, nhưng không phải video stream.

#### 15.3.3 Codec recommendation

| Use case | Codec | Bitrate | Notes |
|---|---|---|---|
| Voice only | Codec2 1300 | 1300 bit/s | Đã có sẵn |
| Voice quality | Codec2 3200 | 3200 bit/s | Đã có sẵn |
| Digital voice narrow | AMBE+ | 3600 bit/s | Cần hardware/port |
| Video low res | MJPEG | 1–4 Mbps | Dễ implement |
| Video standard | H.264 baseline | 512 kbps–2 Mbps | Cần encoder |
| Video high quality | H.265 | 256 kbps–1 Mbps | Cần encoder, phức tạp |

---

### 15.4 Transmission Layer: COFDM

#### 15.4.1 COFDM không cần IP

**Có.** COFDM là **physical layer** hoàn toàn độc lập với IP.

- COFDM chỉ cần: **IFFT/FFT + pilot + cyclic prefix + equalizer**.
- Có mang trực tiếp **voice codec stream** hoặc **video elementary stream**.
- Có thể dùng **custom frame format** thay vì IP.

#### 15.4.2 Simplified COFDM design

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

#### 15.4.3 COFDM vs Single-carrier

| Feature | COFDM | Single-carrier QPSK/16QAM + FHSS |
|---|------|
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

### 15.5 Mesh Layer (No IP Required)

#### 15.5.1 Mesh không cần IP

**Có.** Mesh hoạt động ở **layer 2 (MAC)**, không cần IP stack.

- Định tuyến theo **Node ID**.
- Frame format: `[Header][Payload][CRC]`.
- Routing protocol: **AODV-like** hoặc **BATMAN-like**.

#### 15.5.2 Custom frame format

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

#### 15.5.3 MAC features needed

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

### 15.6 Hardware Requirements

#### 15.6.1 RF Frontend

| Band | LO/VCO | Mixer | PA | LNA | Filter |
|---|---|---|---|---|---|
| 700 MHz | ADF4351/ADF4360, LMX2582 | ADL5801, SIM7600 | 500 mW–1 W | 2–3 dB NF | SAW/ceramic |
| 2.3 GHz | LMX2582, HMC1031 | HMC219, ADL5375 | 100–500 mW | 2–3 dB NF | Microstrip/SAW |

#### 15.6.2 ADC/DAC requirements

| Band | Sample rate | Resolution | Notes |
|---|---|---|---|
| 700 MHz | 16–20 MSPS | 12–14 bit | Down-convert trước khi ADC |
| 2.3 GHz | 50–100 MSPS | 12–14 bit | Cần mixer + ADC ngoại |

#### 15.6.3 MCU selection

| MCU | Voice | Simple video | COFDM | Mesh | Notes |
|---|---|---|---|---|---|
| STM32F4 | ✅ | ❌ | ⚠️ Cực chật | ⚠️ 3 nodes | mcHF hiện tại |
| STM32F7 | ✅ | ⚠️ MJPEG | ✅ Cơ bản | ✅ 3–5 nodes | OVI40 F7 |
| STM32H7 | ✅ | ✅ H.264 baseline | ✅ Khả thi | ✅ 5–10 nodes | OVI40 H7 |

**Kết luận:** STM32H7 là **minimum hợp lý** cho video + COFDM + mesh. STM32F4 chỉ đủ cho voice single-carrier.

---

### 15.7 Software Requirements

#### 15.7.1 New oscillator backends

```
osc_si570.c          ← Đã có, extend cho 700 MHz
osc_si5351a.c        ← Đã có, extend
osc_adf4351.c        ← NEW: PLL 35–4400 MHz
osc_lmx2582.c        ← NEW: PLL 20–8000 MHz
```

#### 15.7.2 New RF board config

```
UHSDR_RF_700m_config.h    ← NEW
UHSDR_RF_2300m_config.h   ← NEW

- Band filter table (700 MHz / 2.3 GHz)
- Coupling table
- Power level table
- PA bias/max power limits
```

#### 15.7.3 Modem layer (PHY)

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

#### 15.7.4 Mesh layer (MAC + routing)

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

#### 15.7.5 Video pipeline

```
drivers/
├── video/
│   ├── video.h                 ← Video API
│   ├── video_capture.c         ← Camera input (MIPI/USB)
│   ├── video_encode.c          ← H.264 baseline / MJPEG
│   ├── video_decode.c          ← Receive side
│   └── video_mux.c             ← Mux with voice
```

#### 15.7.6 Audio enhancements

```
drivers/audio/
├── audio_voicemode.c          ← Voice-specific processing
├── audio_codec2.c              ← Codec2 encode/decode (đã có)
├── audio_ambe.c                ← AMBE+ (nếu cần)
```

---

### 15.8 Layer Architecture

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

### 15.9 Implementation Roadmap

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

### 15.10 Technology Comparison

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

### 15.11 Feasibility Summary

| Question | Answer | Notes |
|---|------|
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

### 15.12 Recommendations

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

### 15.13 Risk Analysis

| Risk | Severity | Mitigation |
|---|---|---|
| STM32 compute insufficient | Cao | Dùng H7, hoặc FPGA/SoC |
| RF frontend complexity | Cao | Thuê design nếu không có kinh nghiệm RF |
| COFDM synchronization | Trung bình | Bắt đầu single-carrier, thêm COFDM sau |
| Video bandwidth | Trung bình | Giới hạn resolution/fps, dùng MJPEG |
| Regulatory (700 MHz / 2.3 GHz) | Cao | Kiểm tra license, power limit, band plan |
| Thermal (PA 700 MHz / 2.3 GHz) | Trung bình | Heat sink, power control |

---

### 15.14 References

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

## 16. References

- `docs/TODO.md` — Task tracking for Phase 43+
- `mchf-eclipse/drivers/cat/cat_driver.c` — Existing CAT protocol implementation
- `mchf-eclipse/hardware/uhsdr_board.c` — Board init code
- `mchf-eclipse/drivers/ui/ui_driver.c` — UI input handling
- `mchf-eclipse/drivers/remote/remote.c` — Remote control module implementation
- `mchf-eclipse/drivers/remote/remote_protocol.c` — Packet framing & CRC8-CCITT

---

*This document is the single source of truth for architecture decisions. All implementation must align with this design.*