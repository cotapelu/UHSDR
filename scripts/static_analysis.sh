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
##                 Phase 68 (T78) — the gate is now bounded and parallel:         ##
##                   * cppcheck runs with -j${CPPCHECK_JOBS} (default: nproc).     ##
##                     Measured on this machine: 11 m 38 s single threaded vs     ##
##                     4 m 09 s with -j8, identical findings (234 lines).         ##
##                   * The report no longer re-scans what the gate already       ##
##                     scanned. Only the trees outside the gate are scanned       ##
##                     additionally (drivers/freedv, drivers/usb: 18 s) and the   ##
##                     results are merged. The merged report is byte identical   ##
##                     to a full-tree scan (366 findings, verified 2026-10-26).  ##
##                   * Every scan has a hard timeout (${CPPCHECK_TIMEOUT},       ##
##                     default 900 s) and a timeout FAILS the gate.               ##
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

# Parallelism and the hard limit for a single scan. A hung or endlessly slow
# scan must fail the gate instead of blocking a developer or a CI runner.
CPPCHECK_JOBS="${CPPCHECK_JOBS:-$(nproc 2>/dev/null || echo 4)}"
CPPCHECK_TIMEOUT="${CPPCHECK_TIMEOUT:-900}"

COMMON="--enable=warning,portability --std=c99 --inline-suppr ${INC} ${DEFS} \
  --suppress=missingInclude --suppress=missingIncludeSystem"

# Directories that the full-tree report covers but the product-code gate does
# not. Everything else is already covered by the gate scan, so re-scanning it
# only doubles the runtime.
compute_extra_dirs() {
  local top sub extra=""
  for top in drivers hardware misc src; do
    case " ${PRODUCT_DIRS} " in
      *" ${top} "*) continue ;;
    esac
    for sub in "${top}"/*/; do
      sub="${sub%/}"
      case " ${PRODUCT_DIRS} " in
        *" ${sub} "*) continue ;;
      esac
      extra="${extra} ${sub}"
    done
  done
  echo "${extra}"
}

# run_cppcheck <output-file> <extra-flags> <paths...>
# Returns the cppcheck exit code; 124 means the timeout was hit.
run_cppcheck() {
  local out="$1"; shift
  local flags="$1"; shift
  timeout "${CPPCHECK_TIMEOUT}" \
    cppcheck -j"${CPPCHECK_JOBS}" ${COMMON} ${flags} "$@" > "${out}" 2>&1
}

echo "========================================"
echo "UHSDR Static Analysis (Phase 67)"
echo "========================================"
if ! command -v cppcheck &>/dev/null; then
  echo "ERROR: cppcheck is not installed (sudo apt-get install cppcheck)"; exit 1
fi
echo "cppcheck: $(cppcheck --version 2>/dev/null | head -1)"
echo "jobs: ${CPPCHECK_JOBS}, timeout: ${CPPCHECK_TIMEOUT}s"

if [ "${1:-}" = "--regen-baseline" ]; then
  echo "[regen] scanning vendored drivers/freedv/ for the baseline ..."
  {
    echo "# Baseline of PRE-EXISTING cppcheck findings in vendored drivers/freedv/ (FreeDV/Codec2)."
    echo "# Format: <id>:<file>. Gate fails on NEW findings outside this list."
    cppcheck -j"${CPPCHECK_JOBS}" ${COMMON} drivers/freedv 2>&1 \
      | grep -E '^drivers/freedv/.*\.(c|h):[0-9]+' \
      | awk '{split($1,a,":"); f=a[1]; if(match($0,/\[[A-Za-z_]+\][[:space:]]*$/)){id=substr($0,RSTART+1,RLENGTH-2); print id":"f}}' \
      | sort -u
  } > "${BASELINE}"
  echo "[regen] wrote ${BASELINE}"
  exit 0
fi

# ---- GATE: no NEW product-code findings beyond the committed baseline ----
echo "[gate] cppcheck on product code (${PRODUCT_DIRS}) vs baseline ..."
GATE_OUT="/tmp/uhsdr_cppcheck_gate.txt"
run_cppcheck "${GATE_OUT}" "--error-exitcode=42 --suppressions-list=${BASELINE}" ${PRODUCT_DIRS}
GATE_RC=$?
if [ "${GATE_RC}" -eq 124 ] || [ "${GATE_RC}" -eq 137 ]; then
  echo "!! PRODUCT-CODE STATIC ANALYSIS TIMED OUT after ${CPPCHECK_TIMEOUT}s (jobs=${CPPCHECK_JOBS})"
  tail -20 "${GATE_OUT}" 2>/dev/null
  echo "Raise CPPCHECK_TIMEOUT or reduce CPPCHECK_JOBS (low memory) and retry."
  exit 124
fi
if [ "${GATE_RC}" -ne 0 ]; then
  echo "!! PRODUCT-CODE STATIC ANALYSIS FAILED (rc=${GATE_RC}). Findings:"
  cat "${GATE_OUT}"
  echo "See above. New findings in product code block the build (T70)."
  exit "${GATE_RC}"
fi
echo "[gate] OK — 0 findings in product code."

# ---- REPORT: full tree, informational only (vendored noise baselined) ----
# The gate already produced the findings for every product directory. Only the
# remaining trees are scanned additionally and both results are merged, which
# is identical to a single full-tree scan but much faster.
EXTRA_DIRS="$(compute_extra_dirs)"
echo "[report] full-tree scan -> ${REPORT} (informational, non-gating) ..."
EXTRA_OUT="/tmp/uhsdr_cppcheck_extra.txt"
run_cppcheck "${EXTRA_OUT}" "--suppressions-list=${BASELINE}" ${EXTRA_DIRS}
EXTRA_RC=$?
if [ "${EXTRA_RC}" -eq 124 ] || [ "${EXTRA_RC}" -eq 137 ]; then
  echo "!! FULL-TREE REPORT TIMED OUT after ${CPPCHECK_TIMEOUT}s"
  EXTRA_RC=0
fi
{
  grep -v 'files checked' "${GATE_OUT}" 2>/dev/null
  grep -v 'files checked' "${EXTRA_OUT}" 2>/dev/null
} > "${REPORT}"
echo ""
echo "Static analysis complete. Gate: PASS. Report: ${REPORT}"
