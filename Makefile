#
# UHSDR - Universal Ham Software Defined Radio
# Root Makefile for building firmware and bootloader
#
# Supports multiple MCU targets: F4, F4-512KB, F7, H7
SHELL := /bin/bash
# Supports multiple hardware configurations: mchf, ovi40
#
# Usage:
#   make                    # Show help
#   make all                # Build firmware for default target (F4, mchf)
#   make firmware           # Build firmware only
#   make bootloader         # Build bootloader only
#   make BUILDFOR=F7 all    # Build for STM32F7
#   make BOARD=ovi40 all    # Build for OVI40 hardware
#   make BUILDFOR=H7 BOARD=ovi40 all  # Build for H7 on OVI40
#   make DEBUG=1 all        # Build with debug symbols
#   make clean              # Clean all build artifacts
#   make clean-firmware     # Clean firmware only
#   make clean-bootloader   # Clean bootloader only
#   make TRX_ID=myradio     # Build with custom TRX_ID
#

# =============================================================================
# Default Configuration
# =============================================================================

# MCU Target: F4, F4-512KB, F7, H7
BUILDFOR ?= F4

# Hardware Board: mchf, ovi40
BOARD = mchf

# Transceiver ID (max 5 chars)
TRX_ID ?= mchf

# Transceiver Name (will use TRX_ID if not set)
TRX_NAME ?= $(TRX_ID)

# Build type: release (default) or debug
DEBUG ?= 0

# Verbose output
VERBOSE ?= 0

# Optional custom toolchain path
OPT_GCC_ARM ?=

# =============================================================================
# Internal Variables
# =============================================================================

# Path to the actual build system
BUILD_DIR := mchf-eclipse

# Determine CONFIGFLAGS based on board
ifeq ($(BOARD),ovi40)
  CONFIGFLAGS := -DUI_BRD_OVI40 -DRF_BRD_OVI40
else ifeq ($(BOARD),mchf)
  CONFIGFLAGS := -DUI_BRD_MCHF -DRF_BRD_MCHF
else
  $(error Unknown BOARD: $(BOARD). Supported: mchf, ovi40)
endif

# Validate BUILDFOR
VALID_BUILDFOR := F4 F4-512KB F7 H7
ifneq ($(filter $(BUILDFOR),$(VALID_BUILDFOR)),$(BUILDFOR))
  $(error Invalid BUILDFOR: $(BUILDFOR). Supported: $(VALID_BUILDFOR))
endif

# Pass configuration to sub-make
SUBMAKE_FLAGS := \
	BUILDFOR=$(BUILDFOR) \
	CONFIGFLAGS="$(CONFIGFLAGS)" \
	TRX_ID=$(TRX_ID) \
	TRX_NAME="$(TRX_NAME)" \
	DEBUG=$(DEBUG) \
	VERBOSE=$(VERBOSE) \
	OPT_GCC_ARM=$(OPT_GCC_ARM) \
	ROOTLOC=.

# Helper macro for building firmware/bootloader in sub-make with explicit variables
# Usage: $(call BUILD_CMD,<BUILDFOR>,<BOARD>,<CONFIGFLAGS>,<target>)
# This avoids GNU Make target-specific variable inheritance bugs
define BUILD_CMD
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) BUILDFOR=$(1) BOARD=$(2) CONFIGFLAGS="$(3)" $(4)
endef

# =============================================================================
# Main Targets
# =============================================================================

.PHONY: all help firmware bootloader clean clean-firmware clean-bootloader clean-libs info \
	doctor check size-summary \
	docs docs-clean handbook handbook-test handbook-ui-menu handbook-ui-menu-clean \
	gcc-version release handy version

# Default target - build ALL firmware and bootloader combinations
all: all-firmware all-bootloader

help:
	@echo ""
	@echo "======================================================================"
	@echo " UHSDR - Universal Ham Software Defined Radio Build System"
	@echo "======================================================================"
	@echo ""
	@echo "Usage: make [target] [options]"
	@echo ""
	@echo "Main Targets:"
	@echo "  all              - Build ALL firmware (5) + ALL bootloader (4)"
	@echo "  firmware         - Build firmware only for current config"
	@echo "  bootloader       - Build bootloader only for current config"
	@echo "  both             - Build firmware + bootloader for current config"
	@echo "  all-firmware     - Build all 5 firmware combinations"
	@echo "  all-bootloader   - Build all 4 bootloader combinations"
	@echo "  clean            - Clean all build artifacts"
	@echo "  clean-firmware   - Clean firmware build artifacts"
	@echo "  clean-bootloader - Clean bootloader build artifacts"
	@echo "  clean-libs       - Clean library object files"
	@echo ""
	@echo "Documentation:"
	@echo "  docs             - Generate Doxygen documentation"
	@echo "  docs-clean       - Remove generated documentation"
	@echo "  handbook         - Generate user handbook"
	@echo "  handbook-test    - Generate handbook in test directory"
	@echo "  handbook-ui-menu - Generate UI menu documentation"
	@echo "  handbook-ui-menu-clean - Clean UI menu documentation"
	@echo ""
	@echo "Information:"
	@echo "  gcc-version      - Show compiler version"
	@echo "  version          - Show firmware version"
	@echo "  release          - Generate quick operating guide"
	@echo "  handy            - Remove all .o files (keep executables)"
	@echo ""
	@echo "Configuration Options:"
	@echo "  BUILDFOR=F4|F4-512KB|F7|H7   - Target MCU (default: F4)"
	@echo "  BOARD=mchf|ovi40             - Hardware board (default: mchf)"
	@echo "  TRX_ID=xxxxx                 - Transceiver ID, max 5 chars (default: mchf)"
	@echo "  TRX_NAME='name'              - Transceiver name (default: TRX_ID)"
	@echo "  DEBUG=1                      - Enable debug build (default: 0)"
	@echo "  VERBOSE=1                    - Verbose output (default: 0)"
	@echo "  OPT_GCC_ARM=/path/to/gcc     - Custom toolchain path"
	@echo ""
	@echo "Examples:"
	@echo "  make firmware                          # Build F4 firmware for mcHF"
	@echo "  make BUILDFOR=F7 firmware              # Build F7 firmware for mcHF"
	@echo "  make BUILDFOR=H7 BOARD=ovi40 firmware  # Build H7 firmware for OVI40"
	@echo "  make BUILDFOR=F4-512KB firmware        # Small build for F4 (512KB flash)"
	@echo "  make DEBUG=1 firmware                  # Debug build"
	@echo "  make bootloader                        # Build bootloader"
	@echo "  make TRX_ID=myradio firmware           # Custom TRX_ID"
	@echo "  make clean                             # Clean everything"
	@echo ""
	@echo "Current Configuration:"
	@echo "  BUILDFOR = $(BUILDFOR)"
	@echo "  BOARD    = $(BOARD)"
	@echo "  TRX_ID   = $(TRX_ID)"
	@echo "  TRX_NAME = $(TRX_NAME)"
	@echo "  DEBUG    = $(DEBUG)"
	@echo "  CONFIGFLAGS = $(CONFIGFLAGS)"
	@echo ""

# Build firmware
firmware:
	@echo "Building firmware for $(BUILDFOR) on $(BOARD)..."
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) BUILDFOR="$(BUILDFOR)" BOARD="$(BOARD)" CONFIGFLAGS="$(CONFIGFLAGS)" firmware

# Build bootloader
bootloader:
	@echo "Building bootloader for $(BUILDFOR) on $(BOARD)..."
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) BUILDFOR="$(BUILDFOR)" BOARD="$(BOARD)" CONFIGFLAGS="$(CONFIGFLAGS)" bootloader

# Build both firmware and bootloader
both: firmware bootloader

# Build ALL firmware combinations (5 configs)
all-firmware:
	@echo "Building ALL firmware combinations..."
	@$(MAKE) -C $(BUILD_DIR) -f Makefile BUILDFOR=F4 clean-firmware
	@mkdir -p build
	@$(MAKE) f4-mchf && cp mchf-eclipse/fw-mchf.bin build/fw-mchf_f4-mchf.bin && cp mchf-eclipse/fw-mchf.dfu build/fw-mchf_f4-mchf.dfu
	@$(MAKE) -C $(BUILD_DIR) -f Makefile BUILDFOR=F4-512KB clean-firmware
	@$(MAKE) f4-small && cp mchf-eclipse/fw-mchf.bin build/fw-mchf_f4-small.bin && cp mchf-eclipse/fw-mchf.dfu build/fw-mchf_f4-small.dfu
	@$(MAKE) -C $(BUILD_DIR) -f Makefile BUILDFOR=F7 clean-firmware
	@$(MAKE) f7-ovi40 && cp mchf-eclipse/fw-mchf.bin build/fw-mchf_f7-ovi40.bin && cp mchf-eclipse/fw-mchf.dfu build/fw-mchf_f7-ovi40.dfu
	@$(MAKE) -C $(BUILD_DIR) -f Makefile BUILDFOR=H7 clean-firmware
	@$(MAKE) h7-ovi40 && cp mchf-eclipse/fw-mchf.bin build/fw-mchf_h7-ovi40.bin && cp mchf-eclipse/fw-mchf.dfu build/fw-mchf_h7-ovi40.dfu
	@echo "ALL firmware builds completed!"
	@ls -la build/fw-mchf_*.bin 2>/dev/null || true

# Build ALL bootloader combinations (3 valid configs)
all-bootloader:
	@echo "Building ALL bootloader combinations..."
	@$(MAKE) -C $(BUILD_DIR) -f Makefile clean-bootloader
	@mkdir -p build
	@$(MAKE) -C $(BUILD_DIR) -f Makefile BUILDFOR=F4 CONFIGFLAGS="-DUI_BRD_MCHF -DRF_BRD_MCHF" bootloader && cp mchf-eclipse/bl-mchf.bin build/bl-mchf_f4-mchf.bin && cp mchf-eclipse/bl-mchf.dfu build/bl-mchf_f4-mchf.dfu
	@$(MAKE) -C $(BUILD_DIR) -f Makefile clean-bootloader
	@$(MAKE) -C $(BUILD_DIR) -f Makefile BUILDFOR=F7 CONFIGFLAGS="-DUI_BRD_OVI40 -DRF_BRD_OVI40" bootloader && cp mchf-eclipse/bl-mchf.bin build/bl-mchf_f7-ovi40.bin && cp mchf-eclipse/bl-mchf.dfu build/bl-mchf_f7-ovi40.dfu
	@$(MAKE) -C $(BUILD_DIR) -f Makefile clean-bootloader
	@$(MAKE) -C $(BUILD_DIR) -f Makefile BUILDFOR=H7 CONFIGFLAGS="-DUI_BRD_OVI40 -DRF_BRD_OVI40" bootloader && cp mchf-eclipse/bl-mchf.bin build/bl-mchf_h7-ovi40.bin && cp mchf-eclipse/bl-mchf.dfu build/bl-mchf_h7-ovi40.dfu
	@$(MAKE) -C $(BUILD_DIR) -f Makefile clean-bootloader
	@echo "ALL bootloader builds completed!"
	@ls -la build/bl-mchf_*.bin 2>/dev/null || true

# Clean targets
clean:
	@echo "Cleaning all build artifacts..."
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) clean

clean-firmware:
	@echo "Cleaning firmware artifacts..."
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) clean-firmware

clean-bootloader:
	@echo "Cleaning bootloader artifacts..."
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) clean-bootloader

clean-libs:
	@echo "Cleaning library objects..."
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) clean-libs

# Documentation
docs:
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) docs

docs-clean:
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) docs-clean

# Handbook generation
handbook:
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) $(TRX_ID).handbook

handbook-test:
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) handbook-test

handbook-ui-menu:
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) handbook-ui-menu

handbook-ui-menu-clean:
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) handbook-ui-menu-clean

# Information targets
gcc-version:
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) gcc-version

version:
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) $(TRX_ID).version

release:
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) release

handy:
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) handy

# =============================================================================
# Convenience Targets for Common Configurations
# =============================================================================

# Quick targets for different MCU targets
f4: 
	$(call BUILD_CMD,F4,mchf,-DUI_BRD_MCHF -DRF_BRD_MCHF,firmware)

f4-small: 
	$(call BUILD_CMD,F4-512KB,mchf,-DUI_BRD_MCHF -DRF_BRD_MCHF -DIS_SMALL_BUILD,firmware)

f7: 
	$(call BUILD_CMD,F7,ovi40,-DUI_BRD_OVI40 -DRF_BRD_OVI40,firmware)

h7: 
	$(call BUILD_CMD,H7,ovi40,-DUI_BRD_OVI40 -DRF_BRD_OVI40,firmware)

# Quick targets for different boards
mchf: 
	$(call BUILD_CMD,F4,mchf,-DUI_BRD_MCHF -DRF_BRD_MCHF,firmware)

ovi40: 
	$(call BUILD_CMD,F4,ovi40,-DUI_BRD_OVI40 -DRF_BRD_OVI40,firmware)

# Combined quick targets
f4-mchf: 
	$(call BUILD_CMD,F4,mchf,-DUI_BRD_MCHF -DRF_BRD_MCHF,firmware)

f4-ovi40: 
	$(call BUILD_CMD,F4,ovi40,-DUI_BRD_OVI40 -DRF_BRD_OVI40,firmware)

f7-mchf: 
	$(call BUILD_CMD,F7,mchf,-DUI_BRD_MCHF -DRF_BRD_MCHF,firmware)

f7-ovi40: 
	$(call BUILD_CMD,F7,ovi40,-DUI_BRD_OVI40 -DRF_BRD_OVI40,firmware)

h7-mchf: 
	$(call BUILD_CMD,H7,mchf,-DUI_BRD_MCHF -DRF_BRD_MCHF,firmware)

h7-ovi40: 
	$(call BUILD_CMD,H7,ovi40,-DUI_BRD_OVI40 -DRF_BRD_OVI40,firmware)

# Debug builds
debug-f4: 
	$(call BUILD_CMD,F4,mchf,-DUI_BRD_MCHF -DRF_BRD_MCHF -DDEBUG -DUSE_FULL_ASSERT -DTRACE,firmware)

debug-f7: 
	$(call BUILD_CMD,F7,ovi40,-DUI_BRD_OVI40 -DRF_BRD_OVI40 -DDEBUG -DUSE_FULL_ASSERT -DTRACE,firmware)

debug-h7: 
	$(call BUILD_CMD,H7,ovi40,-DUI_BRD_OVI40 -DRF_BRD_OVI40 -DDEBUG -DUSE_FULL_ASSERT -DTRACE,firmware)

# =============================================================================
# Utility Targets
# =============================================================================

# Show current configuration
config:
	@echo "BUILDFOR    = $(BUILDFOR)"
	@echo "BOARD       = $(BOARD)"
	@echo "TRX_ID      = $(TRX_ID)"
	@echo "TRX_NAME    = $(TRX_NAME)"
	@echo "DEBUG       = $(DEBUG)"
	@echo "VERBOSE     = $(VERBOSE)"
	@echo "OPT_GCC_ARM = $(OPT_GCC_ARM)"
	@echo "CONFIGFLAGS = $(CONFIGFLAGS)"

# Build matrix summary (T23.5)
info:
	@echo "UHSDR Build Matrix Summary"
	@echo "=========================="
	@echo ""
	@echo "Valid firmware targets:"
	@echo "  f4-mchf     — STM32F4 (mcHF board)"
	@echo "  f4-small    — STM32F4-512KB (mcHF, reduced features)"
	@echo "  f7-ovi40    — STM32F7 (OVI40 board)"
	@echo "  h7-ovi40    — STM32H7 (OVI40 board)"
	@echo ""
	@echo "Valid bootloader targets:"
	@echo "  bl-mchf F4  — STM32F4 mcHF bootloader"
	@echo "  bl-mchf F7  — STM32F7 OVI40 bootloader"
	@echo "  bl-mchf H7  — STM32H7 OVI40 bootloader"
	@echo ""
	@echo "Invalid combos (intentional, guarded by board_config #error):"
	@echo "  f4-ovi40, f7-mchf, h7-mchf, h7-mchf"
	@echo ""
	@echo "Current default: BUILDFOR=$(BUILDFOR) BOARD=$(BOARD)"
	@echo "  CONFIGFLAGS=$(CONFIGFLAGS)"
	@echo ""
	@echo "Quick targets: make f4-mchf | make f4-small | make f7-ovi40 | make h7-ovi40"
	@echo "Meta targets:  make all-firmware | make all-bootloader | make clean"
	@echo "New checks:    make doctor | make check | make size-summary | make info"
	@echo ""
	@echo "Toolchain:"
	@which arm-none-eabi-gcc >/dev/null 2>&1 && \
	  arm-none-eabi-gcc --version | head -1 | sed 's/^/  /' || \
	  echo "  WARNING: arm-none-eabi-gcc not found in PATH"

# List all available targets
list-targets:
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) help

# Check toolchain
check-toolchain:
	@echo "Checking ARM GCC toolchain..."
	@which arm-none-eabi-gcc || (echo "ERROR: arm-none-eabi-gcc not found in PATH" && exit 1)
	@arm-none-eabi-gcc --version | head -1

# =============================================================================
# CI/CD Helper Targets
# =============================================================================

# Build all supported configurations (for CI)
ci-build-all:
	@echo "Building all configurations for CI..."
	@$(MAKE) f4-mchf
	@$(MAKE) f4-ovi40
	@$(MAKE) f7-mchf
	@$(MAKE) f7-ovi40
	@$(MAKE) h7-mchf
	@$(MAKE) h7-ovi40
	@$(MAKE) f4-small
	@echo "All CI builds completed successfully!"

# Build all bootloaders
ci-bootloaders:
	@echo "Building all bootloaders..."
	@$(MAKE) BUILDFOR=F4 BOARD=mchf bootloader
	@$(MAKE) BUILDFOR=F4 BOARD=ovi40 bootloader
	@$(MAKE) BUILDFOR=F7 BOARD=mchf bootloader
	@$(MAKE) BUILDFOR=F7 BOARD=ovi40 bootloader
	@$(MAKE) BUILDFOR=H7 BOARD=mchf bootloader
	@$(MAKE) BUILDFOR=H7 BOARD=ovi40 bootloader
	@echo "All bootloader builds completed!"

# =============================================================================
# Phony declarations
# =============================================================================
# =============================================================================
# Development / Reproducibility Checks (T22.x)
# =============================================================================

# doctor: pre-flight build environment check (T22.1)
#   - verifies ARM toolchain on PATH
#   - checks critical MCU dirs under mchf-eclipse/ exist
#   - checks board configs exist for each board
# Run from repo root. Exit 0 = all good; exit 1 = fix before building.
doctor:
	@set +e; FAIL=0; \
	TOOL=$$(which arm-none-eabi-gcc 2>/dev/null); \
	if [ -z "$$TOOL" ]; then \
	  echo "  FAIL: arm-none-eabi-gcc not in PATH"; FAIL=1; \
	else \
	  echo "  OK  : gcc $$(arm-none-eabi-gcc --version | head -1)"; \
	fi; \
	for d in mchf-eclipse/hardware/board_configs \
	          mchf-eclipse/basesw/mcHF/Src \
	          mchf-eclipse/basesw/ovi40/Src \
	          mchf-eclipse/basesw/ovi40-h7/Src; do \
	  [ -d "$$d" ] && echo "  OK  : $$d" || { echo "  FAIL: $$d missing"; FAIL=1; }; \
	done; \
	for f in mchf-eclipse/hardware/board_configs/UHSDR_UI_mchf_config.h \
	         mchf-eclipse/hardware/board_configs/UHSDR_UI_ovi40_config.h; do \
	  [ -f "$$f" ] && echo "  OK  : $$f" || { echo "  FAIL: $$f missing"; FAIL=1; }; \
	done; \
	for f in mchf-eclipse/files.mak mchf-eclipse/Makefile; do \
	  [ -f "$$f" ] && echo "  OK  : $$f" || { echo "  FAIL: $$f missing"; FAIL=1; }; \
	done; \
	printf "  %-14s " "diagnosis:"; \
	[ "$$FAIL" -eq 0 ] \
	  && echo "PASSED — environment is ready." \
	  || echo "FAILED — fix issues above before building."; \
	exit $$FAIL


# check: lightweight repo sanity (no toolchain required)
#   - no uncommitted changes in tracked product sources
#   - BUILD_DIR and key .mak files exist
check:
	@echo "[check] UHSDR repository sanity"
	@echo ""
	@set +e; FAIL=0; \
	if git diff --quiet -- mchf-eclipse/ docs/ Makefile .gitignore 2>/dev/null; then \
	  echo "  OK  : no uncommitted changes in product tree"; \
	else \
	  echo "  WARN: tracked files have uncommitted changes"; \
	  FAIL=1; \
	fi; \
	if [ ! -f mchf-eclipse/Makefile ]; then \
	  echo "  FAIL: mchf-eclipse/Makefile missing"; FAIL=1; \
	else \
	  echo "  OK  : mchf-eclipse/Makefile present"; \
	fi; \
	if [ ! -f mchf-eclipse/files.mak ]; then \
	  echo "  FAIL: mchf-eclipse/files.mak missing"; FAIL=1; \
	else \
	  echo "  OK  : mchf-eclipse/files.mak present"; \
	fi; \
	for d in mchf-eclipse/f4-files.mak mchf-eclipse/f7-files.mak mchf-eclipse/h7-files.mak mchf-eclipse/bootloader.mak mchf-eclipse/include.mak; do \
	  if [ ! -f "$$d" ]; then \
	    echo "  WARN: missing make fragment $$d"; FAIL=1; \
	  fi; \
	done; \
	if [ "$$FAIL" -ne 0 ]; then \
	  echo "[check] ISSUES FOUND — see WARN/FAIL above"; \
	  exit 1; \
	else \
	  echo "[check] PASSED"; \
	fi

# size-summary: print text/data/bss for last-built firmware and bootloader ELFs
# Artifacts are named fw-mchf.elf / bl-mchf.elf in mchf-eclipse/ (last build wins).
# Requires a working toolchain and at least one prior firmware+bootloader build.
size-summary:
	@echo "=== UHSDR Firmware Size (mchf-eclipse/fw-mchf.elf) ==="
	@echo ""
	@if [ -f mchf-eclipse/fw-mchf.elf ]; then \
	  read -r _txt _data _bss _dec _hex _rest < <(arm-none-eabi-size mchf-eclipse/fw-mchf.elf | tail -1); \
	  echo "  mchf-eclipse/fw-mchf.elf  text=$$_txt  data=$$_data  bss=$$_bss  flash=$$((_txt + _data))  total=$$_dec"; \
	  stat -c '  Built: %y' mchf-eclipse/fw-mchf.elf; \
	else \
	  echo "  MISSING — run 'make f4-mchf' (or any firmware target) first"; \
	fi
	@echo ""
	@echo "=== UHSDR Bootloader Size (mchf-eclipse/bl-mchf.elf) ==="
	@echo ""
	@if [ -f mchf-eclipse/bl-mchf.elf ]; then \
	  read -r _txt _data _bss _dec _hex _rest < <(arm-none-eabi-size mchf-eclipse/bl-mchf.elf | tail -1); \
	  echo "  mchf-eclipse/bl-mchf.elf  text=$$_txt  data=$$_data  bss=$$_bss  flash=$$((_txt + _data))  total=$$_dec"; \
	  stat -c '  Built: %y' mchf-eclipse/bl-mchf.elf; \
	else \
	  echo "  MISSING — run 'make bootloader' first"; \
	fi

# stack-report: parse LTO .su files and show top-10 deepest call stacks in product code
# Requires prior build with LTO (default release builds).
stack-report:
	@echo "=== UHSDR Stack Usage Report (product code, top 10) ==="
	@echo ""
	@if ls mchf-eclipse/*.su >/dev/null 2>&1; then \
	  echo "Sorting by stack depth (bytes), product code only..."; \
	  grep -hE '^(drivers|hardware|misc|src)/' mchf-eclipse/*.su 2>/dev/null | \
	    sed 's/\t/:/g' | \
	    sort -t: -k4 -nr | \
	    head -10 | \
	    awk -F: '{printf "  %4s  %s:%s  %s\n", $$4, $$1, $$2, $$5}'; \
	  echo ""; \
	  echo "Legend: stack_bytes  file:line  function"; \
	else \
	  echo "  MISSING — .su files not found. Rebuild with LTO (default) first."; \
	fi


.PHONY: all help firmware bootloader clean clean-firmware clean-bootloader clean-libs info \
	f4-mchf f4-ovi40 f7-mchf f7-ovi40 h7-mchf h7-ovi40 \
	debug-f4 debug-f7 debug-h7 \
	config list-targets check-toolchain \
	ci-build-all ci-bootloaders both \
	doctor check size-summary stack-report
