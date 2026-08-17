#!/bin/bash
#  -*-  mode: sh; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-
####################################################################################
##                                                                                ##
##                                        UHSDR                                  ##
##               a powerful firmware for STM32 based SDR transceivers              ##
##                                                                                ##
##--------------------------------------------------------------------------------##
##                                                                                ##
##  File name:     size_regression.sh                                             ##
##  Description:   Check firmware size for regression detection                   ##
##  Last Modified:                                                                ##
##  Licence:		GNU GPLv3                                                      ##
####################################################################################

set -e

echo "========================================"
echo "UHSDR Size Regression Detection"
echo "========================================"

BUILDLOC="mchf-eclipse/build"
SIZE_FILE="size_report.txt"
BASELINE_FILE="size_baseline.txt"
THRESHOLD=1024  # 1KB regression threshold

# Initialize report
echo "UHSDR Firmware Size Report" > $SIZE_FILE
echo "==========================" >> $SIZE_FILE
echo "" >> $SIZE_FILE
date >> $SIZE_FILE
echo "" >> $SIZE_FILE

# Function to extract size from build output or elf file
check_size() {
    local build_dir=$1
    local config_name=$2
    local elf_file="$build_dir/fw-mchf.elf"
    
    if [ ! -f "$elf_file" ]; then
        echo "WARNING: $elf_file not found, skipping $config_name"
        return
    fi
    
    # Get text, data, bss sizes
    local size_output=$(arm-none-eabi-size $elf_file 2>/dev/null || echo "size tool not available")
    
    if [ "$size_output" = "size tool not available" ]; then
        # Fallback: parse build output or use binary size
        if [ -f "$build_dir/fw-mchf.bin" ]; then
            local bin_size=$(stat -c%s "$build_dir/fw-mchf.bin" 2>/dev/null || echo "unknown")
            echo "$config_name: binary size = $bin_size bytes" >> $SIZE_FILE
        else
            echo "$config_name: size unavailable" >> $SIZE_FILE
        fi
    else
        echo "$config_name:" >> $SIZE_FILE
        echo "$size_output" >> $SIZE_FILE
        echo "" >> $SIZE_FILE
        
        # Extract text size (first number after text)
        local text_size=$(echo "$size_output" | awk 'NR==2 {print $1}')
        if [ -n "$text_size" ] && [ "$text_size" -gt 0 ] 2>/dev/null; then
            echo "  text size: $text_size bytes" >> $SIZE_FILE
        fi
    fi
}

# Check all build configurations
echo "Checking firmware sizes..."
echo ""

check_size "$BUILDLOC/fw-f4" "F4-mcHF"
check_size "$BUILDLOC/fw-f4-small" "F4-mcHF-small"
check_size "$BUILDLOC/fw-f7" "F7-OVI40"
check_size "$BUILDLOC/fw-h7" "H7-OVI40"
check_size "$BUILDLOC/fw-f4-ili9486-480" "F4-ILI9486"
check_size "$BUILDLOC/fw-f7-lapwing" "F7-LAPWING"

echo ""
echo "Size report saved to: $SIZE_FILE"
cat $SIZE_FILE

# Check against baseline if it exists
if [ -f "$BASELINE_FILE" ]; then
    echo ""
    echo "Checking against baseline..."
    
    # Simple regression check: compare text sizes
    while IFS= read -r line; do
        config=$(echo "$line" | cut -d: -f1)
        current_size=$(echo "$line" | grep -oP '\d+' | head -1)
        
        if [ -n "$current_size" ] && [ "$current_size" -gt 0 ] 2>/dev/null; then
            baseline_size=$(grep "^$config:" "$BASELINE_FILE" | grep -oP '\d+' | head -1)
            
            if [ -n "$baseline_size" ] && [ "$baseline_size" -gt 0 ] 2>/dev/null; then
                diff=$((current_size - baseline_size))
                if [ "$diff" -gt "$THRESHOLD" ]; then
                    echo "REGRESSION: $config size increased by $diff bytes (threshold: $THRESHOLD)"
                    exit 1
                elif [ "$diff" -lt 0 ]; then
                    echo "IMPROVEMENT: $config size decreased by $(( -diff )) bytes"
                else
                    echo "OK: $config size change: +$diff bytes"
                fi
            fi
        fi
    done < <(grep -E "^(F4|F7|H7)-" $SIZE_FILE | grep -v "binary size")
    
    echo ""
    echo "Updating baseline with current sizes..."
    cp $SIZE_FILE $BASELINE_FILE
else
    echo ""
    echo "No baseline found. Creating baseline from current build."
    cp $SIZE_FILE $BASELINE_FILE
fi

echo ""
echo "Size regression check complete."
