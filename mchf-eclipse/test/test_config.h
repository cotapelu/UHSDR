/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_config.h                                                   **
 **  Description:   Minimal board config for host-based unit testing                **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __TEST_CONFIG_H
#define __TEST_CONFIG_H

/* Select mcHF board for testing */
#define UI_BRD_MCHF
#define RF_BRD_MCHF

/* Minimal type definitions for host testing */
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t  uchar;
typedef uint16_t ushort;
typedef uint32_t ulong;
typedef int32_t  long32;

/* Mock ARM math types for host testing */
typedef float float32_t;

/* Stub macros */
#define __MCHF_SPECIALMEM
#define __UHSDR_DMAMEM
#define PI 3.14159265358979323846f

/* Stub functions used by audio modules */
static inline void trace_printf(const char *fmt, ...) { /* no-op for host testing */ }

#endif /* __TEST_CONFIG_H */
