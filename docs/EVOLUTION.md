# Evolution Roadmap (3-6 months)

> **Last updated:** 2026-08-29  
> **Agent:** JF Autonomous Agent  
> **Project:** UHSDR STM32 Optimization Baseline v6.0

---

## Current Trajectory

> **Last verified:** 2026-10-24, ARM GCC 13.2.1, clean tree.
> Every number here is measured. Previous revisions of this file claimed
> "Build Status: 0 warnings" and quoted three different test counts
> (372 / 586 / 670) at once; both were wrong and are corrected below.

- **Firmware builds:** F4, F4-512KB, F7, H7 — **0 errors each**.
  F7 and H7 previously failed to link entirely (11 errors); fixed in Phase 66.
- **Bootloader builds:** F4, F7, H7 — 0 errors each.
- **Host test suite:** **670 passing, 0 failing** (53 test files, 11 475 LOC).
  This is the single most valuable asset in the repo — the audio/DSP logic is
  host-testable without hardware.
- **Compiler warnings:** 295 total on F4, of which **260 are in product code**.
  Breakdown: 188 `-Wmissing-prototypes`, **57 `-Wlto-type-mismatch` (unresolved)**,
  8 `-Wvla-parameter`, 3 `-Wabsolute-value`, 1 each unused-variable / cpp.
- **Binary regression (F4):** 422 785 -> 426 961 text = **+4 176 B (+0.99 %)**.
  Just inside the < 1 % tolerance. The cost is the UART HAL driver, which had
  never actually been compiled (see Phase 66.5).
- **Key strengths (genuinely):**
  - Zero vendor HAL `#include` in product code — verified, not just documented.
  - HAL shim layer is real (4 243 LOC) and product code compiles against it.
  - Audio/DSP logic is covered by host tests with a mock HAL layer.
  - Safety net is real: watchdog, fault dump, stack canary, CRC32 bootloader.
- **Key weaknesses (now documented):**
  - Remote control module was dead code (0 symbols in the ELF) until Phase 66.
  - The UART HAL driver compiled to an empty object on every target.
  - Coverage is unknown — `gcov` is installed but never invoked.
  - Static analysis does not gate CI (`scripts/static_analysis.sh` ends `|| true`).
  - Only Travis CI; no GitHub Actions workflow exists.

---

## Upcoming Refactors

### High Priority
0. **Close the 57 `-Wlto-type-mismatch` warnings** (T67) — blocks any real warning
   triage, and until the cause is known these cannot be classified as safe or unsafe.
0. **Measure test coverage** (T68) — 670 tests with unknown coverage.
0. **Validate remote control on hardware** (T69) — it was dead code until Phase 66.
1. **`ui_driver.c` large-file split** (T31.3)
   - Current: 6430 lines, single file
   - Target: Split into `ui_driver_input.c`, `ui_driver_menu.c`, `ui_driver_render.c`
   - Blocker: Static helper dependencies between touch/encoder/keyboard paths
   - Estimated effort: 2-3 days

2. **`ui_lcd_hy28.c` split** (T31.3)
   - Current: 2671 lines, 73 static functions
   - Target: `ui_lcd_hy28_draw.c` + `ui_lcd_hy28_touch.c`
   - Blocker: `SetSpiPrescaler`, `SpiFinishTransfer`, `lcd_spi_prescaler` static dependencies
   - Estimated effort: 1-2 days

### Medium Priority
3. **Magic number reduction in `ui_menu.c`**
   - 834 conditional/loop keywords in 4228 lines
   - Replace hardcoded delays, coordinates, and string lengths with named constants
   - Estimated effort: 1 day

4. **`audio_rx.c` demod tests** (T42.2)
   - Requires ISR/mocking infrastructure for DMA/codec/RF frontend
   - Consider standalone demod math extraction for host testing
   - Estimated effort: 3-4 days

### Low Priority
5. **Documentation coverage**
   - Add JSDoc-style comments for all public APIs in `drivers/` and `hardware/`
   - Target: 95% documentation coverage for public functions
   - Estimated effort: 2-3 days

---

## Infrastructure Evolution

### Testing
- **Current:** 670 host tests (gcc-based, no hardware required), 0 failing
- **Blocked on:** coverage measurement. `gcov` is available on the host but nothing
  in `test/Makefile` invokes it, so the effective coverage of the 670 tests is unknown.
  This is now the top test-related task (T68).
- **Plan:**
  - Wire `--coverage` into `test/Makefile` and publish a per-module percentage (T68)
  - Mock HAL for I2C/SPI to unblock demod-path tests (T42.2)
  - Add fuzzing tests for protocol parsers (remote, CAT)
  - Complete T47.1 integration tests when hardware available (2x mcHF UI + UART cable)

### Build System
- **Current:** Make-based, single-threaded
- **Target:** Ninja or CMake with dependency scanning
- **Benefit:** Faster incremental builds, better IDE integration
- **Estimated effort:** 2-3 days migration

### Static Analysis
- **Current:** Compiler warnings only (`-Wall -Wextra -Wmissing-prototypes`)
- **Target:** Add `cppcheck` and `clang-tidy` to CI
- **Benefit:** Catch undefined behavior, dead code, portability issues
- **Estimated effort: 1 day setup

---

## Debt Reduction Plan

### Technical Debt
| Debt Type | Current Count (measured) | Target | Timeline |
|---|---|---|---|
| `TODO`/`FIXME`/`HACK` comments | 184 | <100 | 3 months |
| Functions >300 lines | 2 | 0 | 2 months |
| `#ifdef` / `#if defined` in product code | 542 (was documented as ~822) | <50 feature flags | 6 months |
| Test coverage | **unknown** (gcov never run) | >90% measured | 3 months |
| Compiler warnings in product code | 260 | 0 for `hal/`+`hardware/` first | 3 months |

### Process Debt
1. **Test mocking infrastructure** — Build reusable mock HAL layer to unblock T42.2, T34.1, T34.2
2. **Binary size tracking** — Automate size regression detection in CI
3. **Dependency audit** — FreeDV and CMSIS DSP are unpatched third-party code; evaluate updates

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|
| Large-file refactor regression | Medium | High | Incremental extraction with tests |
| Hardware mocking incomplete | High | Medium | Focus on pure logic first |
| Third-party CVE in FreeDV/CMSIS | Low | High | Subscribe to security advisories |
| Binary size creep | Low | Medium | Track size in CI, enforce <1% limit |

---

## Success Metrics

| Metric | Current (measured) | 3-Month Target | 6-Month Target |
|---|---|---|---|
| Host tests | 670 passing | 670 + coverage gate | 700+ |
| Health score | **not scored** (see TODO.md) | score only after T67+T68 close | ≥95/100 |
| TODO/FIXME count | 184 | <100 | <50 |
| Largest file | 6 430 lines | <4000 | <3000 |
| Build warnings (product code) | 260 | <100 | <25 |
| `-Wlto-type-mismatch` | 57 | root cause closed | 0 |
| Binary regression | +0.99 % | <1% | <1% |

---

## Next Actions

1. **Week 1-2:** Create reusable mock HAL layer (I2C/SPI/UART stubs) — UART mock completed
2. **Week 3-4:** Begin `ui_driver.c` incremental split (extract `ui_driver_input.c`)
3. **Week 5-6:** Add `cppcheck`/`clang-tidy` to CI pipeline
4. **Week 7-8:** Expand tests to 450+ with new mocking infrastructure
5. **Ongoing:** Reduce TODO/FIXME count by 20/month through proactive fixes; complete T47.1 integration tests when hardware available
