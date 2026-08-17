/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     softdds.h                                                       **
 **  Description:   Minimal stub softdds for host unit testing                      **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __SOFTDDS_H
#define __SOFTDDS_H

#include <stdint.h>

/* Minimal DDS types for testing */
typedef struct {
    float32_t phase;
    float32_t freq;
} dds_t;

#endif /* __SOFTDDS_H */
