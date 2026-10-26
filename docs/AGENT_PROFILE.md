# Agent Weakness Profile — UHSDR Autonomous Improvement

> **Last updated:** 2026-10-26  
> **Agent:** JF Autonomous Agent

---

## Fragile Modules
- **`drivers/remote/remote.c`**: New module (Phase 43-45), limited testing on real hardware. Queue implementation had off-by-one bug (found and fixed in T46.2).
- **`drivers/ui/ui_driver.c`**: Large file (6443 lines), complex state machine (`ks`, `ts`). Remote mode guards added; risk of regression in local input processing.

## Error-Prone Stacks
- **ARM GCC LTO**: Type mismatch warnings between `extern` declarations and definitions (`-Wlto-type-mismatch`). Requires careful struct alignment.
- **Ring buffers**: Off-by-one errors common (e.g., queue count vs head/tail pointer tracking). Always use count-based tracking.
- **Static function guards**: `static` declaration after implicit declaration causes build errors in C11 with `-Wmissing-prototypes`.

## High-Cost Changes
- **`ui_driver.c` modifications**: Any change touching input processing requires extensive testing (keyboard, encoder, touch, remote). High blast radius.
- **Remote UART protocol**: Changes to packet format affect both main and remote units; requires coordinated testing.

## Recurring Debt
- **Vendor HAL warnings**: `basesw/` and `Drivers/` generate `-Wmissing-prototypes` warnings (pre-existing, out of scope).
- **LTO type mismatches**: Multiple `-Wlto-type-mismatch` warnings in product code (pre-existing, requires struct alignment fixes).
- **Format strings with fixed-width types**: `misc/profiling.c` had `%u` with `uint32_t` (undefined behavior on ARM). Pattern: always use `%lu` + cast or `PRIu32` for portable printf.

## Known Weaknesses
1. **Test mocking**: Host tests require significant scaffolding (mocks for HAL, vendor types). Pattern: reimplement logic inline or use stub headers.
2. **Build system**: `make clean` needed when switching MCU targets; stale `.o` files cause linker errors.
3. **Documentation sync**: TODO.md and ARCH.md must be updated together; risk of divergence.
4. **Test expectation accuracy**: When writing tests for existing functions, initial expectations may not match actual semantics (e.g., clamp direction, wrap thresholds, printf padding behavior). Always trace function logic before writing assertions.
5. **printf padding with negative values**: `%Nd` with negative numbers places the minus sign after leading spaces, not before. Functions that split formatted strings at fixed positions can misplace the sign in the fractional part (documented in `float2fixedstr` T55).
6. **Audio filter mode mapping**: `AudioFilter_GetFilterModeFromDemodMode()` is a pure switch mapping but SAM→AM behavior is non-obvious (post-2017 change). Always verify filter mode mappings against current hardware config.
7. **Struct field type mismatches**: `Goertzel.a` is declared as `int` but computed as float, causing silent truncation. Always check struct field types when testing math functions.
8. **Color/string lookup edge cases**: `UiMenu_MapColors` falls back to grey for out-of-range indices; `UiDriverMenuMapStrings` returns "UNDEFINED" for overflow. Always test boundary values (SPEC_MAX_COLOUR, string_max+1).
9. **String scanning functions**: `UiLcdHy28_StringGetLine()` returns pointer to terminator ('\n' or '\0'), not start of string. Always verify pointer arithmetic in string utility functions.

## Build-System Weaknesses (added Phase 68)

10. **Make recipe quoting for macro values with spaces**: `-DTRX_NAME=\"$(TRX_NAME)\"`
    in a make variable expands to a shell word that is split at the first space.
    Any `-D` value containing a space must be single-quoted in the makefile
    (`-D'TRX_NAME="$(TRX_NAME)"'`) so the shell keeps it as one word and the
    double quotes stay part of the macro body. The old default only worked
    because its makefile value happened to be single-quoted.
11. **Identity/config ownership**: board configuration headers own the
    per-board identity (`TRX_NAME`), the build system owns only the artifact and
    DFU file base name (`TRX_ID`). Never force a board-specific value from the
    build system — a command-line assignment in the root Makefile overrides
    everything the sub-make and the headers can decide.
12. **In-tree object names do not encode the configuration**: objects are emitted
    next to their sources, so a configuration switch used the previous MCU's
    objects and silently produced the wrong ELF. The fingerprint stamp
    (`.uhsdr_build_config`) is now the only thing standing between a target
    switch and a wrong release; do not remove it or bypass it with
    `make -o`/`--assume-old`.
13. **`ifdef` vs numeric variables**: `ifdef DEBUG` is true for `DEBUG=0`.
    Always compare numeric switches with `ifeq ($(DEBUG),1)`.

## Fragile Modules (Phase 68 additions)

- **`mchf-eclipse/Makefile`**: now carries the configuration fingerprint, the
  identity override rule and the separate bootloader object tree. A change here
  can invalidate every object or silently publish a wrong binary; always run
  `make build-switch-test` and `make check-identity` after touching it.
- **`mchf-eclipse/hardware/board_configs/UHSDR_UI_*_config.h`**: single source of
  truth for the per-board identity. Editing them changes the user-visible
  product name; the regression script `scripts/check_identity.sh` verifies the
  result.

## Verification Habits (Phase 68)

- Build artifacts are only trustworthy when they were produced by a clean,
  configuration-checked build. `make build-switch-test` (7 steps) and
  `make check-identity` (23 checks) are the two gates that catch
  "built, but not the thing you asked for" defects.
- A `make` target that exits 0 proves nothing about *what* was built. Assert on
  the artifact (ELF architecture attributes, embedded strings), not on the exit
  code.

## Static-Analysis Weaknesses (Phase 68, Cycle 54)

- **cppcheck's type model is the host's, not the target's.** cppcheck maps
  `int32_t`/`uint32_t` to `int`/`unsigned int` (x86-64), while
  `arm-none-eabi` maps them to `long int`/`long unsigned int`. Every
  `invalidPrintfArgType_*` finding on a fixed-width integer is therefore
  suspect. *Rule:* never "fix" such a finding by editing the format specifier
  alone — check what the **target** compiler says
  (`arm-none-eabi-gcc -E -dM -x c /dev/null | grep __INT32_TYPE__`), and make
  the argument type exact with an explicit cast so that both analysers agree.
- **A gcc `-Wformat` warning on the ARM target outranks a cppcheck format
  finding.** The measured cost of "fixing" 39 format findings the cppcheck way
  was **+98 real warnings** (925 → 1023) plus varargs UB.
- **A committed suppression line is a permanent blind spot.** 14 lines in
  `scripts/cppcheck_suppressions.txt` hid 4 genuine defects (a NULL
  dereference, an out-of-bounds read, a `-inf`/NaN power calculation and a
  NULL vector-table read) for months. *Rule:* when the gate is the only thing
  that sees a finding, treat a baseline entry as a defect report, not as
  accepted debt.
- **cppcheck suppression files: a bare `#` line is not a comment.** cppcheck
  2.13 aborts the whole run with `Failed to add suppression. No id.` Every
  comment line needs text after the `#`.
- **cppcheck false positives must be restructured, not suppressed.** The three
  false positives in this cycle (a sentinel array index, a pointer-to-array
  member, a redundant NULL check) were each resolved by making the *code*
  precise — an explicit bound check, pointer arithmetic, an early NULL guard —
  so the analyser can see the invariant.

## Measurement Pitfalls (Phase 68, Cycle 54)

- **Compare like with like.** The release baseline in `docs/TODO.md` §68.6 is
  the ELF **`text`** size from `arm-none-eabi-size`; the `.bin` file size is
  larger because of flash-page padding and the DFU header. Comparing a `.bin`
  size against a `text` baseline manufactured a phantom "+1951 B" regression.
- **Attribute a size delta before accepting it.** Building the pre-change tree
  and diffing the ELF text sizes is the only reliable attribution; the
  pre-T76 tree produced the identical 429 024 B `.bin`, proving the delta was
  0.
