/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     uhsdr_fault.h                                                   **
 **  Description:   Shared fault handler declarations for F7/H7                     **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __UHSDR_FAULT_H
#define __UHSDR_FAULT_H

#include <stdint.h>

/* Register dumps populated by the fault handlers. */
extern volatile uint32_t fault_r0;
extern volatile uint32_t fault_r1;
extern volatile uint32_t fault_r2;
extern volatile uint32_t fault_r3;
extern volatile uint32_t fault_r12;
extern volatile uint32_t fault_lr;
extern volatile uint32_t fault_pc;
extern volatile uint32_t fault_psr;
extern volatile uint32_t fault_cfsr;
extern volatile uint32_t fault_bfar;
extern volatile uint32_t fault_r5;

__attribute__((naked)) void FaultHandler_Common(void);

#endif /* __UHSDR_FAULT_H */
