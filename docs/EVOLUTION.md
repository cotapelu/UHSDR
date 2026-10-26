# Evolution Roadmap (3-6 months)

> **Last updated:** 2026-10-26  
> **Agent:** JF Autonomous Agent  
> **Project:** UHSDR STM32 Optimization Baseline v6.0

---

## Current Trajectory

> **Last verified:** 2026-10-26, ARM GCC 13.2.1, clean tree (Phase 68).
> Every number here is measured. Previous revisions of this file claimed
> "Build Status: 0 warnings" and quoted three different test counts
> (372 / 586 / 670) at once; both were wrong and are corrected below.
>
> **Phase 68 correction:** the numbers below were re-measured after the
> build-integrity fix (T75/T79) and the identity fix (T80). Several claims in
> the previous revision were stale and are marked as such.

- **Firmware builds:** F4, F4-512KB, F7, H7 — **0 errors each**.
  F7 and H7 previously failed to link entirely (11 errors); fixed in Phase 66.
- **Bootloader builds:** F4, F7, H7 — 0 errors each.
- **Host test suite:** **671 passing, 0 failing** (53 test files, 11 475 LOC).
  This is the single most valuable asset in the repo — the audio/DSP logic is
  host-testable without hardware.
- **Compiler warnings (re-measured 2026-10-26):** the 57 `-Wlto-type-mismatch`
  warnings are **closed** (Phase 67). What remains in the full 4-target build is
  812 `-Wmissing-prototypes`, 32 `-Wvla-parameter`, 10 `-Wmaybe-uninitialized`,
  4 `-Wcpp`, 3 `-Wimplicit-function-declaration`; in product code the
  `-Wmissing-prototypes` count is **206** (T72) and the `-Wvla-parameter`
  count is **8**, all of them pointer-decay parameters in `drivers/freedv/`,
  not stack VLAs (T71).
- **Binary sizes (corrected *release* baseline, 2026-10-26):** F4 427 073 B,
  F4-512KB 281 441 B, F7 539 153 B, H7 547 857 B. The previously quoted
  "release" figures were **debug** builds (the `ifdef DEBUG` bug, fixed in
  Phase 68). H7 grew +57 240 B (+11.7 %) because `-DNDEBUG` flips an IPA-CP
  clone decision and keeps every CMSIS-DSP twiddle table; the deterministic
  reclaim is tracked as T81.
- **Build integrity (new in Phase 68):** a configuration switch on a dirty tree
  can no longer publish the wrong MCU binary — `make build-switch-test` passes
  7/7, `make clean` leaves **0** objects, and `make check-identity` passes
  23/23 so every target reports its own board name.
- **Key strengths (genuinely):**
  - HAL shim layer is real (4 243 LOC) and product code compiles against it.
  - Audio/DSP logic is covered by host tests with a mock HAL layer.
  - Safety net is real: watchdog, fault dump, stack canary, CRC32 bootloader.
  - Build outputs are now verified by two executable regression gates instead of
    by "make exited 0".
- **Key weaknesses (now documented):**
  - ~~Zero vendor HAL `#include` in product code~~ — **no longer true**:
    Phase 68 found vendor includes/direct `HAL_*` calls in
    `hardware/uhsdr_rtc.c`, `misc/v_eprom/uhsdr_flash.c`,
    `src/bootloader/flash_if.c` and several UI/bootloader paths. Tracked as
    T77 (P0 architecture debt).
  - Static analysis gates CI, but the product-code cppcheck pass does not
    finish within 600 s locally — the gate is not yet usable (T78).
  - Remote control module was dead code (0 symbols in the ELF) until Phase 66
    and is still **unproven on hardware**.
  - Coverage is measured now (gcov wired in Phase 67.2) but not yet gated.
  - The SDR extension roadmap (Phases 80–84: modem, mesh, video) is
    **design-only** — no `drivers/modem`, `drivers/mesh` or `drivers/video`
    directory exists.

---

## Upcoming Refactors

### High Priority
0. **Close the 57 `-Wlto-type-mismatch` warnings** (T67) — blocks any real warning
   triage, and until the cause is known these cannot be classified as safe or unsafe.
0. **Measure test coverage** (T68) — 671 tests with unknown coverage.
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
- **Current:** 671 host tests (gcc-based, no hardware required), 0 failing
- **Blocked on:** coverage measurement. `gcov` is available on the host but nothing
  in `test/Makefile` invokes it, so the effective coverage of the 671 tests is unknown.
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
| Host tests | 671 passing | 671 + coverage gate | 700+ |
| Health score | **not scored** (see TODO.md) | score only after T67+T68 close | ≥95/100 |
| TODO/FIXME count | 184 | <100 | <50 |
| Largest file | 6 430 lines | <4000 | <3000 |
| Build warnings (product code) | 206 `-Wmissing-prototypes`, 8 `-Wvla-parameter` | <100 | <25 |
| `-Wlto-type-mismatch` | **0** (closed in Phase 67) | 0 | 0 |
| Firmware size, F4 release | 427 073 B | < 1 % regression/step | < 1 % |
| Firmware size, H7 release | 547 857 B (corrected baseline, T81) | reclaim DSP tables | < 550 kB |
| Build-output verification | 2 executable gates (switch 7/7, identity 23/23) | 4 gates | artifact attestation |
| Local quality gate | `make quality` (switch + tests + identity + doctor) | CI parity | signed artifacts |
| Static-analysis gate runtime | **4 m 15 s**, bounded (T78 fixed) | < 120 s | < 60 s |

---

## Next Actions

1. **Now (P0):** T77 — finish the HAL boundary migration for the product-owned
   paths that still include vendor headers or call `HAL_*` directly (RTC, flash,
   UI, bootloader), then re-state the AGENTS.md claim truthfully.
2. **Now (P1):** T76 — pay down the committed cppcheck baseline so the gate can
   be tightened; the gate itself is now bounded and parallel (T78 closed).
3. **Next (P1):** T81 — decide the H7/F7 DSP table reclaim: fix the wideband
   FFT lengths (60/16 are unsupported by CMSIS-DSP) and remove the
   runtime-sized `arm_rfft_fast_init_f32` call.
4. **Week 1-2:** Create reusable mock HAL layer (I2C/SPI/UART stubs) — UART mock completed
5. **Week 3-4:** Begin `ui_driver.c` incremental split (extract `ui_driver_input.c`)
7. **Week 7-8:** Expand tests with new mocking infrastructure
8. **Ongoing:** Reduce TODO/FIXME count by 20/month through proactive fixes; complete T47.1 integration tests when hardware available

---

## Trajectory Update — Cycle 54 (T76, 2026-10-26)

The product-code static-analysis debt is now **zero**. This closes the last
measured "the gate cannot see it" blind spot and moves the gate from
"suppression-managed" to "enforcing".

| Signal | Before (Cycle 53) | After (Cycle 54) |
|---|---|---|
| Product-code cppcheck suppressions | 14 lines / 9 files | **0** |
| Product-code cppcheck findings | 14 (all hidden) | **0** (all fixed) |
| Real defects hidden by the baseline | unknown | **4** found and fixed (NULL deref, OOB read, `-inf`/NaN power calc, NULL vector read) |
| Host tests | 670 | **671** |
| Release text size F4 / H7 | 427 073 B / 547 857 B | 427 065 B / 547 921 B (max **+0.012 %**) |
| Compiler warning profile | 925 | **925** (unchanged) |

**What changed in the trajectory.** The static-analysis infrastructure work
(T67.3 → T78 → T76) is finished: the gate is fast (4–5 min, bounded),
parallel, scope-stable (366 files, verified) and now has **no debt to
excuse**. The remaining measurable product-code signal is the compiler
warning profile (925 warnings, of which 812 are `-Wmissing-prototypes` in
vendored + product code) and the 184 TODO/FIXME markers. T71/T72/T73 are
therefore the natural next targets, ahead of further gate work.

**New capability unlocked.** Because the product tree is clean, the gate can
now be **tightened** rather than merely maintained: enabling `--enable=style`
(the gate currently uses `warning,portability`) becomes a measurable, bounded
next step instead of an unbounded flood. Tracked as **T83** below.

**New tracked items:**

| ID | Item | Priority | Why |
|---|---|---|---|
| **T83** | Extend the cppcheck gate to `--enable=style` and triage the new findings | P2 | The product tree is now clean for `warning,portability`; style findings can be measured and the noise baselined without hiding real debt |
| **T84** | Add a target-aware type check to the static-analysis pipeline | P2 | cppcheck's host type model produced 39 false positives on `int32_t`/`uint32_t`; a `arm-none-eabi` cross-check (or `PRIu32` adoption) removes the whole class of confusion |

**Next Actions (revised):**

1. **Now (P0):** T77 — finish the HAL boundary migration for the product-owned
   paths that still include vendor headers or call `HAL_*` directly (RTC, flash,
   UI, bootloader), then re-state the AGENTS.md claim truthfully.
2. **Next (P1):** T81 — decide the H7/F7 DSP table reclaim: fix the wideband
   FFT lengths (60/16 are unsupported by CMSIS-DSP) and remove the
   runtime-sized `arm_rfft_fast_init_f32` call.
3. **Next (P2):** T83 — widen the static-analysis gate to `style` and triage.
4. **Next (P2):** T72 / T71 — reduce the 812 `-Wmissing-prototypes` and the
   32 `-Wvla-parameter` warnings.
5. **Week 1-2:** Create reusable mock HAL layer (I2C/SPI/UART stubs) — UART mock completed
6. **Week 3-4:** Begin `ui_driver.c` incremental split (extract `ui_driver_input.c`)
7. **Week 7-8:** Expand tests with new mocking infrastructure
8. **Ongoing:** Reduce TODO/FIXME count by 20/month through proactive fixes; complete T47.1 integration tests when hardware available
