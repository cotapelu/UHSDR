#!/usr/bin/env bash
#
# UHSDR transceiver identity regression check (Phase 68, T80).
#
# Why this check exists
# ---------------------
# The board configuration headers (hardware/board_configs/UHSDR_UI_*_config.h)
# own the transceiver identity. The build system used to force
# -DTRX_NAME="mchf" into every configuration, so a firmware built for OVI40
# reported itself as "mchf Transceiver" over USB/CAT, in the FreeDV beacon and
# in the bootloader banner. The name is user visible, therefore it must follow
# the board it was built for.
#
# What is checked
# ---------------
#   1. Every supported build configuration resolves TRX_NAME to the identity of
#      its own board (resolved with the real compiler flags).
#   2. TRX_ID stays "mchf" in every configuration. TRX_ID is the release
#      artifact and DFU file base name ("fw-<TRX_ID>.bin" on the SD card) and
#      must stay board independent.
#   3. An explicit TRX_NAME override still wins over the board default.
#   4. If release artifacts exist in build/, the identity strings really are
#      present in the binaries (end to end check).
#
# Usage:  scripts/check_identity.sh
# Exit:   0 = all checks passed, 1 = at least one check failed.

set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LOG="${TMPDIR:-/tmp}/uhsdr-check-identity.log"

: > "$LOG"

CHECKS=0
FAILURES=0

log() {
    printf '%s\n' "$*"
    printf '%s\n' "$*" >> "$LOG"
}

fail() {
    CHECKS=$((CHECKS + 1))
    FAILURES=$((FAILURES + 1))
    log "  [FAIL] $*"
}

pass() {
    CHECKS=$((CHECKS + 1))
    log "  [ OK ] $*"
}

# Resolve the identity macros for a configuration through the real build path
# (root Makefile -> mchf-eclipse/Makefile), so the check exercises exactly the
# flags a firmware build uses. TRX_NAME is removed from the environment to
# make sure the board default is what gets tested.
resolve_identity() {
    local buildfor="$1" board="$2"
    shift 2
    env -u TRX_NAME make -C "$ROOT" --no-print-directory print-identity \
        BUILDFOR="$buildfor" BOARD="$board" "$@" 2>> "$LOG"
}

macro_value() {
    # $1 = preprocessor output, $2 = macro name
    printf '%s\n' "$1" | sed -n "s/^#define $2 \"\\(.*\\)\"\$/\\1/p" | head -1
}

check_config() {
    local id="$1" buildfor="$2" board="$3" expected_name="$4" forbidden="$5"

    log "Checking $id (BUILDFOR=$buildfor BOARD=$board) ..."

    local out name trx_id
    out="$(resolve_identity "$buildfor" "$board")"
    name="$(macro_value "$out" TRX_NAME)"
    trx_id="$(macro_value "$out" TRX_ID)"

    if [ -z "$name" ]; then
        fail "$id: TRX_NAME could not be resolved (see $LOG)"
        return
    fi

    if [ "$name" = "$expected_name" ]; then
        pass "$id: TRX_NAME = '$name'"
    else
        fail "$id: TRX_NAME = '$name', expected '$expected_name'"
    fi

    if [ "$trx_id" = "mchf" ]; then
        pass "$id: TRX_ID = '$trx_id' (artifact/DFU base name contract)"
    else
        fail "$id: TRX_ID = '$trx_id', expected 'mchf'"
    fi

    if [ -n "$forbidden" ] && printf '%s' "$name" | grep -qi -- "$forbidden"; then
        fail "$id: TRX_NAME '$name' must not contain '$forbidden'"
    fi
}

check_override() {
    local buildfor="$1" board="$2" override="$3"

    log "Checking explicit TRX_NAME override on $board ($buildfor) ..."

    local out name
    out="$(resolve_identity "$buildfor" "$board" "TRX_NAME=$override")"
    name="$(macro_value "$out" TRX_NAME)"

    if [ "$name" = "$override" ]; then
        pass "override: TRX_NAME = '$name'"
    else
        fail "override: TRX_NAME = '$name', expected '$override'"
    fi
}

check_artifacts() {
    local build_dir="$ROOT/build"

    if [ ! -d "$build_dir" ]; then
        log "[SKIP] artifact check: no build/ directory"
        return
    fi

    local artifacts
    artifacts="$(find "$build_dir" -maxdepth 1 -name '*.bin' 2>/dev/null | sort)"
    if [ -z "$artifacts" ]; then
        log "[SKIP] artifact check: no .bin artifacts in build/"
        return
    fi

    local strings_bin=""
    if command -v strings >/dev/null 2>&1; then
        strings_bin="strings"
    elif command -v arm-none-eabi-strings >/dev/null 2>&1; then
        strings_bin="arm-none-eabi-strings"
    fi

    # file | expected string | forbidden string 1 | forbidden string 2
    # The forbidden patterns are the full identity strings, not the bare board
    # name: an mcHF UI may be fitted with an OVI40 RF board, so the firmware
    # legitimately contains the runtime label "OVI40 RF Board".
    local cases=(
        "fw-mchf_f4-mchf.bin|mcHF QRP Transceiver|OVI40 F7 Transceiver|OVI40 H7 Transceiver"
        "fw-mchf_f4-small.bin|mcHF QRP Transceiver|OVI40 F7 Transceiver|OVI40 H7 Transceiver"
        "fw-mchf_f7-ovi40.bin|OVI40 F7 Transceiver|mcHF QRP Transceiver|"
        "fw-mchf_h7-ovi40.bin|OVI40 H7 Transceiver|mcHF QRP Transceiver|"
        "bl-mchf_f4-mchf.bin|mcHF QRP Bootloader|OVI40 F7 Bootloader|OVI40 H7 Bootloader"
        "bl-mchf_f7-ovi40.bin|OVI40 F7 Bootloader|mcHF QRP Bootloader|"
        "bl-mchf_h7-ovi40.bin|OVI40 H7 Bootloader|mcHF QRP Bootloader|"
    )

    local entry file expected forbidden forbidden2 path found bad
    for entry in "${cases[@]}"; do
        IFS='|' read -r file expected forbidden forbidden2 <<< "$entry"
        path="$build_dir/$file"

        if [ ! -f "$path" ]; then
            log "[SKIP] artifact $file not built"
            continue
        fi

        if [ -n "$strings_bin" ]; then
            found="$($strings_bin "$path" | grep -F -m1 -- "$expected" || true)"
        else
            found="$(grep -a -F -m1 -o -- "$expected" "$path" || true)"
        fi

        if [ -n "$found" ]; then
            pass "artifact $file contains '$expected'"
        else
            fail "artifact $file does not contain '$expected'"
        fi

        bad=""
        for forbidden in "$forbidden" "$forbidden2"; do
            [ -n "$forbidden" ] || continue
            if [ -n "$strings_bin" ]; then
                found="$($strings_bin "$path" | grep -F -m1 -- "$forbidden" || true)"
            else
                found="$(grep -a -F -m1 -o -- "$forbidden" "$path" || true)"
            fi
            [ -n "$found" ] && bad="$forbidden"
        done

        if [ -n "$bad" ]; then
            fail "artifact $file contains the wrong identity '$bad'"
        else
            pass "artifact $file does not contain a wrong identity"
        fi
    done
}

log "=== UHSDR transceiver identity check (T80) ==="
log "Repository: $ROOT"
log ""

# Firmware configurations of the supported build matrix.
check_config "f4-mchf"  "F4"       "mchf"  "mcHF QRP" "OVI40"
check_config "f4-small" "F4-512KB" "mchf"  "mcHF QRP" "OVI40"
check_config "f7-ovi40" "F7"       "ovi40" "OVI40 F7" "mcHF"
check_config "h7-ovi40" "H7"       "ovi40" "OVI40 H7" "mcHF"

log ""
check_override "H7" "ovi40" "Field Radio"

log ""
check_artifacts

log ""
log "=== $((CHECKS - FAILURES))/$CHECKS checks passed ==="
if [ "$FAILURES" -ne 0 ]; then
    log "FAIL: $FAILURES identity check(s) failed (details: $LOG)"
    exit 1
fi

log "PASS: every build configuration reports its own board identity"
exit 0
