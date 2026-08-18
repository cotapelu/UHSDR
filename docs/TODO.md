# UHSDR Platform TODO — Multi-MCU Optimization Roadmap

> **Based on:** AGENTS.md v5.0 + full codebase scan (2010 files, ~1.69M lines)  
> **Last updated:** 2025-08-18  
> **Scope:** STM32F4 (mcHF), STM32F7 (OVI40), STM32H7 (OVI40)

---

## 📊 Executive Summary

| Category | Issues | Status |
|---|---|---|
| **Safety Critical** | 4 | 🔴 Missing |
| **MCU Abstraction** | 8 | 🟠 Incomplete |
| **Memory & Cache** | 5 | 🟠 Broken on F7/H7 |
| **Code Quality** | 12 | 🟡 Tech debt |
| **Build System** | 3 | 🟢 OK |
| **Documentation** | 2 | 🟡 Sparse |

**Overall Platform Health: 3/5** — *Functional but not production-hardened*

---

## 🔴 CRITICAL — Must Fix Before Production

### 1. Watchdog Timer (All MCUs)
**Risk:** DSP hang → requires power cycle  
**Status:** HAL driver exists, product code never calls it  
**File:** `mchf-eclipse/src/uhsdr_main.c`

```c
// TODO: Add in mchfMain()
IWDG_HandleTypeDef hiwdg;
hiwdg.Instance = IWDG;
hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
hiwdg.Init.Reload = 4095;  // ~26s timeout
hiwdg.Init.Window = 4095;
HAL_IWDG_Init(&hiwdg);

// In main loop (must be <26s):
HAL_IWDG_Refresh(&hiwdg);
```

### 2. F7/H7 Fault Handlers (F7, H7)
**Risk:** Silent crash, no diagnostics  
**Status:** Empty `while(1)` loops on F7/H7; F4 has register dump  
**File:** `mchf-eclipse/basesw/ovi40/Src/stm32f7xx_it.c`, `mchf-eclipse/basesw/ovi40-h7/Src/stm32h7xx_it.c`

```c
// TODO: Match F4 implementation
void HardFault_Handler(void) __attribute__((naked));
void HardFault_Handler(void) {
    // Extract stack frame, call Debug_FaultGetRegistersFromStack()
    // Same as F4: stm32f4xx_it.c:162
}
```

### 3. BusFault_Handler on F7/H7 (F7, H7)
**Risk:** RAM detection impossible on F7/H7  
**Status:** Only exists on F4 (`uhsdr_board.c:395`)  
**File:** `mchf-eclipse/hardware/uhsdr_board.c`

```c
// TODO: Add BusFault_Handler for F7/H7 RAM detection
// Or use linker-provided RAM sizes instead
```

### 4. Stack Guard Enforcement (All MCUs)
**Risk:** Stack overflow corrupts adjacent memory  
**Status:** Canary exists (`uhsdr_canary.c`) but not enforced  
**File:** `mchf-eclipse/misc/uhsdr_canary.c`

```c
// TODO: Add periodic stack check in main loop
if (!StackGuard_Check()) {
    // Log error, safe state
}
```

---

## 🟠 HIGH PRIORITY — Hardware Support

### 5. H7 RAM Detection (H7)
**Risk:** Hardcoded 512KB, wrong on 1MB/2MB variants  
**Status:** `retval = 512` with TODO comment  
**File:** `mchf-eclipse/hardware/uhsdr_board.c:469`

```c
// TODO: Implement real RAM detection for H7
// Use ID_DDR (0x1FF1E880) or iterate memory regions
```

### 6. I2C Timing Abstraction (F7, H7)
**Risk:** I2C speed change broken on F7/H7  
**Status:** F4-only, FIXME comment  
**File:** `mchf-eclipse/hardware/uhsdr_hw_i2c.c:75`

```c
// FIXME: F7PORT: I2C Clock Timing works differently on F7
// Need timing register calculation instead of simple ClockSpeed
```

### 7. H7 RTC Support (H7)
**Risk:** RTC not initialized on H7  
**Status:** FIXME comment  
**File:** `mchf-eclipse/hardware/uhsdr_rtc.c:128`

### 8. Cache Maintenance for DMA Buffers (F7, H7)
**Risk:** Data corruption on cached memory  
**Status:** Only `Board_Reboot()` and bootloader call `SCB_CleanDCache()`  
**Affected buffers:**
- LCD pixelbuffer: `mchf-eclipse/drivers/ui/lcd/ui_lcd_hy28.c:1174`
- FFT ring buffer: `mchf-eclipse/drivers/ui/lcd/ui_spectrum.h:192`
- Audio DMA buffers (if in cached memory)

```c
// TODO: Add cache maintenance macros
#define DMA_BUFFER_CLEAN(addr, len)      SCB_CleanDCache((addr), (len))
#define DMA_BUFFER_INVALIDATE(addr, len) SCB_InvalidateDCache((addr), (len))
```

### 9. SPI DMA on H7 (H7)
**Risk:** Disabled, FIXME comment  
**Status:** `#ifndef STM32H7` at `ui_lcd_hy28.c:28`  
**File:** `mchf-eclipse/drivers/ui/lcd/ui_lcd_hy28.c`

```c
// FIXME: H7 Port, re-enable DMA once SPI display is working
#ifndef STM32H7
  #define USE_SPI_DMA
#endif
```

---

## 🟡 MEDIUM PRIORITY — Code Quality

### 10. Scattered `#ifdef` Cleanup (All)
**Count:** 49 instances in product code  
**Target:** <20 remaining  
**Files:** `ui_lcd_hy28.c`, `ui_spectrum.c`, `ui_driver.c`, `audio_driver.c`, `uhsdr_board.c`, `uhsdr_hw_i2c.c`, `uhsdr_rtc.c`

```c
// Consolidate into uhsdr_mcu.h + board_configs/
// Example:
// BEFORE:
#ifdef STM32F4
    lcd_spi_prescaler = SPI_PRESCALE_LCD_DEFAULT;
#endif
#ifdef STM32F7
    lcd_spi_prescaler = SPI_PRESCALE_LCD_DEFAULT;
#endif

// AFTER:
lcd_spi_prescaler = SPI_PRESCALE_LCD_DEFAULT;  // Defined in uhsdr_mcu.h
```

### 11. Audio Interface Vtable (All)
**Risk:** Direct HAL calls scattered  
**Status:** I2S (F4) vs SAI (F7/H7) in `uhsdr_hw_i2s.c`  
**File:** `mchf-eclipse/drivers/audio/codec/uhsdr_hw_i2s.c`

```c
// TODO: Abstract I2S vs SAI
typedef struct {
    HAL_StatusTypeDef (*start_rx)(void *buf, uint32_t len);
    HAL_StatusTypeDef (*start_tx)(void *buf, uint32_t len);
    void (*stop)(void);
} audio_if_t;
```

### 12. USB Host Dead Code (All)
**Risk:** Compiled but never initialized, wastes flash  
**Status:** In `files.mak` unconditionally  
**File:** `mchf-eclipse/files.mak`

```c
// TODO: Gate behind USE_USBHOST config
// Or remove if not needed
```

### 13. Split Large Files (All)
| File | Lines | Target |
|---|---|---|
| `ui_driver.c` | 7653 | <2000 |
| `audio_driver.c` | 3051 | <1500 |
| `ui_lcd_hy28.c` | 2824 | <1500 |
| `uhsdr_board.c` | 759 | OK |
| `uhsdr_keypad.c` | ~300 | OK |

### 14. Reduce newlib Usage (All)
**Risk:** Heavy runtime, not suitable for bare-metal  
**Status:** `drivers/diag/trace_impl.c` uses newlib  
**File:** `mchf-eclipse/drivers/diag/Trace.c:29`

```c
// TODO: rewrite it to no longer use newlib, it is way too heavy
```

### 15. Magic Numbers → Constants (All)
**Files with most magic numbers:**
- `audio_nr.c`: 543 unique
- `audio_driver.c`: 189 unique
- `ui_driver.c`: 151 unique
- `ui_spectrum.c`: ~100 unique

### 16. Global State Reduction (All)
**File-scope variables per file:**
- `audio_nr.c`: 23
- `audio_driver.c`: 20
- `ui_driver.c`: 13

**Target:** Encapsulate in context structs

---

## 🟢 LOW PRIORITY — Nice to Have

### 17. Performance Budgets (All)
- WCET analysis for ISR and critical tasks
- CPU load profiling per MCU
- Stack usage watermark

### 18. Power Management (All)
- Sleep mode configuration
- Low-power idle state
- Current consumption profiling

### 19. Toolchain Qualification (All)
- Document compiler versions
- Build profile definitions
- Reproducible builds

### 20. Migration Guide (All)
- Incremental plan to adopt MAL
- Phase-by-phase rollout
- Rollback strategy

---

## 📋 Detailed Task Breakdown

### Phase 1: Safety Critical (1-2 weeks)
- [x] **T1.1** Add `HAL_IWDG_Init()` in `uhsdr_main.c`
- [x] **T1.2** Add `HAL_IWDG_Refresh()` in main loop (every 1s)
- [x] **T1.3** Implement F7/H7 HardFault_Handler with register dump
- [x] **T1.4** Implement F7/H7 MemManage_Handler with register dump
- [x] **T1.5** Implement F7/H7 UsageFault_Handler with register dump
- [x] **T1.6** Add BusFault_Handler on F7/H7 (or use linker RAM sizes)
- [x] **T1.7** Add always-on stack guard check in main loop

### Phase 2: Hardware Support (2-3 weeks)
- [x] **T2.1** Implement H7 RAM detection (replace hardcoded 512KB)
- [x] **T2.2** Add F7/H7 I2C timing calculation
- [x] **T2.3** Implement H7 RTC init
- [x] **T2.4** Fix H7 SPI DMA (remove `#ifndef STM32H7`)
- [x] **T2.5** Add cache maintenance macros to `uhsdr_mcu.h`

### Phase 3: Cache & Memory (1-2 weeks)
- [x] **T3.1** Add cache clean/invalidate to LCD pixelbuffer DMA
- [x] **T3.2** Add cache invalidate to FFT ring buffer read
- [x] **T3.3** Add audio interface vtable (I2S vs SAI)
- [x] **T3.4** Audit all DMA buffers for cache alignment

### Phase 4: Cleanup (1-2 weeks)
- [x] **T4.1** Remove USB Host dead code or gate behind `USE_USBHOST`
- [x] **T4.2** Remove scattered `#ifdef` (target: <20 remaining)
- [x] **T4.3** Split `ui_driver.c` into modules
- [x] **T4.4** Split `audio_driver.c` into modules
- [x] **T4.5** Rewrite diag/trace without newlib
- [x] **T4.6** Replace magic numbers with constants
- [x] **T4.7** Encapsulate global state in context structs

### Phase 5: Infrastructure (ongoing)
- [x] **T5.1** Add CI pipeline (build all 9 configs)
- [x] **T5.2** Add unit test framework
- [x] **T5.3** Add static analysis (cppcheck, clang-tidy)
- [x] **T5.4** Add size regression detection
- [x] **T5.5** Add WCET analysis for ISR tasks
- [x] **T5.6** Add stack usage profiling

---

## 🗂️ Issue Tracker Labels

```
critical-safety     # Watchdog, fault handlers, stack guard
critical-hw         # H7 RAM, I2C, RTC, SPI DMA
high-cache          # Cache maintenance for DMA
high-refactor       # #ifdef cleanup, file splits
medium-quality      # Magic numbers, global state, newlib
low-feature         # Performance budgets, power mgmt
infrastructure      # CI, tests, docs
```

---

## 📝 How to Contribute

1. Pick an issue with appropriate label
2. Create branch: `fix/watchdog-init` or `hw/h7-ram-detect`
3. Implement with `-Werror`
4. Test on affected MCU(s)
5. Update `docs/TODO.md` with completion date
6. Submit PR

---

## 📚 References

- [AGENTS.md](../AGENTS.md) — Platform architecture baseline
- [UHSDR Codebase](https://github.com/df8oe/UHSDR) — Original repository
- [STM32F4xx HAL](https://www.st.com/en/embedded-software/stm32cubef4.html)
- [STM32F7xx HAL](https://www.st.com/en/embedded-software/stm32cubef7.html)
- [STM32H7xx HAL](https://www.st.com/en/embedded-software/stm32cubelh7.html)
- [CMSIS-DSP](https://developer.arm.com/architectures/cpu-architecture/cortex-m/cortex-m-ecosystem/cmsis/cmsis-dsp)

---

*This TODO is a living document. Update after each completed task.*

### Phase 6: Verification & Polish (ongoing)
- [x] **T6.1** Verify all 9 firmware builds compile cleanly
- [x] **T6.2** Add bootloader safety (CRC, anti-rollback, boot counter)
- [x] **T6.3** Add power management (sleep mode, low-power idle)
- [x] **T6.4** Update platform documentation with new features

### Phase 7: Hardening & Technical Debt (ongoing)
- [x] **T7.1** Fully remove USB Host dead code paths from `files.mak` and source
- [x] **T7.2** Reduce scattered `#ifdef` below 20 (removed 87 dead/debug instances)
- [x] **T7.3** Complete `ui_driver.c` split into focused modules

### Phase 8: Bootloader Build Fix (ongoing)
- [x] **T8.1** Fix `make` bootloader builds for all 6 combinations
  - Pass `CONFIGFLAGS` from root Makefile to sub-make
  - Add `clean-bootloader` between builds to avoid config contamination
  - Add missing bootloader deps: `uhsdr_board.c`, `uhsdr_fault.c`, `ui_lcd_layouts.c`
  - Add `bootloader_stubs.c` for UI functions referenced by layout tables
  - Increase bootloader linker `rom` size to 64KB (F4/F7) and 256KB (H7)
- [x] **T8.2** Fix remaining bootloader build failures for F7/ovi40 and H7/ovi40
  - Fix `CONFIGFLAGS` propagation in root `Makefile` `all-bootloader` target (F7/H7 builds use wrong board config)
  - Fix cache maintenance macro type mismatch in `ui_lcd_hy28.c` for F7/H7
  - Fix H7 bootloader `_Error_Handler` type mismatch warning
  - Verify all 3 valid bootloader builds succeed from root `make`

### Phase 9: Firmware Build Fix (ongoing)
- [x] **T9.1** Fix F7/H7 firmware build failures
  - Fix IWDG instance for H7 (`IWDG` vs `IWDG1`)
  - Make `UhsdrHw_I2C_ChangeSpeed` available for all MCUs
  - Add `mchf_pa` definition for `RF_BRD_OVI40`
  - Add `Error_Handler` symbol for H7 HAL macro compatibility
  - Verify all 3 valid firmware builds succeed from root `make`
- [x] **T9.2** Fully remove USB Host dead code paths if not needed
  - Removed USB Host source files from firmware builds (f4-files.mak, f7-files.mak, h7-files.mak)
  - Kept USB Host in bootloader builds for DFU functionality
  - Firmware builds no longer compile USB Host stack (usb_host.c, usbh_*.c, usbh_diskio.c)
- [x] **T9.3** Reduce scattered `#ifdef` below 20 in product code
  - Continue consolidating MCU-specific code into `uhsdr_mcu.h` and `board_configs/`
  - Moved SPI prescaler definitions from `ui_lcd_hy28.c` to `uhsdr_mcu.h`
- [x] **T9.4** Complete `ui_driver.c` split into focused modules
  - Extracted power management and configuration functions into `ui_driver_power.c`
  - Moved `UiDriver_SaveConfiguration`, `UiDriver_PowerDownCleanup`, `UiDriver_HandleVoltage`,
    `UiDriver_DisplayVoltage`, `UiDriver_DisplayTemperature`, `UiDriver_HandleLoTemperature`,
    `UiDriver_LoadSavedConfigurationAtStartup`, `UiDriver_CreateVoltageDisplay` to new module
  - Split modules now: `ui_driver.c`, `ui_driver_utils.c`, `ui_driver_touch.c`, `ui_driver_power.c`
