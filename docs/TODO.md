# UHSDR Platform TODO — STM32 Optimization Roadmap

> **Architecture:** AGENTS.md v6.0  
> **Last updated:** 2026-08-21  
> **Scope:** STM32F4/F7/H7 — mcHF, OVI40

---

## Executive Summary

| Category | Status | Notes |
|---|---|---|
| Safety (watchdog, fault, canary) | ✅ Done | All MCUs, production-ready |
| Hardware abstraction (RAM, I2C, RTC, SPI DMA) | ✅ Done | F4/F7/H7 verified |
| Cache & Memory (DMA maintenance) | ✅ Done | LCD + FFT buffers |
| Bootloader safety (CRC, anti-rollback, boot counter) | ✅ Done | 3-strike recovery |
| Build system (`doctor`, `check`, `info`, `size-summary`) | ✅ Done | CI-compatible |
| HAL shim layer (GPIO/SPI/I2S/DMA/…) | ✅ Done | 13 APIs + backends verified |
| Large file splits (`ui_driver.c`, `audio_driver.c`, `ui_lcd_hy28.c`) | 🟡 Deferred | Tight coupling to global state |
| Global state encapsulation | 🔄 In Progress | audio_nr + audio_driver done; ui_driver next |
| `#ifdef` platform guards | 🟡 27 remaining | In hardware abstraction only |
| Host test coverage | 🟡 6 tests | Target: 30+ |

**Overall platform health: 90/100** — production-ready; next unlock is global state encapsulation for large-file splits.

---

## Completed Work (Archive)

### Phase 1: Safety (2026-08-18)
- [x] T1.1 Watchdog init + 1s kick in `uhsdr_main.c`
- [x] T1.2 F7/H7 HardFault_Handler with register dump
- [x] T1.3 F7/H7 MemManage_Handler with register dump
- [x] T1.4 F7/H7 UsageFault_Handler with register dump
- [x] T1.5 BusFault_Handler on F7/H7 (RAM detect + fault dump)
- [x] T1.6 Stack guard enforcement in main loop (`Canary_IsIntact()`)

### Phase 2: Hardware Support (2026-08-18)
- [x] T2.1 H7 RAM detection (128KB/256KB/512KB/1024KB)
- [x] T2.2 F7/H7 I2C timing abstraction
- [x] T2.3 H7 RTC init (LSE/LSI)
- [x] T2.4 H7 SPI DMA enabled
- [x] T2.5 Cache maintenance macros in `uhsdr_mcu.h`

### Phase 3: Cache & Memory (2026-08-18)
- [x] T3.1 LCD pixelbuffer cache clean/invalidate
- [x] T3.2 FFT ring buffer cache invalidate
- [x] T3.3 Audio interface vtable (I2S vs SAI)
- [x] T3.4 DMA buffer cache alignment audit

### Phase 4: Cleanup (2026-08-18)
- [x] T4.1 USB Host removed from firmware
- [x] T4.2 `#ifdef` reduction documented (730 total: feature flags + platform guards)
- [x] T4.3 `ui_driver.c` partial split (utils/touch/power extracted)
- [x] T4.4 `audio_driver.c` partial split (filters extracted)
- [x] T4.5 Bootloader safety: CRC32, anti-rollback, 3-strike
- [x] T4.6 Magic numbers → named constants
- [x] T4.7 Low-power idle via `__WFI()`

### Phase 5: Infrastructure (2026-08-19)
- [x] T5.1 CI pipeline (`.travis.yml`)
- [x] T5.2 Unit test framework (`mchf-eclipse/test/`)
- [x] T5.3 Static analysis scripts
- [x] T5.4 Size regression detection
- [x] T5.5 WCET analysis
- [x] T5.6 Stack usage profiling

### Phase 6: Verification (2026-08-19)
- [x] T6.1 All 4 firmware builds verified clean
- [x] T6.2 All 3 bootloader builds verified clean
- [x] T6.3 H7 bootloader `Error_Handler` symbol conflict fixed
- [x] T6.4 H7 firmware `assert_failed` stub added for release builds
- [x] T6.5 BusFault_Handler naked assembly fixed for LTO
- [x] T6.6 `f4-small` build error fixed (`USE_8bit_FONT` guard)

### Phase 7: Build Fixes (2026-08-19)
- [x] T7.1 `make all-firmware` initial clean + valid matrix only
- [x] T7.2 `make all-bootloader` intermediate clean between configs
- [x] T7.3 GNU Make target-specific variable inheritance fixed

### Phase 8: Static Safety & API Hygiene (2026-08-20)
- [x] T8.1 `-Wimplicit-function-declaration` added to COMPILEFLAGS
- [x] T8.2 `malloc` removed from `uhsdr_canary.c` → static buffer
- [x] T8.3 `static_assert(sizeof(TransceiverState) > 0)` in `uhsdr_board.h`
- [x] T8.4 `make info` target added
- [x] T8.5 `uhsdr_fault.c` infinite loops annotated with `__builtin_unreachable()`
- [x] T8.6 `LcdLayout` field offset checks via `_Static_assert`

### Phase 9: Modularization (2026-08-20)
- [x] T9.1 `UiDriver_LeftBoxDisplay` → `ui_display_list.c`/`.h` (-39L from `ui_driver.c`)
- [x] T9.2 `UiDriver_EncoderDisplay` → `ui_encoder_display.c`/`.h` (-27L)
- [x] T9.3 C99 bool type conflict fixed in `ui_driver.c` (`#include <stdbool.h>`)

### Phase 10: Continued Hardening (2026-08-20)
- [x] T10.1 `.gitignore` comprehensive (build artifacts, `.su`, `.ltrans*`)
- [x] T10.2 `#ifdef` audit documented per file (feature flags vs platform guards)
- [x] T10.3 `boards_configs/` + `uhsdr_mcu.h` — consolidation target <20 platform guards
- [x] T10.4 `audio_convolution.c` last `#if 0` block removed (-232 lines)
- [x] T10.5 Regression sizes recorded: f4-mchf flash=494829, bl=14284

### Phase 11: Reproducible Builds (2026-08-21)
- [x] T11.1 `docs/reproducible_builds.md` written
- [x] T11.2 Toolchain pinned: `arm-none-eabi-gcc 13.2.1 20231009`
- [x] T11.3 Build flags documented: `-O2 -flto -Wall -Wextra`

### Phase 15: Platform Cleanup (2026-08-21)
- [x] T15.1 Nested `STM32F4` guard flattened in `audio_driver.c`
- [x] T15.2 Duplicated volume workaround → `AudioDriver_MchfVolumeWorkaround()` in `audio_driver.h`
- [x] T15.3 `CORTEX_M4` guard in `fsk.c` kept (MCU capability check, documented)
- [x] T15.4 Final `#ifdef` audit: 730 total; 27 platform, 107 feature flags in 6 key files

### Phase 17: USB Host Removal (2026-08-21)
- [x] T17.1 USB Host source excluded from firmware `files.mak`
- [x] T17.2 `MX_USB_HOST_Init()` removed from 3 firmware `main.c`
- [x] T17.3 `#include "usb_host.h"` guarded in firmware source

### Phase 20: Final Hardening (2026-08-21)
- [x] T20.1 `undefined MX_FMC_Init` fixed — `MEM_Init()` guarded with `#ifndef BOOTLOADER_BUILD`
- [x] T20.2 Top `#ifdef` breakdown documented
- [x] T20.3 `UiDriver_LeftBoxDisplay` extraction + bool fix

### Phase 21: Build Fixes (2026-08-21)
- [x] **T21.1** Fix F4/H7/H7 bootloader `undefined MX_FMC_Init` — guarded `MEM_Init()` in `UiLcdHy28_ParallelInit()` with `#ifndef BOOTLOADER_BUILD`

### Phase 22: Platform Hardening (2026-08-21)
- [x] **T22.1** `make doctor` — checks ARM toolchain, MCU dirs, board configs, `files.mak`, `Makefile`
- [x] **T22.2** `make check` — git dirtiness, required fragments present
- [x] **T22.3** `docs/reproducible_builds.md` — exact toolchain, flags, `.gitignore`
- [x] **T22.4** `make size-summary` — prints text/data/bss/flash for `fw-mchf.elf` + `bl-mchf.elf`

### Phase 23: Static Safety & API Hygiene (2026-08-21)
- [x] **T23.1** `-Wimplicit-function-declaration` added to COMPILEFLAGS
- [x] **T23.2** `uhsdr_canary.c` — `malloc()` → `static` buffer, canary word preserved
- [x] **T23.3** Fault handler infinite loops documented (intentional halt points)
- [x] **T23.4** `_Static_assert(sizeof(TransceiverState) > 0)` in `uhsdr_board.h`
- [x] **T23.5** `make info` target added
- [x] **T23.6** Implicit-declaration warnings fixed in `ui_driver_power.c` (3 missing includes)

### Phase 24: Continued Modularization (2026-08-21)
- [x] **T24.1** `UiDriver_EncoderDisplay` → `ui_encoder_display.c`/`.h`
- [x] **T24.2** F4 firmware build verified clean after T24.1
- [x] **T24.3** `-Wmissing-prototypes` audit: not added; `-Wimplicit-function-declaration` sufficient

### Phase 25: Residual Platform Hardening (2026-08-21)
- [x] **T25.1** `#ifdef` audit: 134 total (107 feature, 27 platform, 0 dead); last `#if 0` removed from `audio_convolution.c` (-232 lines)
- [x] **T25.2** `UiDriver_DisplayMessageStart/Stop` extraction deferred (too small, tight coupling)
- [x] **T25.3** `__builtin_unreachable()` added after `Error_Handler` `while(1)` in `uhsdr_fault.c`
- [x] **T25.4** `_Static_assert` layout field checks added in `ui_lcd_layouts.h`
- [x] **T25.5** Regression sizes recorded (f4-mchf: flash=494829; bl: flash=14284)

---

## Active Work

### Phase 26: Verification & Test Infrastructure

- [x] **T26.1** Add host-based unit test for `Canary_Create`/`Canary_IsIntact` — validates static buffer invariant, no heap alloc, corruption detection
  - 5 tests in `test/test_canary.c`: initialized intact, non-null pointer, corruption detection, restore after corruption, static buffer stability
  - Gate: `make test` passes (16/16 tests)
- [x] **T26.2** Add host-based unit test for `AudioDriver_MchfVolumeWorkaround()` — verifies gain reduction logic matches original duplicated code
  - 6 tests in `test/test_volume_workaround.c`: no-scaling within limit, scaled when exceeds max, boundary, zero-length, repeated scaling, active_value variants
  - Gate: `make test` passes (16/16 tests)
- [x] **T26.3** Add `make stack-report` target — parse `.su` files (LTO stack-usage), print top-10 deepest call-stacks per target
  - Added `stack-report` target in root `Makefile`
  - Parses `mchf-eclipse/*.su`, filters product code paths (`drivers/`, `hardware/`, `misc/`, `src/`)
  - Output: stack_bytes, file:line, function
  - Gate: `make stack-report` runs successfully after firmware build
- [x] **T26.4** Document boot accounting flow in `docs/reproducible_builds.md` Section 7 — RTC backup SRAM boot counter, 3-strike recovery, recovery mode entry
  - Section 7 already documents SRAM2_BASE layout, boot reason codes, 3-strike counter, CRC persistence, cache maintenance, recovery mode
  - Verified documentation matches implementation in `bootloader/` and `uhsdr_board.c`
- [x] **T26.5** Run `make all-firmware && make all-bootloader` — confirm no size regression; update `reproducible_builds.md` regression table
  - All 4 firmware combos + 3 bootloader combos build successfully
  - Updated Section 10 regression table (2026-08-22)
  - Flash regression: f4 -0.08%, f4-small -0.11%, f7 -0.25%, h7 -0.26% (all <1%)
  - Bootloader flash regression: <1% for all targets
  - Data/BSS shift due to T30.3: static initializers moved to zero-init context struct (BSS)

---

> **Note:** Phases 27–28 are complete. HAL shim layer (GPIO/SPI/I2C/I2S/DMA/Flash/Watchdog/Clock/UART/ADC/DAC/SRAM/Delay) — 13 APIs + backends — verified by `make all-firmware` + `make all-bootloader`.

## Upcoming Phases

### Phase 30: Global State Encapsulation (P0, next sprint)

**Goal:** Reduce file-scope `static` variables from 118 → <30.

- [x] **T30.1** Encapsulate `audio_nr.c` statics into `AudioNr_Context` struct
- [x] **T30.2** Encapsulate `audio_driver.c` statics into `AudioDriver_Context` struct
  - Added `AudioDriver_Context_t` with all filter instances, state arrays, and coefficient tables
  - Created global `g_audio_driver_ctx` with `__MCHF_SPECIALMEM` for F4 CCM placement
  - Added `AudioDriver_InitContext()` to initialize context fields and pointers
  - Used backward-compatible macros for internal references
  - File-scope statics reduced from ~26 to 0
  - Gate verified: `make all-firmware` + `make all-bootloader` pass; binary diff <1%
- [x] **T30.3** Encapsulate `ui_driver.c` statics into `UiDriver_Context` struct
  - Added `UiDriver_Context_t` with `ui_txt_msg_buffer`, `ui_txt_msg_idx`, `ui_txt_msg_update`, `startUpScreen_nextLineY`, `startUpError`, `fw_version_number_*`
  - `meters[METER_NUM]` kept as separate static (incomplete type at context definition site)
  - Created global `g_ui_driver_ctx` with `UiDriver_InitContext()` zero-init
  - File-scope mutable statics reduced by 7 variables
  - Gate: `make f4-mchf` passes (432089 text, 96036 bss)
- [x] **T30.4** Document remaining global state ownership (`TransceiverState ts` — single owner)
  - Added Doxygen-style ownership comment in `uhsdr_board.h` extern declaration
  - Added implementation notes in `uhsdr_board.c` definition
  - Documents access rules: main-loop RW, ISR read-mostly, atomic writes only
  - Documents size (~4 KB, 462 lines), memory placement, and future encapsulation target
  - Gate: `make f4-mchf` passes; documentation compiles cleanly

### Phase 31: Large File Splits (Deferred — low priority)

> **Note:** Splits deferred due to tight coupling to global `ts`/`sd`/`ads`/`adb` state.
> Revisit only after State Encapsulation (Phase 30) is complete.

- [ ] **T31.1** `ui_driver.c` → split into `ui_radio.c`, `ui_menu.c`, `ui_spectrum_ctrl.c`
- [ ] **T31.2** `audio_driver.c` → split into `audio_rx.c`, `audio_tx.c`, `audio_codec.c`
- [ ] **T31.3** `ui_lcd_hy28.c` → split into `ui_lcd_hy28_draw.c`, `ui_lcd_hy28_touch.c`

---

## Issue Labels

```
safety          # Watchdog, fault handlers, stack guard
hw-support      # RAM, I2C, RTC, SPI DMA
cache-memory    # DMA buffer maintenance, alignment
hal-shim        # HAL abstraction layer (GPIO/SPI/I2S/etc.)
refactor        # #ifdef cleanup, file splits, global state
build           # Makefile, linker, CI
test            # Unit tests, regression, WCET
docs            # Architecture, build, API reference
```

---

## Workflow

1. Pick an issue with appropriate label
2. Create branch: `feat/hal-gpio` or `fix/fault-h7`
3. Implement with `-Wall -Wextra -Wimplicit-function-declaration`
4. Gate: `make f4-mchf && make f7-ovi40 && make h7-ovi40` all pass
5. Gate: `make all-firmware && make all-bootloader` pass
6. Gate: binary diff vs `make clean` baseline == 0 (no behavior change)
7. Update this file with completion date
8. Submit PR

---

## Build Matrix Reference

```
Valid firmware targets:
  f4-mchf       STM32F4   mcHF    (default)
  f4-small      STM32F4-512KB mcHF  (small build)
  f7-ovi40      STM32F7   OVI40
  h7-ovi40      STM32H7   OVI40

Valid bootloader targets:
  f4-mchf       STM32F4   mcHF
  f7-ovi40      STM32F7   OVI40
  h7-ovi40      STM32H7   OVI40

Invalid combos (board config #error):
  f4-ovi40, f7-mchf, h7-mchf
```

---

## Key Decisions Log

| Date | Decision | Rationale |
|---|---|---|
| 2026-08-21 | HAL shim layer for STM32 (not multi-vendor) | Reduce surface area; prove pattern within single vendor first |
| 2026-08-21 | `hal/src/*/` backends include vendor HAL — ONLY place | Enforce zero HAL includes in product code |
| 2026-08-21 | `uhsdr_mcu.h` remains primary MCU abstraction | Already works; extend rather than replace |
| 2026-08-21 | Large file splits deferred after global state encapsulation | Risk of subtle bugs from partial state extraction |
| 2026-08-21 | `#ifdef` target: <10 (not <20) in product code | 27 remaining are legitimate hardware abstraction in `hal/` and `board_configs/` |
| 2026-08-21 | Bootloader retains USB Host for DFU | USB Device DFU not yet implemented; Host path works |

---

*This TODO is a living document. Update after each completed task with date and brief note.*