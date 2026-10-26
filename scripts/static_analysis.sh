#!/bin/bash
#  -*-  mode: sh; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-
####################################################################################
##                                                                                ##
##                                        UHSDR                                  ##
##               a powerful firmware for STM32 based SDR transceivers              ##
##                                                                                ##
##---------------------------------------------------------------------------------##
##                                                                                ##
##  File name:     static_analysis.sh                                             ##
##  Description:   Run cppcheck on product code (GATE) + full tree (report)       ##
##  Modified:      Phase 67.3 — was `2>&1 | tee ... || true` (never gated). Now   ##
##                 the product-code scan EXITS NON-ZERO on findings.               ##
##                                                                                ##
##  Usage:         scripts/static_analysis.sh            # gate + report (default) ##
##                 scripts/static_analysis.sh --regen-baseline                     ##
##                                                                                ##
##  Licence:		GNU GPLv3                                                      ##
####################################################################################

set -o pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${REPO_ROOT}/mchf-eclipse" || exit 2

BASELINE="${REPO_ROOT}/scripts/cppcheck_suppressions.txt"
REPORT="${REPO_ROOT}/cppcheck_report.txt"

# Include paths mirror mchf-eclipse/Makefile for the F4/mcHF target.
INC="-I. -Ihal/include -Ihardware -Ihardware/board_configs \
  -Idrivers/audio -Idrivers/audio/codec -Idrivers/audio/softdds -Idrivers/audio/cw -Idrivers/audio/filters \
  -Idrivers/ui -Idrivers/ui/encoder -Idrivers/ui/lcd -Idrivers/ui/menu -Idrivers/ui/oscillator \
  -Imisc/v_eprom -Imisc -Isrc -Isrc/bootloader -Idrivers/cat -Idrivers/remote -Idrivers/diag \
  -Idrivers/usb/app -Idrivers/usb/device/class/composite \
  -Idrivers/usb/device/class/AUDIO/Inc -Idrivers/usb/device/class/CDC/Inc \
  -Ibasesw/mcHF/Inc \
  -Ibasesw/mcHF/Drivers/CMSIS/Device/ST/STM32F4xx/Include \
  -Ibasesw/mcHF/Drivers/STM32F4xx_HAL_Driver/Inc \
  -Ibasesw/mcHF/Drivers/CMSIS/Include"

# __GNUC__ keeps core_cm4.h from firing "#error Unknown compiler" under cppcheck.
DEFS="-D__GNUC__ -DARM_MATH_CM4 -DUI_BRD_MCHF -DRF_BRD_MCHF -DCORTEX_M4 -DSTM32F407xx -DUSE_ALTERNATE_NR"

# PRODUCT code owned by this project (AGENTS.md 5.3). Vendored third-party
# (drivers/freedv, basesw/) is EXCLUDED from the gate and covered by BASELINE.
PRODUCT_DIRS="drivers/audio drivers/ui hardware misc src drivers/cat drivers/remote drivers/diag"

COMMON="--enable=warning,portability --std=c99 --inline-suppr ${INC} ${DEFS} \
  --suppress=missingInclude --suppress=missingIncludeSystem"

echo "========================================"
echo "UHSDR Static Analysis (Phase 67)"
echo "========================================"
if ! command -v cppcheck &>/dev/null; then
  echo "ERROR: cppcheck is not installed (sudo apt-get install cppcheck)"; exit 1
fi
echo "cppcheck: $(cppcheck --version 2>/dev/null | head -1)"

if [ "${1:-}" = "--regen-baseline" ]; then
  echo "[regen] scanning vendored drivers/freedv/ for the baseline ..."
  {
    echo "# Baseline of PRE-EXISTING cppcheck findings in vendored drivers/freedv/ (FreeDV/Codec2)."
    echo "# Format: <id>:<file>. Gate fails on NEW findings outside this list."
    cppcheck ${COMMON} drivers/freedv 2>&1 \
      | grep -E '^drivers/freedv/.*\.(c|h):[0-9]+' \
      | awk '{split($1,a,":"); f=a[1]; if(match($0,/\[[A-Za-z_]+\][[:space:]]*$/)){id=substr($0,RSTART+1,RLENGTH-2); print id":"f}}' \
      | sort -u
  } > "${BASELINE}"
  echo "[regen] wrote ${BASELINE}"
  exit 0
fi

# ---- GATE: no NEW product-code findings beyond the committed baseline ----
echo "[gate] cppcheck on product code (${PRODUCT_DIRS}) vs baseline ..."
cppcheck ${COMMON} --error-exitcode=42 --suppressions-list="${BASELINE}" ${PRODUCT_DIRS} > /tmp/uhsdr_cppcheck_gate.txt 2>&1
GATE_RC=$?
if [ "${GATE_RC}" -ne 0 ]; then
  echo "!! PRODUCT-CODE STATIC ANALYSIS FAILED (rc=${GATE_RC}). Findings:"
  cat /tmp/uhsdr_cppcheck_gate.txt
  echo "See above. New findings in product code block the build (T70)."
  exit "${GATE_RC}"
fi
echo "[gate] OK — 0 findings in product code."

# ---- REPORT: full tree, informational only (vendored noise baselined) ----
echo "[report] full-tree scan -> ${REPORT} (informational, non-gating) ..."
{
  cppcheck ${COMMON} --suppressions-list="${BASELINE}" \
    drivers/ hardware/ misc/ src/ 2>&1
} | tee "${REPORT}" >/dev/null
echo ""
echo "Static analysis complete. Gate: PASS. Report: ${REPORT}"
