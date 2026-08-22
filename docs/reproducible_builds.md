# Reproducible Builds — UHSDR Platform

> **Verified:** 2026-08-21  
> **Toolchain:** `arm-none-eabi-gcc 13.2.1 20231009` (ARM GCC 13.2.Rel1-2)  
> **OS:** Ubuntu 24.04 (Noble Numbat), x86_64  
> **Make:** GNU Make 4.3  
> **Python:** 3.12.3 (for `hex2dfu` in build post-processing)

---

## 1. Quick Start (Clean Repro)

```bash
# 1. Install toolchain
sudo apt install arm-none-eabi-gcc make python3

# 2. Verify environment
make doctor   # must print "PASSED — environment is ready."
make check    # must print "PASSED" (or WARN on uncommitted changes only)

# 3. Clean build
make clean
make f4-mchf  # or any valid target: f4-small, f7-ovi40, h7-ovi40
```

---

## 2. Verified Toolchain

| Component | Version | Source |
|---|---|---|
| `arm-none-eabi-gcc` | 13.2.1 20231009 | `apt install arm-none-eabi-gcc` (Ubuntu 24.04) |
| GNU Make | 4.3 | `apt install make` |
| Python | 3.12.3 | system (for `support/hex2dfu/hex2dfu.py`) |

**Linker:** `arm-none-eabi-ld` (included with GCC toolchain)  
**Binutils:** `arm-none-eabi-objcopy`, `arm-none-eabi-size` (included with GCC toolchain)

---

## 3. Build Invariants (from `mchf-eclipse/Makefile`)

### 3.1 Common Flags

```
-D_GNU_SOURCE -DTRX_ID=\"<id>\" -DTRX_NAME=\"<name>\" \
-DUI_BRD_<BOARD> -DRF_BRD_<BOARD> \
-DNDEBUG -DUSE_HAL_DRIVER -DFDV_ARM_MATH \
-ffunction-sections -fdata-sections -flto \
-Wall -Wuninitialized -Wextra -Wno-unused-parameter \
-Wno-unused-function -Wno-sign-compare -g3 -fstack-usage
```

### 3.2 Per-MCU Machine Flags

| Variable | F4 | F7 | H7 |
|---|---|---|---|
| `-mcpu=` | `cortex-m4` | `cortex-m7` | `cortex-m7` |
| `-mfpu=` | `fpv4-sp-d16` | `fpv5-d16` | `fpv5-d16` |
| `-mfloat-abi=` | `hard` | `hard` | `hard` |
| `-mthumb` | ✅ | ✅ | ✅ |
| `-DARM_MATH_CM4` | ✅ | — | — |
| `-DCORTEX_M4` | ✅ | — | — |
| `-DCORTEX_M7` | — | ✅ | ✅ |
| `-DSTM32F407xx` | ✅ | — | — |
| `-DSTM32F767xx` | — | ✅ | — |
| `-DSTM32H743xx` | — | — | ✅ |

### 3.3 Per-Build Type Optimization

| Target | Flag | `CFLAGS` | `LDFLAGS` |
|---|---|---|---|
| **Firmware** | `-O2` | ✅ | `-flto -g3` |
| **Bootloader** | `-Os` | ✅ | `-flto -g3` |
| **Debug** | `-O0` + `-DDEBUG -DUSE_FULL_ASSERT -DTRACE` | ✅ | `-flto -g3` |

### 3.4 Feature Flags (F7/H7 only)

```
-DFREEDV_MODE_700D_EN=1 -DCODEC2_MODE_700C_EN=1
```

F4 builds do **not** define these (FreeDV 700D/C codec2 modes disabled on F4).

---

## 4. Exact Verified Build Commands

### 4.1 Firmware (4 valid combos)

```bash
# mcHF on F4 (default, 1MB flash)
make BUILDFOR=F4 BOARD=mchf CONFIGFLAGS="-DUI_BRD_MCHF -DRF_BRD_MCHF" firmware
# → mchf-eclipse/fw-mchf.bin  (434396 bytes, 2026-08-21)

# mcHF on F4-512KB (small build)
make BUILDFOR=F4-512KB BOARD=mchf CONFIGFLAGS="-DUI_BRD_MCHF -DRF_BRD_MCHF -DIS_SMALL_BUILD" firmware
# → mchf-eclipse/fw-mchf.bin  (288468 bytes, 2026-08-21)

# OVI40 on F7
make BUILDFOR=F7 BOARD=ovi40 CONFIGFLAGS="-DUI_BRD_OVI40 -DRF_BRD_OVI40" firmware
# → mchf-eclipse/fw-mchf.bin  (485236 bytes, 2026-08-21)

# OVI40 on H7
make BUILDFOR=H7 BOARD=ovi40 CONFIGFLAGS="-DUI_BRD_OVI40 -DRF_BRD_OVI40" firmware
# → mchf-eclipse/fw-mchf.bin  (494704 bytes, 2026-08-21)
```

### 4.2 Bootloader (3 valid combos)

```bash
# mcHF on F4
make BUILDFOR=F4 BOARD=mchf CONFIGFLAGS="-DUI_BRD_MCHF -DRF_BRD_MCHF" bootloader
# → mchf-eclipse/bl-mchf.bin  (12264 bytes, 2026-08-21)

# OVI40 on F7
make BUILDFOR=F7 BOARD=ovi40 CONFIGFLAGS="-DUI_BRD_OVI40 -DRF_BRD_OVI40" bootloader
# → mchf-eclipse/bl-mchf.bin  (13224 bytes, 2026-08-21)

# OVI40 on H7
make BUILDFOR=H7 BOARD=ovi40 CONFIGFLAGS="-DUI_BRD_OVI40 -DRF_BRD_OVI40" bootloader
# → mchf-eclipse/bl-mchf.bin  (14284 bytes, 2026-08-21)
```

### 4.3 Build All (CI / release)

```bash
make all-firmware   # builds 4 firmware combos, cleans between each
make all-bootloader # builds 3 bootloader combos, cleans between each
make size-summary   # prints sizes of last-built fw-mchf.elf and bl-mchf.elf
```

---

## 5. Output Artifacts

| File | Description |
|---|---|
| `mchf-eclipse/fw-mchf.elf` | Firmware ELF (last build wins; used by `size-summary`) |
| `mchf-eclipse/fw-mchf.bin` | Raw binary (copy to root as `fw-mchf_<config>.bin`) |
| `mchf-eclipse/fw-mchf.dfu` | DFU package for USB flashing |
| `mchf-eclipse/fw-mchf.hex` | Intel HEX (intermediate, removed after DFU) |
| `mchf-eclipse/fw-mchf.map` | Linker map |
| `mchf-eclipse/fw-mchf.elf.ltrans*.su` | LTO stack usage (can be ignored) |
| `mchf-eclipse/bl-mchf.elf` | Bootloader ELF (last build wins) |
| `mchf-eclipse/bl-mchf.bin` | Bootloader raw binary |
| `mchf-eclipse/bl-mchf.dfu` | Bootloader DFU package |

**Note:** The inner `mchf-eclipse/Makefile` always writes to `fw-mchf.*` / `bl-mchf.*`.  
The outer root `Makefile` (`all-firmware`, `all-bootloader`) **copies** these to `fw-mchf_<config>.*` / `bl-mchf_<config>.*` after each clean build.

---

## 6. `.gitignore` Rules (as of 2026-08-21)

```
# Build artifacts
*.bin
*.dfu
*.elf
*.hex
*.map
*.su

# LTO intermediate files
*.ltrans*

# Object/build dirs
build/
*.o
*.d

# Agent internals
.pi/
.piclaw/
```

---

## 7. Boot Accounting Flow

The bootloader and firmware use **STM32 backup SRAM (SRAM2)** to maintain boot state across resets without relying on Flash wear-leveling.

### 7.1 Memory Layout (SRAM2_BASE)

| Offset | Content | Type |
|---|---|---|
| `+0x00` | Boot reason code | `uint32_t` |
| `+0x04` | Boot failure counter | `uint32_t` |
| `+0x08` | Firmware CRC32 | `uint32_t` |

### 7.2 Boot Reason Codes

```c
BOOT_CLEARED     = 0x00000000
BOOT_REBOOT      = 0x00000055   // firmware-requested immediate reboot
BOOT_DFU         = 0x00000099   // enter DFU mode
BOOT_FIRMWARE    = 0x66993300   // normal firmware boot
```

These are written by `COMMAND_ResetMCU()` before `NVIC_SystemReset()`.

### 7.3 Boot Failure Counter (3-Strike Recovery)

- **Increment:** `Bootloader_IncrementBootCounter()` adds 1 on flash/CRC failure (capped at 255).
- **Reset:** `Bootloader_ResetBootCounter()` clears counter after successful flash + CRC verification.
- **Check:** Before jumping to firmware, bootloader compares counter against `BOOTLOADER_MAX_BOOT_FAILURES` (3).
- **Recovery:** If counter >= 3, bootloader calls `BootFail_Handler(5)` which blinks the backlight and halts, prompting user to reflash.

### 7.4 Firmware CRC Persistence

After successful flash, the computed CRC32 is stored at `SRAM2_BASE + 8`. This allows the firmware (if desired) to verify its own integrity on next boot without re-reading Flash.

### 7.5 Cache Maintenance

On F7, `COMMAND_ResetMCU()` performs `SCB_CleanDCache()` before `NVIC_SystemReset()` to ensure SRAM2 writes are visible to the next boot stage. H7 does not enable DCache in current main.c, so this is guarded.

### 7.6 Recovery Mode Entry

1. User reflashes firmware via DFU or external programmer.
2. Bootloader verifies CRC32 of new firmware image.
3. On success: `Bootloader_ResetBootCounter()` → counter = 0.
4. On next boot: counter < 3 → normal jump to application.

---

## 8. Reproducibility Checklist

- [x] Toolchain version pinned to `arm-none-eabi-gcc 13.2.1 20231009`
- [x] Build order enforced (`clean` between configs in `all-firmware` / `all-bootloader`)
- [x] `.gitignore` covers all generated artifacts
- [x] `make doctor` verifies toolchain + source file presence before build
- [x] `make check` verifies repo tree + make fragment integrity
- [x] `make size-summary` provides regression baseline
- [x] All 4 firmware + 3 bootloader combos verified clean (2026-08-21)
- [x] Size regression: all configs within <1% variance
- [ ] Containerized build env (`Dockerfile`) — deferred (no urgent need; toolchain available via `apt`)

---

## 9. Known Limitations

1. **Invalid combos fail at compile time** — `f4-ovi40`, `f7-mchf`, `h7-mchf` hit `#error` guards in `UHSDR_UI_*_config.h`. This is intentional.
2. **Inner `fw-mchf.elf` overwritten** — building multiple configs in the same `mchf-eclipse/` dir clobbers the ELF. The outer Makefile handles copying to unique names.
3. **No `Dockerfile` yet** — Ubuntu 24.04 `apt install arm-none-eabi-gcc` is sufficient for local repro.
4. **LTO stack-usage `.su` files** — generated but not tracked; `.gitignore` excludes them.

---

## 10. Regression Size Table (2026-08-22)

| Config | text | data | bss | flash (text+data) | total dec |
|---|---|---|---|---|---|
| `f4-mchf` firmware | 432089 | 1968 | 96036 | 434057 | 530093 |
| `f4-small` firmware | 286233 | 1904 | 94532 | 288137 | 382669 |
| `f7-ovi40` firmware | 481361 | 2656 | 104936 | 484017 | 588953 |
| `h7-ovi40` firmware | 490769 | 2652 | 105312 | 493421 | 598733 |
| `f4-mchf` bootloader | 12360 | 8 | 2472 | 12368 | 14840 |
| `f7-ovi40` bootloader | 13320 | 8 | 2488 | 13328 | 15816 |
| `h7-ovi40` bootloader | 14372 | 8 | 2544 | 14380 | 16924 |

> Full table populated by running `make all-firmware && make all-bootloader && make size-summary`.