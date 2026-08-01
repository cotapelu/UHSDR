# UHSDR - Technical Debt & Improvement Roadmap

> **Generated from static analysis of codebase**  
> Last updated: 2024

---

## 📋 Executive Summary

| Category | Score | Status |
|----------|-------|--------|
| **Filter Bank / Demodulation** | ⭐⭐⭐⭐⭐ | Production Ready |
| **CW Keyer / FreeDV** | ⭐⭐⭐⭐ | Production Ready |
| **AGC / NR / Noise Blanker** | ⭐⭐ | **Needs Rewrite** |
| **Architecture / Modularity** | ⭐⭐ | High Coupling |
| **Testing / CI** | ⭐ | Missing |
| **Documentation** | ⭐⭐ | Sparse |

**Overall: 2.5/5** — *Hobby/Research grade, not Production ready*

---

## 🔴 CRITICAL — Must Fix Before Production

### 1. Firmware Validation in Bootloader
**File:** `mchf-eclipse/src/bootloader/bootloader_main.c`  
**Risk:** Flash corruption → permanent brick  
**Fix:** Add CRC32/signature verification before `JumpToApplication()`

```c
// TODO: Add in uhsdrBl_IsValidApplication()
bool validate_firmware_image(uint32_t addr) {
    // 1. Check vector table sanity
    // 2. Verify CRC32 of firmware image
    // 3. Verify Ed25519 signature (optional)
    // 4. Check version compatibility
}
```

### 2. Watchdog Timer
**File:** `src/uhsdr_main.c`  
**Risk:** DSP hang → requires power cycle  
**Fix:** Enable IWDG, refresh in superloop

```c
// TODO: Add in mchfMain()
HAL_IWDG_Init(&hiwdg);
// In superloop:
HAL_IWDG_Refresh(&hiwdg);
```

### 3. Firmware CRC/Integrity Check
**File:** `mchf-eclipse/src/bootloader/bootloader_main.c`  
**Risk:** Silent flash corruption  
**Fix:** CRC32 at build time, verify at boot

---

## 🟠 HIGH PRIORITY — Core DSP Modules Need Rewrite

### 4. Noise Reduction (`audio_nr.c`) — **877 lines, single function**
**Problems:**
- `spectral_noise_reduction()` = 877 lines (god function)
- Spectral subtraction (1990s) → musical artifacts
- O(N²) complexity, not real-time safe
- No modular structure

**Target:** Modular, <5% CPU on Cortex-M7
- Option A: Wiener filter with decision-directed SNR
- Option B: RNNoise (DNN, 1.5KB model, ~10% CPU on M7)
- Option C: Minimum statistics (Martin Rainer)

**Files:** `audio_nr.c`, `audio_nr.h`

### 5. Noise Blanker (`audio_nr.c`)
**Problems:**
- Simple threshold `|x|² > thresh`
- Cannot distinguish impulse vs signal
- No correlation-based detection

**Target:** Correlation-based impulse detection
- Exploit cyclostationarity of man-made noise
- Correlation over 2-4 symbols

### 6. AGC (`audio_agc.c`) — **WDSP 2005 port**
**Problems:**
- Complex state machine (300+ lines `AudioAgc_SetupAgcWdsp`)
- Magic thresholds everywhere
- Pumping on SSB
- No dual-loop (RF + IF)

**Target:** Modern dual-loop AGC
- RF gain (slow) + IF gain (fast)
- Look-ahead peak detection
- Psychoacoustic loudness (EBU R128)

---

## 🟡 MEDIUM PRIORITY — Optimization & Modernization

### 7. Convolution Engine (`audio_convolution.c`)
- Overlap-add naive → Overlap-save + FFT pruning
- Real-only FFT pruning (50% ops)
- CMSIS-DSP `arm_fir_fast_q31` for short filters

### 8. Auto Notch (`audio_nr.c` — currently disabled)
- Biquad per-bin → unstable
- **Target:** RLS/LMS adaptive lattice (track 8 tones, <2% CPU)

### 9. CW Keyer (`cw_gen.c`)
- Straight key pause too long
- Add interpolation for smoother edges
- Runtime FM CTCSS generation (replace hardcoded table)

### 10. S-Meter Calibration
- Per-band dBm offset in EEPROM
- True dBm/dBm/Hz readout (EBU R128)

---

## 🟢 MEDIUM — Architecture & Code Quality

### 11. God Functions Split
| Function | Lines | Target |
|----------|-------|--------|
| `UiDriver_TaskHandler_MainTasks` | 249 | Split into sub-tasks |
| `UiDriver_CheckEncoderTwo` | 266 | Extract encoder logic |
| `AudioDriver_RxProcessor` | 340 | Split RX chain stages |
| `RadioManagement_SwitchTxRx` | 185 | Extract TX/RX switch logic |

### 12. Magic Numbers → Constants
**Files with most magic numbers:**
- `audio_nr.c`: 543 unique
- `audio_driver.c`: 189 unique
- `ui_driver.c`: 151 unique

**Fix:** Centralize in `audio_constants.h` / `ui_constants.h`

### 13. Global State Reduction
**File-scope variables per file:**
- `audio_nr.c`: 23
- `audio_driver.c`: 20
- `ui_driver.c`: 13

**Fix:** Encapsulate in context structs, pass as parameter

### 14. Error Handling
**Current:** `Error_Handler()` = `while(1)`
**Target:** 
- Error codes + recovery strategies
- Crash dump to EEPROM (PC, LR, stack)
- Graceful degradation (disable NR, not brick)

---

## 🔵 LOW PRIORITY — Modern Features

### 15. Modern Noise Reduction (DNN)
- Integrate RNNoise (1.5KB model, MIT license)
- ~10% CPU on Cortex-M7
- Massive quality improvement

### 16. Parametric EQ
- User-adjustable PEQ (3-5 bands)
- Linear-phase FIR option

### 17. Adaptive Noise Blanker
- Correlation-based impulse detection
- Cyclostationary detection

### 17. Diversity RX
- Phase-coherent dual antenna
- Maximal ratio combining

### 18. Secure Boot
- Ed25519 firmware signature
- Anti-rollback counter

---

## 🏗️ INFRASTRUCTURE

### 19. Build System
- [ ] Enable `-Werror -Wshadow -Wconversion -Wdouble-promotion`
- [ ] Fix all warnings (currently 50+)
- [ ] Add `-fstack-protector-strong`
- [ ] Add `-fstack-clash-protection`

### 20. Static Analysis
- [ ] Cppcheck in CI
- [ ] Clang-tidy (MISRA C subset)
- [ ] Coverity Scan (free for open source)

### 21. Unit Testing
- [ ] Unity test framework
- [ ] Test DSP filters (impulse response, frequency response)
- [ ] Test AGC (step response, attack/decay)
- [ ] Test NR (SNR improvement metric)
- [ ] Test CW keyer (timing accuracy)

### 22. CI/CD Pipeline
- [ ] GitHub Actions: build all 9 configs (F4/F7/H7 × mchf/ovi40)
- [ ] Size regression detection
- [ ] Binary artifact upload
- [ ] Release automation

### 22. Documentation
- [ ] Architecture diagram (Mermaid)
- [ ] DSP algorithm docs (LaTeX → PDF)
- [ ] API reference (Doxygen + Graphviz)
- [ ] Porting guide for new boards

---

## 📦 DEPENDENCIES TO UPDATE

| Library | Current | Target |
|---------|---------|--------|
| CMSIS-DSP | 1.x | Latest 1.13+ |
| CMSIS-Core | 5.x | Latest 5.9+ |
| FreeRTOS | N/A | Optional v11+ |
| TinyUSB | N/A | Replace custom USB stack |
| mbedTLS | N/A | For secure boot |

---

## 📊 METRICS TARGETS

| Metric | Current | Target |
|--------|---------|--------|
| Cyclomatic Complexity (avg) | 15/100 LOC | <10/100 LOC |
| Function length (avg) | 29-89 lines | <50 lines |
| Functions >200 lines | 6 | 0 |
| Magic numbers/file | 150-500 | <20 |
| Global vars/file | 13-23 | <5 |
| Test coverage | 0% | >80% (DSP) |
| Build warnings | 50+ | 0 (`-Werror`) |
| Binary size (F4 fw) | 414 KB | <380 KB |

---

## 🗓️ SUGGESTED ROADMAP

### Phase 1: Safety (1-2 weeks)
- [ ] Firmware CRC in bootloader
- [ ] Watchdog timer
- [ ] Stack guard
- [ ] Config save debounce

### Phase 2: Core DSP Rewrite (6-8 weeks)
- [ ] NR modularization + Wiener/RNNoise
- [ ] Adaptive Noise Blanker
- [ ] AGC redesign (dual-loop)
- [ ] Auto-notch RLS

### Phase 3: Architecture (4-6 weeks)
- [ ] Split god functions
- [ ] Encapsulate global state
- [ ] Error handling framework
- [ ] Magic numbers cleanup

### Phase 4: Infrastructure (2-4 weeks)
- [ ] CI pipeline (9 configs)
- [ ] Unit test framework
- [ ] Static analysis
- [ ] Documentation

### Phase 5: Modern Features (ongoing)
- [ ] RNNoise integration
- [ ] Parametric EQ
- [ ] Secure boot
- [ ] Diversity RX

---

## 🏷️ LABELS FOR ISSUE TRACKER

```
critical-safety     # Firmware validation, watchdog
critical-dsp        # NR, Blanker, AGC rewrite
high-architecture   # God functions, global state
high-optimization   # Convolution, magic numbers
medium-feature      # DNN NR, EQ, Diversity
low-infra           # CI, tests, docs
```

---

## 📝 HOW TO CONTRIBUTE

1. Pick an issue with `good-first-issue` label
2. Create branch: `fix/nr-wiener-filter`
3. Write test first (TDD)
4. Implement with `-Werror`
5. Run all 9 configs in CI
6. Submit PR with benchmarks

---

## 📚 REFERENCES

- [WDSP AGC Paper](http://www.wd5eaq.com/agc/) — Original AGC algorithm
- [RNNoise](https://github.com/xiph/rnnoise) — DNN noise suppression
- [Martin Rainer Minimum Statistics](https://www.researchgate.net/publication/220565280) — Noise estimation
- [EBU R128](https://tech.ebu.ch/docs/tech/tech-r128.pdf) — Loudness standard
- [MISRA C:2012](https://www.misra.org.uk/) — Coding standard

---

*This TODO is a living document. Update after each major refactor.*