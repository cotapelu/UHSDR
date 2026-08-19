#
# UHSDR - Universal Ham Software Defined Radio
# Root Makefile for building firmware and bootloader
#
# Supports multiple MCU targets: F4, F4-512KB, F7, H7
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
BOARD ?= mchf

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

# =============================================================================
# Main Targets
# =============================================================================

.PHONY: all help firmware bootloader clean clean-firmware clean-bootloader clean-libs \
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
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) CONFIGFLAGS="$(CONFIGFLAGS)" firmware

# Build bootloader
bootloader:
	@echo "Building bootloader for $(BUILDFOR) on $(BOARD)..."
	@$(MAKE) -C $(BUILD_DIR) -f Makefile $(SUBMAKE_FLAGS) CONFIGFLAGS="$(CONFIGFLAGS)" bootloader

# Build both firmware and bootloader
both: firmware bootloader

# Build ALL firmware combinations (5 configs)
all-firmware:
	@echo "Building ALL firmware combinations..."
	@$(MAKE) -C $(BUILD_DIR) -f Makefile clean-firmware
	@$(MAKE) f4-mchf && cp mchf-eclipse/fw-mchf.bin fw-mchf_f4-mchf.bin && cp mchf-eclipse/fw-mchf.dfu fw-mchf_f4-mchf.dfu
	@$(MAKE) -C $(BUILD_DIR) -f Makefile clean-firmware
	@$(MAKE) f4-small && cp mchf-eclipse/fw-mchf.bin fw-mchf_f4-small.bin && cp mchf-eclipse/fw-mchf.dfu fw-mchf_f4-small.dfu
	@$(MAKE) -C $(BUILD_DIR) -f Makefile clean-firmware
	@$(MAKE) f7-ovi40 && cp mchf-eclipse/fw-mchf.bin fw-mchf_f7-ovi40.bin && cp mchf-eclipse/fw-mchf.dfu fw-mchf_f7-ovi40.dfu
	@$(MAKE) -C $(BUILD_DIR) -f Makefile clean-firmware
	@$(MAKE) h7-ovi40 && cp mchf-eclipse/fw-mchf.bin fw-mchf_h7-ovi40.bin && cp mchf-eclipse/fw-mchf.dfu fw-mchf_h7-ovi40.dfu
	@echo "ALL firmware builds completed!"
	@ls -la fw-mchf_*.bin 2>/dev/null || true

# Build ALL bootloader combinations (3 valid configs)
all-bootloader:
	@echo "Building ALL bootloader combinations..."
	@$(MAKE) -C $(BUILD_DIR) -f Makefile BUILDFOR=F4 CONFIGFLAGS="-DUI_BRD_MCHF -DRF_BRD_MCHF" bootloader && cp mchf-eclipse/bl-mchf.bin bl-mchf_f4-mchf.bin && cp mchf-eclipse/bl-mchf.dfu bl-mchf_f4-mchf.dfu
	@$(MAKE) -C $(BUILD_DIR) -f Makefile clean-bootloader
	@$(MAKE) -C $(BUILD_DIR) -f Makefile BUILDFOR=F7 CONFIGFLAGS="-DUI_BRD_OVI40 -DRF_BRD_OVI40" bootloader && cp mchf-eclipse/bl-mchf.bin bl-mchf_f7-ovi40.bin && cp mchf-eclipse/bl-mchf.dfu bl-mchf_f7-ovi40.dfu
	@$(MAKE) -C $(BUILD_DIR) -f Makefile clean-bootloader
	@$(MAKE) -C $(BUILD_DIR) -f Makefile BUILDFOR=H7 CONFIGFLAGS="-DUI_BRD_OVI40 -DRF_BRD_OVI40" bootloader && cp mchf-eclipse/bl-mchf.bin bl-mchf_h7-ovi40.bin && cp mchf-eclipse/bl-mchf.dfu bl-mchf_h7-ovi40.dfu
	@$(MAKE) -C $(BUILD_DIR) -f Makefile clean-bootloader
	@echo "ALL bootloader builds completed!"
	@ls -la bl-mchf_*.bin 2>/dev/null || true

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
f4: BUILDFOR=F4
f4: firmware

f4-small: BUILDFOR=F4-512KB
f4-small: firmware

f7: BUILDFOR=F7
f7: firmware

h7: BUILDFOR=H7
h7: firmware

# Quick targets for different boards
mchf: BOARD=mchf
mchf: firmware

ovi40: BOARD=ovi40
ovi40: firmware

# Combined quick targets
f4-mchf: BUILDFOR=F4 BOARD=mchf
f4-mchf: CONFIGFLAGS=-DUI_BRD_MCHF -DRF_BRD_MCHF
f4-mchf: firmware

f4-ovi40: BUILDFOR=F4 BOARD=ovi40
f4-ovi40: CONFIGFLAGS=-DUI_BRD_OVI40 -DRF_BRD_OVI40
f4-ovi40: firmware

f7-mchf: BUILDFOR=F7 BOARD=mchf
f7-mchf: CONFIGFLAGS=-DUI_BRD_MCHF -DRF_BRD_MCHF
f7-mchf: firmware

f7-ovi40: BUILDFOR=F7 BOARD=ovi40
f7-ovi40: CONFIGFLAGS=-DUI_BRD_OVI40 -DRF_BRD_OVI40
f7-ovi40: firmware

h7-mchf: BUILDFOR=H7 BOARD=mchf
h7-mchf: CONFIGFLAGS=-DUI_BRD_MCHF -DRF_BRD_MCHF
h7-mchf: firmware

h7-ovi40: BUILDFOR=H7 BOARD=ovi40
h7-ovi40: CONFIGFLAGS=-DUI_BRD_OVI40 -DRF_BRD_OVI40
h7-ovi40: firmware

# Debug builds
debug-f4: BUILDFOR=F4 DEBUG=1
debug-f4: firmware

debug-f7: BUILDFOR=F7 DEBUG=1
debug-f7: firmware

debug-h7: BUILDFOR=H7 DEBUG=1
debug-h7: firmware

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
.PHONY: f4 f4-small f7 h7 mchf ovi40 \
	f4-mchf f4-ovi40 f7-mchf f7-ovi40 h7-mchf h7-ovi40 \
	debug-f4 debug-f7 debug-h7 \
	config list-targets check-toolchain \
	ci-build-all ci-bootloaders both