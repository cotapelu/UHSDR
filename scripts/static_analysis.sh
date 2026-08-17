#!/bin/bash
#  -*-  mode: sh; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-
####################################################################################
##                                                                                ##
##                                        UHSDR                                  ##
##               a powerful firmware for STM32 based SDR transceivers              ##
##                                                                                ##
##--------------------------------------------------------------------------------##
##                                                                                ##
##  File name:     static_analysis.sh                                             ##
##  Description:   Run static analysis tools (cppcheck) on product code           ##
##  Last Modified:                                                                ##
##  Licence:		GNU GPLv3                                                      ##
####################################################################################

set -e

echo "========================================"
echo "UHSDR Static Analysis"
echo "========================================"

# Check if cppcheck is installed
if ! command -v cppcheck &> /dev/null; then
    echo "ERROR: cppcheck is not installed"
    echo "Install with: sudo apt-get install cppcheck"
    exit 1
fi

CPPCHECK="cppcheck"
CPPCHECK_FLAGS="--enable=all --std=c99 --inline-suppr \
    -I mchf-eclipse/hardware \
    -I mchf-eclipse/hardware/board_configs \
    -I mchf-eclipse/drivers/audio \
    -I mchf-eclipse/drivers/ui \
    -I mchf-eclipse/drivers/usb \
    -I mchf-eclipse/drivers/freedv \
    -I mchf-eclipse/drivers/cat \
    -I mchf-eclipse/drivers/diag \
    -I mchf-eclipse/misc \
    -I mchf-eclipse/src \
    -I mchf-eclipse/basesw/mcHF/Inc \
    -I mchf-eclipse/basesw/ovi40/Inc \
    -I mchf-eclipse/basesw/ovi40-h7/Inc \
    -DUI_BRD_MCHF -DRF_BRD_MCHF -DCORTEX_M4 -DSTM32F407xx -DUSE_ALTERNATE_NR \
    --suppress=missingIncludeSystem"

echo ""
echo "Running cppcheck on product code..."
echo ""

# Run cppcheck on key directories
$CPPCHECK $CPPCHECK_FLAGS \
    mchf-eclipse/drivers/ \
    mchf-eclipse/hardware/ \
    mchf-eclipse/misc/ \
    mchf-eclipse/src/ \
    2>&1 | tee cppcheck_report.txt || true

echo ""
echo "Static analysis complete."
echo "Report saved to: cppcheck_report.txt"
