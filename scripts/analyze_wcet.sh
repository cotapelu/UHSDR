#!/bin/bash
#  -*-  mode: sh; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-
####################################################################################
##                                                                                ##
##                                        UHSDR                                  ##
##               a powerful firmware for STM32 based SDR transceivers              ##
##                                                                                ##
##--------------------------------------------------------------------------------##
##                                                                                ##
##  File name:     analyze_wcet.sh                                                ##
##  Description:   Analyze WCET data from stack usage files                       **
##  Last Modified:                                                                **
##  Licence:		GNU GPLv3                                                      **
####################################################################################

set -e

echo "========================================"
echo "UHSDR WCET Analysis"
echo "========================================"

# Parse .su files for stack usage and create WCET budget report
if command -v python3 &> /dev/null; then
    python3 -c "
import os, re, glob

print('\n=== WCET Budget Analysis (from stack usage files) ===\n')

# Find all .su files
su_files = glob.glob('mchf-eclipse/**/*.su', recursive=True)

if not su_files:
    print('No .su files found. Build with -fstack-usage first.')
    exit(0)

# Critical ISR functions to check
critical_funcs = [
    'AudioDriver_I2SCallback',
    'PendSV_Handler',
    'UiDriver_TaskHandler_HighPrioTasks',
    'UiDriver_TaskHandler_MainTasks',
    'AudioNr_HandleNoiseReduction',
    'FreeDv_HandleFreeDv',
    'RadioManagement_HandlePttOnOff',
]

print('Critical Function Stack Usage:')
print('-' * 80)
print(f'%-40s %10s %10s' % ('Function', 'Stack (bytes)', 'Budget (bytes)'))
print('-' * 80)

found_any = False
for su_file in sorted(su_files):
    with open(su_file, 'r') as f:
        for line in f:
            # Parse: func_name:stack_size:dynamic_size
            parts = line.strip().split(':')
            if len(parts) >= 2:
                func_name = parts[0]
                stack_size = int(parts[1]) if parts[1].isdigit() else 0
                
                for critical in critical_funcs:
                    if critical in func_name:
                        budget = 512  # Default 512 bytes budget for ISR
                        if 'PendSV' in func_name or 'HighPrio' in func_name:
                            budget = 1024
                        elif 'MainTasks' in func_name:
                            budget = 2048
                        
                        status = 'OK'
                        if stack_size > budget:
                            status = 'OVER BUDGET!'
                        
                        print(f'%-40s %10d %10d (%s)' % (func_name[:40], stack_size, budget, status))
                        found_any = True
                        break

if not found_any:
    print('No critical functions found in stack usage files.')
    print('This is expected if -fstack-usage is not enabled.')

print('-' * 80)
print('\nNote: Stack usage is measured in bytes. WCET requires runtime measurement.')
print('Use PROFILE_EVENTS and DWT cycle counter for WCET measurement on target.\n')
" || echo "Python script failed"
else
    echo "Python3 not available, skipping WCET analysis"
fi
