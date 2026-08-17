/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     UHSDR_UI_mchf_config.h                                          **
 **  Description:   Minimal stub mcHF board config for host unit testing            **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __UHSDR_UI_MCHF_CONFIG_H
#define __UHSDR_UI_MCHF_CONFIG_H

/* Memory section attributes */
#define __MCHF_SPECIALMEM
#define __UHSDR_DMAMEM

/* Board identification */
#ifndef TRX_NAME
    #define TRX_NAME "mcHF QRP"
#endif
#ifndef TRX_ID
    #define TRX_ID "mchf"
#endif
#ifndef TRX_HW_LIC
    #define TRX_HW_LIC "CC BY-NC-SA 3.0"
    #define TRX_HW_CREATOR "K. Atanassov, M0NKA, www.m0nka.co.uk"
#endif

/* Minimal GPIO definitions for testing */
typedef struct { volatile uint32_t BSRR; } GPIO_TypeDef;
#define GPIO_PIN_0  (1 << 0)
#define GPIO_PIN_1  (1 << 1)
#define GPIO_PIN_2  (1 << 2)
#define GPIO_PIN_3  (1 << 3)
#define GPIO_PIN_4  (1 << 4)
#define GPIO_PIN_5  (1 << 5)
#define GPIO_PIN_6  (1 << 6)
#define GPIO_PIN_7  (1 << 7)
#define GPIO_PIN_8  (1 << 8)
#define GPIO_PIN_9  (1 << 9)
#define GPIO_PIN_10 (1 << 10)
#define GPIO_PIN_11 (1 << 11)
#define GPIO_PIN_12 (1 << 12)
#define GPIO_PIN_13 (1 << 13)
#define GPIO_PIN_14 (1 << 14)
#define GPIO_PIN_15 (1 << 15)

extern GPIO_TypeDef GPIOA, GPIOB, GPIOC, GPIOD, GPIOE;

/* Feature flags */
#define USE_ALTERNATE_NR
#define USE_TWO_CHANNEL_AUDIO
#define USE_HMC1023

#endif /* __UHSDR_UI_MCHF_CONFIG_H */
