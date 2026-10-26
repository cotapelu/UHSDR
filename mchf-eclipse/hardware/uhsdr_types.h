/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/

#ifndef __MCHF_TYPES_H
#define __MCHF_TYPES_H

#include <stdint.h>
// Guarantee bool == _Bool in *every* translation unit that includes this header.
// Without this, the legacy `typedef int bool` below survives in TUs that have
// NOT already included <stdbool.h>, while TUs that did include it see real
// `_Bool` — LTO then reports ~57 `-Wlto-type-mismatch` ("type 'bool' should match
// type '_Bool'") and any bool-holding struct (e.g. TransceiverState) is read with
// a different layout in different TUs (latent ABI bug). Including <stdbool.h>
// here makes the legacy typedef/defines below dead (bool/true/false become
// macros) and unifies the type everywhere. See docs/TODO.md T67.
#include <stdbool.h>
#include "arm_math.h"
// defines float32_t

#ifndef uchar
typedef	unsigned char	uchar;
#endif

#ifndef ushort
typedef	unsigned short	ushort;
#endif

#ifndef uint
typedef	unsigned int	uint;
#endif

#ifndef ulong
typedef	unsigned long	ulong;
#endif

#ifndef bool
typedef	int				bool;
#endif

#ifndef true
#define true			1
#endif

#ifndef false
#define false			0
#endif

typedef struct {
    float32_t i;
    float32_t q;
} iq_float_t;

typedef enum {
    IQ_TRANS_OFF = 0,
    IQ_TRANS_ON,
    IQ_TRANS_NUM
} iq_trans_idx_t;



#endif
