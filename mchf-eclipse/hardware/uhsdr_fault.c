/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     uhsdr_fault.c                                                   **
 **  Description:   Shared fault handler helpers for F7/H7                          **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#include "uhsdr_board.h"
#include "uhsdr_fault.h"

#if defined(STM32H7)
/* H7 HAL headers define Error_Handler as a macro to _Error_Handler, but some
   product code and LTO builds still require a real Error_Handler symbol. */
#undef Error_Handler
#endif

/* Provide a weak Error_Handler fallback for all builds. The CubeMX-generated
   main.c in basesw/ provides a strong definition for both firmware and
   bootloader on F4/F7, so the linker uses that instead. With LTO and
   --gc-sections this guarantees a real symbol always exists without causing
   multiple-definition errors. */
__attribute__((weak)) void Error_Handler(void)
{
    /* User can add his own implementation to report the HAL error return state */
    /* T25.3: intentional halt — this function never returns;
       __builtin_unreachable() documents the unreachable path to the compiler
       and prevents it from inlining cleanup callers’ epilogues into this path. */
    while (1)
    {
    }
    __builtin_unreachable(); /* should never execute; informs optimizer and static analysis */
}

/* These are volatile to try and prevent the compiler/linker optimising them
   away as the variables never actually get used.  If the debugger won't show the
   values of the variables, make them global by moving their declaration outside
   of this function. */
volatile uint32_t fault_r0 __attribute__ ((unused));
volatile uint32_t fault_r1 __attribute__ ((unused));
volatile uint32_t fault_r2 __attribute__ ((unused));
volatile uint32_t fault_r3 __attribute__ ((unused));
volatile uint32_t fault_r12 __attribute__ ((unused));
volatile uint32_t fault_lr __attribute__ ((unused));
volatile uint32_t fault_pc __attribute__ ((unused));
volatile uint32_t fault_psr __attribute__ ((unused));
volatile uint32_t fault_cfsr __attribute__ ((unused));
volatile uint32_t fault_bfar __attribute__ ((unused));
volatile uint32_t fault_r5 __attribute__ ((unused));

static void Debug_FaultGetRegistersFromStack(uint32_t *pulFaultStackAddress, uint32_t r5x)
{
    fault_r0 = pulFaultStackAddress[0];
    fault_r1 = pulFaultStackAddress[1];
    fault_r2 = pulFaultStackAddress[2];
    fault_r3 = pulFaultStackAddress[3];

    fault_r12 = pulFaultStackAddress[4];
    fault_lr = pulFaultStackAddress[5];
    fault_pc = pulFaultStackAddress[6];
    fault_psr = pulFaultStackAddress[7];

    fault_cfsr = SCB->CFSR;
    fault_bfar = SCB->BFAR;

    /* Keep a copy of R5 in a dedicated global so it is visible in the debugger. */
    fault_r5 = r5x;

    /* Signal fault state via LEDs and then halt. */
    Board_GreenLed(LED_STATE_OFF);
    Board_RedLed(LED_STATE_ON);

    while (1)
    {
        /* Stay here for debugger attach. */
    }
}

__attribute__((naked)) void FaultHandler_Common(void)
{
    __asm volatile(
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " mov r1, r5                                                \n"
        " ldr r2, [r0, #24]                                         \n"
        " mov r3, %0                                                \n"
        " bx r3                                                     \n"
        : : "l" (Debug_FaultGetRegistersFromStack)
    );
}
