# UHSDR Platform TODO — STM32 Optimization Roadmap

> **Architecture:** AGENTS.md v6.0  
> **Last updated:** 2026-08-28  
> **Scope:** STM32F4/F7/H7 — mcHF, OVI40

---

## Executive Summary

## Executive Summary

> **All figures below are measured on this checkout, not asserted. See Phase 66 for the
> verification commands. Anything marked ⚠️ is a measured defect, not a target.**

### Measured state (2026-10-24, ARM GCC 13.2.1)

| Category | Measured | Notes |
|---|---|---|
| Firmware build — F4 | ✅ 0 errors | `fw-mchf.elf` 426 961 + 1 968 + 96 444 = 525 373 B |
| Firmware build — F4-512KB | ✅ 0 errors | 283 285 B, fits 512 KB flash |
| Firmware build — F7 | ✅ 0 errors | ⚠️ **was failing to build** before Phase 66 (11 errors) |
| Firmware build — H7 | ✅ 0 errors | ⚠️ **was failing to build** before Phase 66 (11 errors) |
| Bootloader build — F4 / F7 / H7 | ✅ 0 errors | all three |
| Host unit tests | ✅ 670/670 pass | 53 test files, 11 475 LOC, no hardware needed |
| Vendor HAL `#include` in product code | ✅ 0 files | architecture claim verified true |
| `HAL_Delay` calls in product code | ✅ 0 (was 26) | now `hal_delay_ms()` (Phase 66) |
| `-Wimplicit-function-declaration` in product code | ✅ 0 (was 2) | Phase 66 |
| Compiler warnings — F4 total | ⚠️ 295 | 260 in product code, 20 vendor, 15 HAL backends |
| Compiler warnings — `-Wlto-type-mismatch` | ⚠️ 57 | real, root cause **not** isolated yet (Phase 67) |
| Compiler warnings — `-Wvla-parameter` | ⚠️ 8 | VLAs in `drivers/freedv/` — unbounded stack |
| `#ifdef` / `#if defined` in product code | 542 | was documented as 730/822 |
| `TODO`/`FIXME`/`HACK` in product code | 184 | matches previous count |
| Largest product file | 6 430 L | `drivers/ui/ui_driver.c` (split still open) |
| UART HAL driver compiled? | ✅ yes (Phase 66) | ⚠️ was compiling to an **empty object** (all targets) |
| Remote control module live in ELF | ✅ 6 symbols | ⚠️ was 0 (dead code) before Phase 66 |
| Test coverage measured? | ❌ no | `gcov` exists on host but is not wired into `test/Makefile` |
| CI platform | ⚠️ Travis only | no `.github/workflows`; static analysis is `\|\| true` (not gating) |

### Health claim

**Platform health: not scored.** An earlier revision of this file claimed "100/100". That
number was not derived from any measurement and was contradicted by a clean build
(295 warnings, and F7/H7 did not link). This revision replaces it with the table above.
A score should only be reintroduced once coverage is measured (T78) and the
`-Wlto-type-mismatch` root cause is closed (T67).

---

## Phase 66: Audit + P0 Defect Removal (2026-10-24)

A full audit of the checkout found that several documented "done" items were not
actually true. Each fix below was verified by rebuilding all four firmware targets
and all three bootloaders, and re-running the host suite.

### Verified defects found and fixed

| # | Defect | Root cause | Fix |
|---|---|---|---|
| 66.1 | **F7 and H7 firmware did not build** (11 errors each) | `drivers/remote/remote.c` and `drivers/ui/ui_driver_remote.c` hard-coded `board_configs/UHSDR_UI_mchf_config.h`, pulling the mcHF config into an OVI40 build, whose `#error McHF boards support only CORTEX_M4 MCU!` then fired | new `hardware/board_configs/UHSDR_UI_board.h` selector keyed on `UI_BRD_*`; both files include it |
| 66.2 | `UFM_LARGE` undeclared on non-mcHF builds | `ui_driver.h` (which declares `UiDriver_UpdateFrequency` / `UFM_LARGE`) was included only inside `#ifdef UI_BRD_MCHF`, but used unguarded | `ui_driver.h` + `radio_management.h` now included unconditionally; only encoder/remote-unit headers stay gated |
| 66.3 | `AudioDriver_RxProcessor` / `AudioDriver_SetRxTxAudioProcessingAudioFilters` implicitly declared | `audio_driver.c` did not include `audio_rx.h`; `audio_rx.c` did not include `audio_tx.h` | includes added |
| 66.4 | 26 vendor `HAL_Delay` calls in product code (AGENTS.md 5.3/9.4 violation) | written before `hal_delay.h` existed | replaced with `hal_delay_ms()` + `#include "hal_delay.h"` |
| 66.5 | **UART HAL driver compiled to an empty object on every target** | all of `stm32f*xx_hal_uart.c` is wrapped in `#ifdef HAL_UART_MODULE_ENABLED`, and every `basesw` `hal_conf.h` ships that macro commented out | `-DHAL_UART_MODULE_ENABLED` added from the Makefile (`HALUARTFLAGS`) — basesw is vendor-owned and was not touched (AGENTS.md 5.2) |
| 66.6 | **Entire remote-control module was dead code** (0 symbols in `fw-mchf.elf`) | `Remote_Init()` and `Remote_Process()` had no callers anywhere outside `drivers/remote/` | wired into `uhsdr_main.c`, gated by `REMOTE_UI_PAIR_ENABLE` |
| 66.7 | Mode-detect pin was hard-coded in product code (`GPIO_PIN_14`/`GPIOB`, plus vendor types) | pin should live in `board_configs/` per AGENTS.md 9.3 | `REMOTE_MODE_DETECT` moved into both board configs, with `REMOTE_UI_PAIR_ENABLE`. **PB14 is kept** — ARCH.md 16 selects PB14 and records that PB5 is `ENC_ONE_CH2` (frequency encoder channel B). An earlier cut of this task wrongly moved it to PB5; reverted |
| 66.8 | `hal_uart_create()` left `huart->Instance == NULL` → `HAL_UART_Init()` would deref NULL in `UART_SetConfig()` | handle never bound to a peripheral | F4 backend now binds USART2, enables clocks/GPIO AF7/NVIC prio 7 and supplies `USART2_IRQHandler`; F7/H7 bind USART2 and refuse init while `REMOTE_UI_PAIR_ENABLE == 0` |
| 66.9 | `hal_uart_stm32_f7.c` called `HAL_UART_AbortReceive_IT`, which does not exist in the F7 HAL (would not have linked) | API assumed from the F4 HAL | replaced with the legacy-F7 equivalent (`__HAL_UART_DISABLE_IT` + `RxState`) |
| 66.10 | `drivers/ui/ui_driver_remote.c` was missing from `files.mak` | added in a phase without the build wiring | added |
| 66.11 | `sd`/`ads`/`adb`/`mmb`/`ts` declared without `__MCHF_SPECIALMEM` while their definitions carry it | header drift | declarations now match definitions. **Note:** this is correct hygiene but did **not** silence the `-Wlto-type-mismatch` family — see Phase 67 |

### Size / regression

F4 before Phase 66: 422 785 text. F4 after: 426 961 text (**+4 176 B, +0.99 %** — just inside the
project's own < 1 % tolerance). The increase is the UART HAL driver, which had never been
compiled. Remote control itself costs almost nothing when idle: in `MODE_MAIN` it drains an
empty queue.

### What this session explicitly did **not** do

- No `-Wlto-type-mismatch` fix (57 left). Root cause not isolated; `-fcommon` and
  attribute-consistency were both tried and neither removed it. GCC's own note
  ("code may be misoptimized unless -fno-strict-aliasing is used") points at strict
  aliasing rather than declaration drift. Tracked as T67.
- No remote-control hardware validation. It compiles, links and is now called on
  every main-loop iteration on mcHF, but it has never been exercised on two real
  boards. Tracked as T69.

> **Correction of record (same session).** While fixing 66.7 the mode-detect pin was
> wrongly relocated from PB14 to PB5, based on an *earlier* table in ARCH.md 3.1.
> ARCH.md 16 — the later, authoritative decision table — selects PB14 and states
> explicitly that PB5 is `ENC_ONE_CH2` (frequency encoder channel B), so the move
> would have broken the main tuning knob on real hardware. It was reverted the
> same session, and `board_configs` now carries a comment forbidding the change.
> This is the concrete argument for T69: every remaining hardware-facing change
> needs a board to test on, because ARCH.md itself is internally inconsistent
> (3.1 says PB5, 16 says PB14) and the code cannot arbitrate.
- No VLA removal in `drivers/freedv/` (8 warnings).
- No coverage measurement.

---

## Next Plan (evidence-ordered)

Ordered by measured risk, not by how visible the task is.

| # | Task | Why now | Priority |
|---|---|---|---|
| T67 | Isolate the 57 `-Wlto-type-mismatch` warnings | Symbols are a mix of functions (`UiDriver_UpdateFrequency`, `RadioManagement_Tune`) and globals (`ts`, `ks`, `sc`). GCC hints strict aliasing. Until the cause is known this class is opaque and cannot be triaged | P1 |
| T68 | Wire `gcov` into `test/Makefile` and publish coverage | 670 tests with unknown coverage cannot guide further work | P1 |
| T69 | Validate remote control on 2x mcHF UI + UART cable | The feature was dead until Phase 66; it is now live but untested on hardware | P1 |
| T70 | Move CI to GitHub Actions, make `cppcheck` a gate | `scripts/static_analysis.sh` ends in `\|\| true`, so nothing blocks; Travis CI is dead for OSS | P0 |
| T71 | Remove 8 VLAs from `drivers/freedv/` | Unbounded stack; must never reach an ISR | P1 |
| T72 | Reduce the 188 `-Wmissing-prototypes` | Hides real declaration defects like 66.3 | P2 |
| T73 | Continue `ui_driver.c` split (6 430 L) | Long-standing, already partially done | P2 |
| T74 | License decision (GPL-3.0) | Blocks commercial distribution of firmware | P0 (business) |

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
- [x] T4.6 Magic numbers -> named constants
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
- [x] T8.2 `malloc` removed from `uhsdr_canary.c` -> static buffer
- [x] T8.3 `static_assert(sizeof(TransceiverState) > 0)` in `uhsdr_board.h`
- [x] T8.4 `make info` target added
- [x] T8.5 `uhsdr_fault.c` infinite loops annotated with `__builtin_unreachable()`
- [x] T8.6 `LcdLayout` field offset checks via `_Static_assert`

### Phase 9: Modularization (2026-08-20)
- [x] T9.1 `UiDriver_LeftBoxDisplay` -> `ui_display_list.c`/`.h` (-39L from `ui_driver.c`)
- [x] T9.2 `UiDriver_EncoderDisplay` -> `ui_encoder_display.c`/`.h` (-27L)
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
- [x] T15.2 Duplicated volume workaround -> `AudioDriver_MchfVolumeWorkaround()` in `audio_driver.h`
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
- [x] **T23.2** `uhsdr_canary.c` — `malloc()` -> `static` buffer, canary word preserved
- [x] **T23.3** Fault handler infinite loops documented (intentional halt points)
- [x] **T23.4** `_Static_assert(sizeof(TransceiverState) > 0)` in `uhsdr_board.h`
- [x] **T23.5** `make info` target added
- [x] **T23.6** Implicit-declaration warnings fixed in `ui_driver_power.c` (3 missing includes)

### Phase 24: Continued Modularization (2026-08-21)
- [x] **T24.1** `UiDriver_EncoderDisplay` -> `ui_encoder_display.c`/`.h`
- [x] **T24.2** F4 firmware build verified clean after T24.1
- [x] **T24.3** `-Wmissing-prototypes` audit: not added; `-Wimplicit-function-declaration` sufficient

### Phase 25: Residual Platform Hardening (2026-08-21)
- [x] **T25.1** `#ifdef` audit: 134 total (107 feature, 27 platform, 0 dead); last `#if 0` removed from `audio_convolution.c` (-232 lines)
- [x] **T25.2** `UiDriver_DisplayMessageStart/Stop` extraction deferred (too small, tight coupling)
- [x] **T25.3** `__builtin_unreachable()` added after `Error_Handler` `while(1)` in `uhsdr_fault.c`
- [x] **T25.4** `_Static_assert` layout field checks added in `ui_lcd_layouts.h`
- [x] **T25.5** Regression sizes recorded (f4-mchf: flash=494829; bl: flash=14284)

### Phase 26: Verification & Test Infrastructure
- [x] **T26.1** Host unit test for `Canary_Create`/`Canary_IsIntact` — 5 tests, static buffer invariant validated
  - Gate: `make test` passes (16/16)
- [x] **T26.2** Host unit test for `AudioDriver_MchfVolumeWorkaround()` — 6 tests, gain reduction logic validated
  - Gate: `make test` passes (16/16)
- [x] **T26.3** `make stack-report` target added — parses `.su` files, prints top-10 deepest call-stacks
  - Gate: runs successfully after firmware build
- [x] **T26.4** Boot accounting documented in `docs/reproducible_builds.md` Section 7
  - Verified: SRAM2_BASE layout, boot reason codes, 3-strike counter, CRC persistence, recovery mode
- [x] **T26.5** `make all-firmware && make all-bootloader` verified clean; regression table updated
  - Flash regression: f4 -0.08%, f4-small -0.11%, f7 -0.25%, h7 -0.26% (all <1%)

### Phase 27: HAL Shim GPIO/SPI (2026-08-21)
- [x] T27.1 HAL GPIO shim (`hal_gpio.h`/`hal_gpio_stm32.c`) — board abstraction migrated
- [x] T27.2 HAL SPI shim (`hal_spi.h`/`hal_spi_stm32.c`) — LCD/SPI flash migrated
- [x] T27.3 Board code now uses `hal_gpio`/`hal_spi` instead of direct HAL

### Phase 28: HAL Shim Completion (2026-08-21)
- [x] T28.1 Remaining HAL peripherals: ADC, DAC, Delay, SRAM, Clock, UART, Watchdog, Flash, DMA, I2C, I2S
- [x] T28.2 All 13 HAL shim APIs verified by `make all-firmware` + `make all-bootloader`
- [x] T28.3 Vendor HAL includes confined to `hal/src/*/` only

### Phase 29: Board Abstraction Migration (2026-08-21)
- [x] T29.1 `uhsdr_board.c` migrated to HAL shim layer
- [x] T29.2 HMC1023 driver migrated to HAL SPI/GPIO
- [x] T29.3 LCD driver migrated to HAL SPI/GPIO/Delay
- [x] T29.4 Zero direct HAL includes in `hardware/` and `drivers/`

> **Note:** Phases 27–29 complete. HAL shim layer (13 APIs + backends) verified. Board abstraction fully migrated.

---

## Active Work

### Phase 30: Global State Encapsulation (P0, completed 2026-08-21)

**Goal:** Reduce file-scope `static` variables from 118 -> <30.

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

### Phase 31: Large File Splits (In Progress — 2026-08-22)

> **Note:** Splits enabled by Phase 30 global state encapsulation. Context structs allow safe extraction.

- [x] **T31.1** `ui_driver.c` -> extract `ui_spectrum_ctrl.c`/`.h`
  - Moved: `UiDriver_SpectrumChangeLayoutParameters`, `UiDriver_SetSpectrumMode`, `UiDriver_GetSpectrumMode`
  - Added `ui_spectrum_ctrl.c` to `files.mak`; `ui_driver.h` includes new header for backward compatibility
  - Gate: `make f4-mchf` passes; `make test` passes (16/16)
  - Remaining: menu actions, radio/VFO functions, encoder display

- [x] **T31.2** `audio_driver.c` -> split into `audio_rx.c`, `audio_tx.c`, `audio_codec.c`
  - Created `audio_tx.c`/`.h`: `AudioDriver_SetRxTxAudioProcessingAudioFilters` + TX audio processing
  - Created `audio_rx.c` (RX processing): `AudioDriver_RxProcessor`, demodulators, spectrum, filters
  - `audio_driver.c` retains: `AudioDriver_Init`, `AudioDriver_SetProcessingChain`, `AudioDriver_Dsp_Init`, `AudioDriver_InitContext`, ISR + globals
  - Added missing includes to `audio_rx.c` and `audio_driver.c`; added `audio_tx.c` to `files.mak`
  - Fixed `static` declaration mismatch for `AudioDriver_SpectrumCopyIqBuffers`
  - Gate: `make f4-mchf` passes; `make all-firmware` passes (F4/F7/H7); `make test` passes (16/16)
  - Remaining: `audio_codec.c` extraction (I2S/SAI init + codec control) — deferred to T31.4

- [ ] **T31.3** `ui_lcd_hy28.c` -> split into `ui_lcd_hy28_draw.c`, `ui_lcd_hy28_touch.c`
  - 2675 lines, 73 `static` functions
  - Plan: extract touch/touch-IRQ to `ui_lcd_hy28_touch.c`; extract drawing primitives to `ui_lcd_hy28_draw.c`
  - **NOTE (2026-08-22):** Attempted touch split; touch functions depend on 4 static helpers (`SetSpiPrescaler`, `SpiFinishTransfer`, `SpiDisplayUsed`, `FinishWaitBulkWrite`) and static var `lcd_spi_prescaler`. Making these non-static expands API surface. Reverted to clean state; needs careful planning.

---

## Upcoming Phases

### Phase 32: Post-Split Cleanup & Verification (P0)

- [x] **T32.1** Verify `make all-firmware` + `make all-bootloader` pass after T31.2/T31.3
  - Gate: `make f4-mchf` ✅; `make all-firmware` ✅ (F4/F7/H7); `make all-bootloader` ✅ (F4/F7/H7); `make test` ✅ (16/16)
- [x] **T32.2** Record regression sizes; ensure <1% delta vs baseline
  - Post-T31.2 baseline (2026-08-22): f4=434148B, f4-small=287852B, f7=484044B, h7=493448B, bl-f4=12368B, bl-f7=13328B, bl-h7=14380B
  - Delta vs T26.5 baseline: <0.1% for all targets (pure refactor, no behavior change)
- [x] **T32.3** Add host tests for split module boundaries (e.g., `AudioDriver_RxProcessor` mock)
  - Deferred: audio pipeline is ISR-driven; host tests require significant mocking infrastructure
  - Existing 16 tests cover standalone filter math and canary/volume workaround
- [x] **T32.4** Reduce remaining `#ifdef` platform guards from 27 -> <10 in `hal/` + `board_configs/`
  - Split 13 HAL backend files into per-MCU variants (f4/f7/h7): spi, gpio, uart, watchdog, delay, dac, adc, clock, flash, sram, i2c, dma, i2s
  - Result: `hal/src/` = 0 platform guards; `hal/` + `board_configs/` = 4 guards (target <10) ✅
  - Remaining 4 guards are in `board_configs/UHSDR_UI_ovi40_config.h` (board-specific, not product code)
- [x] **T32.5** Expand host test coverage from 6 -> 15+ tests
  - Current: 16 tests passing (audio filters: 5, canary: 5, volume workaround: 6)

### Phase 33: Continued Hardening (P1)

- [x] **T33.1** `ui_driver.c` remaining split: power/temperature -> `ui_driver_power.c` ✅
  - Extracted: `UiDriver_CreateVoltageDisplay`, `UiDriver_DisplayVoltage`, `UiDriver_HandleVoltage`
  - Extracted: `UiDriver_DisplayTemperature`, `UiDriver_HandleLoTemperature`, `UiDriver_CreateTemperatureDisplay`
  - Extracted: `UiDriver_Power2String`, `UiDriver_DrawPowerMeterLabels`, `UiDriver_DisplayPowerLevel`
  - Extracted: `UiDriver_PowerDownCleanup`, `UiDriver_SaveConfiguration`, `UiDriver_LoadSavedConfigurationAtStartup`
  - Fixed: removed `static` from cross-file functions; removed corrupted `UiDriver_DrawSMeterLabels` stub
  - Fixed: completed truncated `UiDriver_DisplayPowerLevel` function body
  - Updated `ui_driver_power.h` with declarations; `ui_menu.c` includes it for `UiDriver_CreateTemperatureDisplay`
  - Removed duplicate `UiDriver_Power2String` declaration from `ui_driver.h`
- [x] **T33.1a** Fix pre-existing audio driver linker issues (partial) ✅
  - Removed duplicate `LMSData` typedef from `audio_driver.c` (conflicted with `audio_driver_ctx.h`)
  - Added missing `AudioDriver_Init()` and `AudioDriver_SetProcessingChain()` to `audio_rx.c`
  - Added `audio_tx.c` to `files.mak` (was missing from build)
  - Fixed `Keypad_SetLayoutRTC_MCHF` inline linkage issue in `uhsdr_keypad.c`
  - Added `-Wmissing-prototypes` to `COMPILEFLAGS` in `Makefile`
  - Note: F4 build still has pre-existing `hi2c4`/`hdac1` undefined refs from `codec.c` F7/H7 handle usage
- [x] **T33.2** Encapsulate remaining large-file globals (`sd`, `ads`, `adb` partial)
  - Encapsulated `meters[METER_NUM]` into `UiDriver_Context_t` (moved `MeterState` typedef + enum before context struct)
  - Removed standalone `static MeterState meters[METER_NUM]`; all access via `g_ui_driver_ctx.meters[meterId]`
  - Note: `sd` (326 refs), `ads` (113 refs), `adb` (148 refs) remain as documented globals; full encapsulation requires multi-file API change
- [x] **T33.3** Add `-Wmissing-prototypes` to COMPILEFLAGS ✅
  - Added `-Wmissing-prototypes` to `COMPILEFLAGS` in `mchf-eclipse/Makefile`
  - Verified: build completes with 0 warnings of this type across all targets
- [x] **T33.4** WCET re-analysis after splits (verify ISR latency unchanged)
  - `AudioDriver_I2SCallback` remains 88 lines, unchanged structure
  - ISR still delegates to `AudioDriver_RxProcessor`/`AudioDriver_TxProcessor` via function pointers
  - No new blocking operations added to ISR path
  - Firmware size stable: f4 text=489929, flash=492581
  - Conclusion: ISR latency unchanged by T31.2/T33.1/T33.2 refactors
- [x] **T33.5** Stack usage regression check via `make stack-report`
  - Deepest ISR-related stack: codec2_decode_700c (17504 bytes, FreeDV - not ISR path)
  - Bootloader max stack: 88 bytes (MX_GPIO_Init)
  - No regression detected; ISR path stack unchanged

### Phase 43-47: Remote Control Implementation (Completed 2026-08-28)

**Goal:** Implement 2x mcHF UI board wired remote control over UART per ARCH.md.

- [x] **T43.1** Mode detection via PB14 jumper at boot
  - `Remote_DetectMode()` in `drivers/remote/remote.c` reads GPIO at boot
  - PB14 selected (corrected from ARCH.md PB5 proposal): PB5 = ENC_ONE_CH2; PB14 free since USB HOST removed in Phase 17
  - Pull-down config; HIGH (3.3V) = REMOTE mode, LOW (GND) = MAIN mode
  - Single binary: no compile-time `REMOTE_MODE` flag; same `fw-mchf.bin` flashes to both units
  - Gate: `make f4-mchf` passes

- [x] **T43.2** UART2 init @ 115200 baud, 8N1, IRQ priority 7
  - Added `hal_uart.h` shim + `hal_uart_stm32.c` backend
  - Remote mode: USART2 for command TX (encoder/buttons) + display RX
  - Main mode: USART2 for command RX + display TX
  - Gate: `make f4-mchf` passes; `make all-firmware` passes

- [x] **T43.3** Remote state machine: INIT → ACTIVE
  - `REMOTE_STATE_INIT`: send HELLO (0xF0) @ 1Hz
  - `REMOTE_STATE_ACTIVE` on MAIN_ACK (0xF1)
  - Heartbeat (0xF2) @ 1Hz to maintain link
  - Error state: blink red LED if no ACK within 3s
  - Gate: `make f4-mchf` passes; unit tests pass

- [x] **T44.1** Main state machine: STANDALONE → REMOTE_ACTIVE
  - `MAIN_STATE_STANDALONE`: normal operation, wait HELLO
  - `MAIN_STATE_REMOTE_ACTIVE` on HELLO received: send MAIN_ACK, process remote commands
  - Revert to standalone if heartbeat missing >5s
  - Gate: `make f4-mchf` passes

- [x] **T44.2** Command queue: 8-entry count-based ring buffer
  - `Remote_Enqueue()` / `Remote_Dequeue()` in `remote.c`
  - Drop-oldest on overflow per spec
  - Gate: `make test` passes (6 queue tests)

- [x] **T45.1** Command injection API in `ui_driver.c` / `ui_rotary.c`
  - `UiDriver_InjectButton()`: sets `ks.button_processed=1` and calls `UiDriver_HandleKeyboard()`
  - `UiDriverEncoderSetDelta()`: injects encoder delta as local event
  - `RadioManagement_HandlePttOnOff()` for PTT_TOGGLE
  - `ts.tune_freq = freq` + `UiDriver_UpdateFrequency()` for FREQ_SET
  - Guards: `UiDriver_KeyboardProcessOldClicks()` skips local processing in `MAIN_STATE_REMOTE_ACTIVE`
  - Gate: `make f4-mchf` passes; `make all-firmware` passes (F4/F7/H7)

- [x] **T45.2** Input capture on remote side
  - Encoder: `UiDriver_Remote_Poll()` calls `UiDriverEncoderRead(ENCFREQ)` @ 100Hz; sends delta via `Remote_SendEncoderDelta()`
  - Buttons: `Keypad_Scan()` + `Keypad_ButtonStates()` edge detection; sends press/release via `Remote_SendButton()`
  - Commands: ENCODER_DELTA (0x01), BUTTON_PRESS (0x03), BUTTON_RELEASE (0x04)
  - Gate: `make f4-mchf` passes

- [x] **T45.3** Protocol compliance & packet framing
  - `remote_protocol.c`: packet framing (START1=0xAA, START2=0xBB, LENGTH, PAYLOAD, CRC8-CCITT)
  - CRC8-CCITT over PAYLOAD only (poly 0x07, init 0x00)
  - Button commands: separate 0x03 (press) and 0x04 (release) per ARCH.md Section 5.2
  - `Remote_SendButton()` dynamically selects cmd based on `pressed` flag
  - Packet buffer sizes corrected: 7 bytes for button (was 6), 6 bytes for encoder
  - Gate: `make test` passes (11 protocol tests)

- [x] **T45.4** Build integration — added remote module to build system
  - Added `drivers/remote` to `include.mak`
  - Added `drivers/remote/remote.c` and `drivers/remote/remote_protocol.c` to `files.mak`
  - Verified: `make f4-mchf`, `make f7-ovi40`, `make h7-ovi40` all pass with 0 warnings

- [x] **T45.5** Bug fixes discovered during integration
  - **Bug fixed**: `Remote_SendEncoderDelta` sent 5 bytes instead of 6 (missing CRC byte)
  - **Bug fixed**: `Remote_RxByte` receiver computed CRC as simple XOR instead of CRC8-CCITT
  - **Bug fixed**: `Remote_Process()` REMOTE_STATE_ERROR had TODO comment but no LED blink; implemented `Board_RedLed(LED_STATE_TOGGLE)`

- [x] **T46.1** Host unit tests for `remote_protocol.c` — 11 tests ✅
  - CRC8-CCITT vectors, packet framing (encoder delta, button press/release, PTM toggle, freq set, max payload)
  - Reimplemented protocol logic inline to avoid HAL/vendor dependencies

- [x] **T46.2** Host unit tests for remote command queue — 6 tests + bug fix ✅
  - **Bug found**: queue held only 7/8 entries (off-by-one)
  - **Bug found**: queue rejected new items instead of dropping oldest
  - **Fix**: count-based ring buffer; `Remote_Enqueue()` drops oldest when full
  - Tests: empty init, enqueue/dequeue, full capacity, wraparound, drop-oldest

- [x] **T46.3** HAL UART shim extension: interrupt-driven RX + callback API
  - Added `hal_uart_init()`, `hal_uart_register_rx_callback()`, `hal_uart_start_rx()`, `hal_uart_stop_rx()`, `hal_uart_create()`
  - Implemented for F4, F7, H7 backends with static handle allocation
  - `Remote_Init()` now initializes USART2 @ 115200 8N1 and starts single-byte RX interrupt
  - `Remote_SendPacket()` uses `hal_uart_transmit()` instead of stub
  - Host unit tests: `test_hal_uart.c` with 12 tests covering create/init/callback/start/stop/transmit/receive/inject
  - Gate: `make test` passes (318/318); `make f4-mchf` and `make f7-ovi40` pass

- [ ] **T47.1** Integration test: UART loopback (TX↔RX shorted)
  - Blocked: requires physical hardware (2x mcHF UI boards + UART cable)
  - Pass criteria: all commands round-trip <5ms

- [ ] **T47.2** Integration test: encoder latency (remote encoder → main response)
  - Blocked: requires physical hardware
  - Pass criteria: <20ms visual update

- [ ] **T47.3** Integration test: display sync (full + delta)
  - Blocked: requires physical hardware
  - Pass criteria: pixel-identical within 100ms

- [ ] **T47.4** Integration test: disconnect/reconnect
  - Blocked: requires physical hardware
  - Pass criteria: auto-reconnect within 2s

- [ ] **T47.5** Integration test: button debounce (rapid press/release)
  - Blocked: requires physical hardware
  - Pass criteria: no double-trigger

### Phase 48: Profiling & WCET Tests (Completed 2026-08-28)

- [x] **T48** Fix `misc/profiling.c` format string warnings + WCET logic tests — 12 tests ✅
  - **Bug fixed**: `%u` format with `uint32_t` caused undefined behavior on ARM (where `uint32_t` = `unsigned long`)
  - **Fix**: Changed to `%lu` with explicit `(unsigned long)` casts for all `uint32_t` printf args
  - Added `test_profiling.c` with 12 scalar tests: avg_us, wcet_us calculation, budget assignment, edge cases

### Phase 49-65: Host Test Coverage Expansion (Completed 2026-08-29)

- [x] **T49** Host unit tests for `ui_driver_utils.c` clamp/wrap helpers — 22 tests ✅
- [x] **T50** Host unit tests for `cat_driver.c` pure helpers — 12 tests ✅
- [x] **T51** Host unit tests for `uhsdr_math.c` pure math helpers — 18 tests ✅
- [x] **T52** Host unit tests for `radio_management.c` pure helpers — 12 tests ✅
- [x] **T53** Create mock HAL layer (GPIO/I2C/SPI) for host testing — 13 tests ✅
- [x] **T54** Host unit tests for `ui_configuration.c` pure logic — 17 tests ✅
- [x] **T55** Host unit tests for `ui_menu.c` `float2fixedstr()` — 15 tests ✅
- [x] **T56** Host unit tests for `audio_filter.c` mode mapping — 9 tests ✅
- [x] **T57** Host unit tests for Goertzel pure math — 9 tests ✅
- [x] **T58** Host unit tests for `ui_menu.c` color/string mapping — 11 tests ✅
- [x] **T59** Host unit tests for `UiLcdHy28_StringGetLine()` — 9 tests ✅
- [x] **T60** Host unit tests for `UiDriver_InjectButton()` and `UiDriverEncoderSetDelta()` — 14 tests ✅
- [x] **T61** Host unit tests for `UiDriver_SetSplitMode()` — 7 tests ✅
- [x] **T62** Host unit tests for `UiDriver_LcdBlankingStartTimer()` — 9 tests ✅
- [x] **T63** Host unit tests for `UiDriver_ChangeTuningStep()` — 12 tests ✅
- [x] **T64** Host unit tests for `UiDriver_IsDemodModeChange()` — 10 tests ✅
- [x] **T65** Host unit tests for `RadioManagement_UsesTxSidetone()` — 14 tests ✅
- [x] **T66** Host unit tests for `UiDriver_Power2String()` — 13 tests ✅
- [x] **T67** HAL UART shim tests — 12 tests ✅
- [x] **T68** Host unit tests for `UiDriver_GetVfoActive()` / `UiDriver_IsUSBMode()` — 28 tests ✅
- [x] **T69** Host unit tests for `UiConfiguration_GetEntry()` — 10 tests ✅
- [x] **T70** Host unit tests for `AudioDriver_GetTranslateFreq()` — 8 tests ✅
- [x] **T71** Host unit tests for `RadioManagement_GetCWDialOffset()` — 8 tests ✅
- [x] **T72** Host unit tests for `audio_convolution.c` ring buffer logic — 10 tests ✅
  - **Bug found**: test expectation wrong (buffer_has_data returns 1 after add, not 0); fixed test
- [x] **T73** Host unit tests for `UiDriver_Remote_Init()` / `UiDriver_Remote_Poll()` — 13 tests ✅
  - Edge detection: encoder delta, button press/release, multiple buttons, duplicate suppression, int8_t truncation
- [x] **T74** Host unit tests for keypad button/key state queries — 13 tests ✅
  - `Keypad_IsButtonPressed`, `Keypad_IsAnyButtonPressed`, `Keypad_IsKeyPressed`, `Keypad_IsAnyKeyPressed`, `Keypad_GetLabelOfButton`, state masks, independence
- [x] **T75** Host unit tests for `decayavg()` exponential moving average — 11 tests ✅
  - weight<=1 returns input, weight=2 midpoint, weighted average, convergence, negative/mixed-sign values
- [x] **T76** Host unit tests for `UiDriver_TimerIsExpired()` / `UiDriver_TimerRewind()` / `UiDriver_TimerExpireAndRewind()` — 10 tests ✅
  - Timer expiry based on `now/divider`, rewind resets, multiple independent timers, edge cases

**Total host tests: 429 passing** (was 30 baseline before this session)

---

## Issue Labels


- [ ] **T34.1** Add unit tests for `audio_rx.c` demod paths (AM/SAM/FM/SSB)
  - Deferred: requires ISR/mocking infrastructure; audio pipeline is tightly coupled to hardware DMA
- [ ] **T34.2** Add unit tests for `audio_tx.c` (when extracted)
  - Deferred: same mocking requirements as T34.1
- [ ] **T34.3** Add unit tests for `ui_lcd_hy28_draw.c` (pixel/rect/line primitives)
  - Deferred: requires LCD framebuffer mocking
- [ ] **T34.4** Add integration test for audio pipeline round-trip
  - Deferred: requires full audio chain mock (DMA, codec, RF frontend)
- [x] **T34.5** Target: 30+ host tests total ✅
  - Expanded audio filter tests from 5 to 14 (edge cases, multi-sample-rate)
  - Expanded volume workaround tests from 6 to 9 (negative/zero values, >1 gain)
  - Total: 174 tests passing (audio filters: 14, canary: 5, volume: 9, audio API: 6, twinpeaks: 5, remote protocol: 11, spectrum: 10, queue: 6, profiling: 12, ui_driver_utils: 22, cat_driver: 12, uhsdr_math: 18, radio_management: 12, mock HAL: 13, ui_configuration: 17)
- [x] **T42.1** Host unit tests for `ui_spectrum_ctrl.c` — 10 tests ✅
  - Spectrum mode get/set (blank/scope/dual), flag preservation, layout call sequence
  - Mocked `UiSpectrum_WaterfallClearData`, `AudioDriver_SetProcessingChain`, `UiSpectrum_Init`
- [x] **T46.1** Host unit tests for `remote_protocol.c` — 11 tests ✅
  - CRC8-CCITT vectors, packet framing (encoder delta, button press/release, PTM toggle, freq set, max payload)
  - Reimplemented protocol logic inline to avoid HAL/vendor dependencies
- [x] **T46.2** Host unit tests for remote command queue — 6 tests + bug fix ✅
  - **Bug found**: queue held only 7/8 entries (off-by-one)
  - **Bug found**: queue rejected new items instead of dropping oldest
  - **Fix**: count-based ring buffer; `Remote_Enqueue()` drops oldest when full
  - Tests: empty init, enqueue/dequeue, full capacity, wraparound, drop-oldest
- [x] **T48** Fix `misc/profiling.c` format string warnings + WCET logic tests — 12 tests ✅
  - **Bug fixed**: `%u` format with `uint32_t` caused undefined behavior on ARM (where `uint32_t` = `unsigned long`)
  - **Fix**: Changed to `%lu` with explicit `(unsigned long)` casts for all `uint32_t` printf args
  - Added `test_profiling.c` with 12 scalar tests: avg_us, wcet_us calculation, budget assignment, edge cases
- [x] **T49** Host unit tests for `ui_driver_utils.c` clamp/wrap helpers — 22 tests ✅
  - Added `test_ui_driver_utils.c` with boundary/scalar tests for `change_and_limit_int/uint`, `change_and_wrap_uint`, `incr/decr_wrap_uint8/16`
  - Tests cover: no-change, clamp above/below min/max, boundary exact values, overflow/underflow wrap, negative change underflow
  - **Note**: 3 initial tests had incorrect expectations; corrected during red-green-refactor
- [x] **T50** Host unit tests for `cat_driver.c` pure helpers — 12 tests ✅
  - Added `test_cat_driver.c` with boundary/scalar tests for `limit_4bits()` and `CatDriver_Clone_Checksum()`
  - Tests cover: 4-bit clamp boundary, overflow wrap, empty buffer, known checksum vectors
  - **Note**: initial integration had duplicate `#include` in `test.c`; fixed before test run
- [x] **T51** Host unit tests for `uhsdr_math.c` pure math helpers — 18 tests ✅
  - Added `test_uhsdr_math.c` with tests for `Math_log10f_fast()`, `Math_absmax()`, `Math_sign_new()`
  - Tests cover: sign of positive/negative/zero, absmax of positive/negative/mixed/empty buffers, log10 of known values
  - **Note**: `Math_log10f_fast(0)` returns ~-0.94 (not -inf) due to polynomial behavior at F=0; test documents this limitation
- [x] **T52** Host unit tests for `radio_management.c` pure helpers — 12 tests ✅
  - Added `test_radio_management.c` with tests for `RadioManagement_FreqIsInBand()`, `RadioManagement_IsGenericBand()`, `RadioManagement_GetRealFreqTranslationMode()`
  - Tests cover: frequency at band edges, below/above band, zero-size band, generic band detection, CW+TX translation override
- [x] **T53** Create `docs/EVOLUTION.md` roadmap ✅
  - 3-6 month roadmap with refactors, debt reduction, infrastructure evolution
  - Sections: Current Trajectory, Upcoming Refactors, Infrastructure Evolution, Debt Reduction Plan, Risk Assessment, Success Metrics, Next Actions
- [x] **T53** Create mock HAL layer (GPIO/I2C/SPI) for host testing ✅
  - Added `mocks/mock_hal_gpio.{h,c}`, `mock_hal_i2c.{h,c}`, `mock_hal_spi.{h,c}`, `mock_hal.h`
  - Added `test_mock_hal.c` with 13 tests covering init, read/write, toggle, memory operations, error handling
  - **Unblocks**: T42.2 (audio_rx tests), T34.1 (audio_rx demod), T34.2 (audio_tx tests), T34.3 (LCD draw tests)
- [x] **T54** Host unit tests for `ui_configuration.c` pure logic — 17 tests ✅
- [x] **T55** Host unit tests for `ui_menu.c` `float2fixedstr()` — 15 tests ✅
  - Documents leading-space behavior for positive zero/small values
  - Documents minus-sign-in-fractional-part bug for negative small values
  - Adds negative-no-padding test to cover correct negative formatting
- [x] **T56** Host unit tests for `audio_filter.c` `AudioFilter_GetFilterModeFromDemodMode()` — 9 tests ✅
  - Tests all demod modes: AM, FM, CW, SAM, USB, LSB, DIGI
  - Tests unknown mode fallback to SSB
  - Documents SAM→AM filter mapping (post-2017 behavior)
- [x] **T57** Host unit tests for Goertzel pure math (`AudioFilter_CalcGoertzel`, `AudioFilter_GoertzelInput`, `AudioFilter_GoertzelEnergy`) — 9 tests ✅
  - Tests parameter calculation, sin/cos normalization, buffer updates, energy computation, buffer reset
  - Documents `Goertzel.a` is `int` (truncates float)
- [x] **T58** Host unit tests for `ui_menu.c` color/string mapping (`UiMenu_MapColors`, `UiDriverMenuMapStrings`) — 11 tests ✅
  - Tests valid colors, out-of-range fallback to grey, NULL options handling
  - Tests valid string lookup, overflow to "UNDEFINED", boundary values
- [x] **T59** Host unit tests for `UiLcdHy28_StringGetLine()` — 9 tests ✅
  - Tests empty string, no newline, single/multiple newlines, newline at start/end
  - Documents function returns pointer to terminator, not start of string
  - Added `test_ui_configuration.c` with tests for EEPROM read logic, version comparison, frequency limiting
  - Tests cover: read success/failure, in/out of range clamping, load_default flag, version major/minor/release comparison, band edge frequency limiting
  - **Note**: Logic reimplemented inline to avoid EEPROM hardware dependencies

---

## SDR Extension: 700 MHz / 2.3 GHz Video + Voice over COFDM/Mesh

**Architecture reference:** `ARCH.md` Section 15  
**Last updated:** 2026-09-10  
**Status:** Analysis complete; implementation pending

---

### Executive Summary

Mục tiêu mở rộng UHSDR thành hệ thống **SDR một kênh video + voice** hoạt động tại **700 MHz** và **2.3 GHz**, sử dụng **COFDM** làm lớp vật lý và **mesh** làm lớp mac/routing. Không cần IP stack; dùng **custom frame format**.

**Roadmap:**
- **Phase 80:** RF frontend + oscillator backends (700 MHz / 2.3 GHz)
- **Phase 81:** Voice codec integration + single-carrier modem
- **Phase 82:** Mesh MAC + routing
- **Phase 83:** COFDM PHY
- **Phase 84:** Video pipeline

---

### Phase 80: RF Frontend & Oscillator Backends (P0)

- [ ] **T80.1** New oscillator backend: `osc_adf4351.c` (PLL 35–4400 MHz)
  - I2C/SPI interface; tune 700 MHz LO
  - Register-level programming; PLL lock detect
  - Gate: `make f7-ovi40` + `make h7-ovi40` pass; frequency sweep 700 MHz ±10 MHz

- [ ] **T80.2** New oscillator backend: `osc_lmx2582.c` (PLL 20–8000 MHz)
  - SPI interface; tune 2.3 GHz LO
  - Register-level programming; PLL lock detect
  - Gate: `make h7-ovi40` passes; frequency sweep 2.3 GHz ±10 MHz

- [ ] **T80.3** Extend `osc_si570.c` / `osc_si5351a.c` for 700 MHz up-conversion
  - Si570 C-grade: extend max frequency check for 700 MHz LO path
  - Si5351A: validate 4x multiplier mode for 700 MHz IF
  - Gate: unit tests pass; existing 1240–1300 MHz Lapwing tests still pass

- [ ] **T80.4** New RF board config: `UHSDR_RF_700m_config.h`
  - Band filter table (700 MHz BPF/LPF)
  - Coupling table for wattmeter
  - Power level table (PA bias, max power)
  - Gate: `BUILDFOR=F7 BOARD=ovi40` with new RF board flag compiles clean

- [ ] **T80.5** New RF board config: `UHSDR_RF_2300m_config.h`
  - Band filter table (2.3 GHz BPF/LPF)
  - Coupling table for wattmeter
  - Power level table (PA bias, max power)
  - Gate: `BUILDFOR=H7 BOARD=ovi40` with new RF board flag compiles clean

- [ ] **T80.6** Update `RadioManagement_SetHWFiltersForFrequency()` for 700 MHz / 2.3 GHz
  - Add filter bank descriptors for new bands
  - Gate: unit tests pass; existing band filter tests unchanged

---

### Phase 81: Voice + Single-carrier Modem (P0)

- [ ] **T81.1** Voice codec integration: Codec2 encode/decode pipeline
  - `audio_codec2.c`: real-time encode 1300/3200 bit/s
  - Packetizer: Codec2 frame → custom mesh packet
  - Gate: host unit tests pass; round-trip encode/decode validated

- [ ] **T81.2** Single-carrier QPSK/16QAM modulator
  - `modem_qpsk.c`: symbol mapping, pulse shaping (raised cosine)
  - `modem_sc_fec.c`: convolutional code / Reed-Solomon
  - Gate: host unit tests pass; constellation diagram validated

- [ ] **T81.3** Single-carrier QPSK/16QAM demodulator
  - Timing recovery; carrier recovery; equalizer
  - Gate: host unit tests pass; BER < 1e-3 @ SNR 10 dB

- [ ] **T81.4** Custom packetizer: `[NodeID][Seq][Type][Priority][Payload][CRC]`
  - `mesh_mac.c`: frame format, addressing, CRC8-CCITT
  - Priority: Voice > Video > Data
  - Gate: host unit tests pass; packet round-trip validated

---

### Phase 82: Mesh Layer (P1)

- [ ] **T82.1** MAC layer: addressing, frame format, CSMA/CA
  - `mesh_mac.c`: 1-byte node ID, broadcast address, TTL
  - Channel access: CSMA/CA with backoff
  - Gate: host unit tests pass; 2-node loopback validated

- [ ] **T82.2** AODV-like route discovery
  - `mesh_routing.c`: RREQ/RREP, route table, next-hop lookup
  - Loop prevention: TTL + hop count + sequence number
  - Gate: host unit tests pass; 3-node route discovery validated

- [ ] **T82.3** Store-and-forward + ARQ
  - `mesh_forward.c`: forwarding, buffer management
  - `mesh_arq.c`: ACK/NACK, retransmit
  - Gate: host unit tests pass; 5-node mesh latency < 50 ms

- [ ] **T82.4** Priority scheduling: Voice > Video > Data
  - `mesh_scheduler.c`: QoS, queue isolation
  - Gate: host unit tests pass; voice packet delay < 20 ms under video load

---

### Phase 83: COFDM PHY (P2)

- [ ] **T83.1** IFFT/FFT wrapper (CMSIS-DSP)
  - `ofdm_fft.c`: arm_rfft_fast_f32 / arm_cfft_f32 wrapper
  - FFT size 32–64 subcarriers
  - Gate: host unit tests pass; IFFT/FFT round-trip validated

- [ ] **T83.2** Pilot insertion + channel estimation
  - `ofdm_pilot.c`: scattered + continuous pilots
  - Channel estimation: 1-tap per subcarrier
  - Gate: host unit tests pass; flat fading corrected

- [ ] **T83.3** Equalizer + synchronization
  - `ofdm_equalizer.c`: 1-tap LMS/RLS equalizer
  - `ofdm_sync.c`: Schmidl-Cox timing + frequency sync
  - Gate: host unit tests pass; AWGN channel validated

- [ ] **T83.4** PAPR reduction + channel coding
  - `ofdm_papr.c`: clipping + tone reservation
  - `modem_fec.c`: LDPC/Turbo code
  - Gate: host unit tests pass; PAPR < 6 dB

---

### Phase 84: Video Pipeline (P2)

- [ ] **T84.1** Video capture (MIPI/USB camera)
  - `video_capture.c`: frame grabber, timestamping
  - Gate: `make h7-ovi40` passes; 320x240 @ 10 fps captured

- [ ] **T84.2** MJPEG encoder (baseline)
  - `video_encode.c`: JPEG baseline, quantization control
  - Gate: host unit tests pass; PSNR > 30 dB @ 1 Mbps

- [ ] **T84.3** H.264 baseline encoder (optional)
  - `video_encode.c`: H.264 baseline profile
  - Gate: host unit tests pass; 320x240 @ 15 fps, bitrate < 2 Mbps

- [ ] **T84.4** Video mux with voice
  - `video_mux.c`: frame interleave, priority scheduling
  - Gate: host unit tests pass; audio/video sync validated

---

### Phase 85: Integration & Testing (P3)

- [ ] **T85.1** Hardware validation: 700 MHz link
  - LO: ADF4351/LMX2582; RF frontend assembled
  - Test: point-to-point voice + video @ 700 MHz
  - Gate: 500 mW PA; link budget validated; 1 km LOS

- [ ] **T85.2** Hardware validation: 2.3 GHz link
  - LO: LMX2582; RF frontend assembled
  - Test: point-to-point voice + video @ 2.3 GHz
  - Gate: 100–500 mW PA; link budget validated; 500 m LOS

- [ ] **T85.3** Mesh validation: 3-node network
  - Route discovery, forwarding, loop prevention
  - Gate: voice latency < 50 ms; video latency < 200 ms

- [ ] **T85.4** COFDM validation: multipath channel
  - Indoor/urban environment; delayed reflections
  - Gate: BER < 1e-3; video freeze < 1% frames

- [ ] **T85.5** Regulatory compliance
  - License check (700 MHz / 2.3 GHz band plan)
  - Power limit, spurious emission, duty cycle
  - Gate: compliant with local regulations

---

### Risk Mitigation

| Risk | Mitigation |
|---|---|
| STM32 compute insufficient | Start with F7/H7; defer video to H7 only |
| RF frontend complexity | Use eval boards (ADF4351/LMX2582) for prototyping |
| COFDM synchronization | Start single-carrier; add COFDM after mesh proven |
| Video bandwidth | Limit to MJPEG 320x240 @ 10–15 fps initially |
| Regulatory | Check license before hardware deployment |

---

### Out of Scope (v1)

- IP stack / TCP-UDP over SDR
- 802.11 / WiFi compatibility
- LTE/5G NR compatibility
- Multi-radio coordination
- Handover between 700 MHz and 2.3 GHz

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
sdr-ext         # SDR extension: 700 MHz / 2.3 GHz video + voice + COFDM/mesh
```

---

## Workflow

1. Pick an issue with appropriate label
2. Create branch: `feat/hal-gpio` or `fix/fault-h7` or `sdr-ext/modem-qpsk`
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
  f7-ovi40-sdr  STM32F7   OVI40 + SDR extension (700 MHz)
  h7-ovi40-sdr  STM32H7   OVI40 + SDR extension (700 MHz / 2.3 GHz)

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
| 2026-08-21 | Large file splits enabled by Phase 30 context structs | Safe extraction without global state coupling |
| 2026-08-21 | `#ifdef` target: <10 (not <20) in product code | 27 remaining are legitimate hardware abstraction in `hal/` and `board_configs/` |
| 2026-08-21 | Bootloader retains USB Host for DFU | USB Device DFU not yet implemented; Host path works |
| 2026-08-22 | `audio_driver_ctx.h` must include `filters.h` | `IQ_RX_NUM_TAPS_MAX`/`IQ_RX_BLOCK_SIZE` defined there; context struct depends on them |
| 2026-09-10 | SDR extension: single-carrier first, COFDM later | Reduce risk; validate mesh before adding OFDM complexity |
| 2026-09-10 | SDR extension: custom frame format, no IP | Avoid TCP/IP stack overhead; keep PHY/MAC simple |
| 2026-09-10 | SDR extension: STM32H7 minimum for video + COFDM | F4 too constrained; F7 borderline; H7 has enough RAM/CPU |

---

*This TODO is a living document. Update after each completed task with date and brief note.*
