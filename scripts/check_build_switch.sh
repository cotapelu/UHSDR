#!/usr/bin/env bash
# Regression check for UHSDR configuration-safe builds (Phase 68, T75).
#
# The legacy build emitted all objects in-tree. Switching MCU without a clean
# could therefore reuse the previous MCU's ELF and still exit successfully.
# This script verifies the ELF itself, not only make's exit code.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${REPO_ROOT}"

ELF="mchf-eclipse/fw-mchf.elf"
BOOT_ELF="mchf-eclipse/bl-mchf.elf"
READELF="${READELF:-arm-none-eabi-readelf}"

if ! command -v arm-none-eabi-gcc >/dev/null 2>&1; then
    echo "ERROR: arm-none-eabi-gcc is not in PATH" >&2
    exit 2
fi
if ! command -v "${READELF}" >/dev/null 2>&1; then
    echo "ERROR: ${READELF} is not in PATH" >&2
    exit 2
fi

elf_arch() {
    "${READELF}" -A "$1" | sed -n 's/^[[:space:]]*Tag_FP_arch:[[:space:]]*//p'
}

elf_mtime() {
    stat -c '%Y' "$1"
}

require_fp_arch() {
    local elf="$1"
    local expected="$2"
    local actual
    actual="$(elf_arch "${elf}")"
    if [[ "${actual}" != *"${expected}"* ]]; then
        echo "FAIL: ${elf}: expected FP architecture '${expected}', got '${actual}'" >&2
        exit 1
    fi
}

BUILD_LOG="${TMPDIR:-/tmp}/uhsdr-build-switch.log"
run_make() {
    if ! make "$@" >"${BUILD_LOG}" 2>&1; then
        echo "FAIL: make $* failed; tail of ${BUILD_LOG}:" >&2
        tail -100 "${BUILD_LOG}" >&2
        exit 1
    fi
}

echo "[1/7] clean build tree"
make clean >/dev/null

echo "[2/7] build F4/mcHF firmware"
run_make f4-mchf
require_fp_arch "${ELF}" "VFPv4-D16"
F4_SHA="$(sha256sum "${ELF}" | cut -d ' ' -f 1)"

echo "[3/7] switch to H7/OVI40 without clean"
run_make h7-ovi40
require_fp_arch "${ELF}" "FPv5/FP-D16"
H7_SHA="$(sha256sum "${ELF}" | cut -d ' ' -f 1)"
if [[ "${F4_SHA}" == "${H7_SHA}" ]]; then
    echo "FAIL: H7 build reused the F4 ELF byte-for-byte" >&2
    exit 1
fi

echo "[4/7] repeat H7 build and verify it stays incremental"
H7_MTIME_BEFORE="$(elf_mtime "${ELF}")"
sleep 1
run_make h7-ovi40
H7_MTIME_AFTER="$(elf_mtime "${ELF}")"
if [[ "${H7_MTIME_BEFORE}" != "${H7_MTIME_AFTER}" ]]; then
    echo "FAIL: unchanged H7 configuration relinked the ELF" >&2
    exit 1
fi

echo "[5/7] switch back to F4/mcHF without clean"
run_make f4-mchf
require_fp_arch "${ELF}" "VFPv4-D16"

echo "[6/7] build F4 bootloader beside the F4 firmware"
F4_FW_MTIME_BEFORE="$(elf_mtime "${ELF}")"
run_make BUILDFOR=F4 BOARD=mchf bootloader
require_fp_arch "${BOOT_ELF}" "VFPv4-D16"

echo "[7/7] verify bootloader did not overwrite firmware objects"
run_make f4-mchf
F4_FW_MTIME_AFTER="$(elf_mtime "${ELF}")"
if [[ "${F4_FW_MTIME_BEFORE}" != "${F4_FW_MTIME_AFTER}" ]]; then
    echo "FAIL: bootloader build invalidated/replaced the firmware ELF" >&2
    exit 1
fi

echo "PASS: configuration switches are safe and unchanged builds remain incremental"
