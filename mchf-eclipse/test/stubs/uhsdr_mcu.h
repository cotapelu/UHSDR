/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     uhsdr_mcu.h                                                     **
 **  Description:   Minimal MCU abstraction stubs for host unit testing            **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __UHSDR_MCU_H
#define __UHSDR_MCU_H

#include <stdint.h>

/* MCU type enum */
typedef enum {
    CPU_NONE = 0,
    CPU_STM32F4 = 1,
    CPU_STM32F7 = 2,
    CPU_STM32H7 = 3,
} mchf_cpu_t;

/* Current MCU detection */
inline static mchf_cpu_t MchfHW_Cpu(void)
{
    return CPU_STM32F4; /* Default to F4 for host testing */
}

/* GPIO abstraction */
inline static void GPIO_SetBits(GPIO_TypeDef *PORT, uint32_t PINS) { (void)PORT; (void)PINS; }
inline static void GPIO_ResetBits(GPIO_TypeDef *PORT, uint32_t PINS) { (void)PORT; (void)PINS; }
inline static void GPIO_ToggleBits(GPIO_TypeDef *PORT, uint32_t PINS) { (void)PORT; (void)PINS; }

/* Flash size abstraction */
#define STM32_GetFlashSize()    (1024) /* 1MB default */
#define STM32_GetRevision()     (0)
#define STM32_UUID              (0x12345678U)

/* Cache maintenance stubs */
#define SCB_CleanDCache(addr, len)         ((void)(addr), (void)(len))
#define SCB_InvalidateDCache(addr, len)    ((void)(addr), (void)(len))
#define SCB_CleanInvalidateDCache(addr, len) ((void)(addr), (void)(len))

#define DMA_BUFFER_CLEAN(addr, len)        ((void)(addr), (void)(len))
#define DMA_BUFFER_INVALIDATE(addr, len)   ((void)(addr), (void)(len))

/* CPU detection macros */
#if defined(UI_BRD_MCHF)
    #define CORTEX_M4 1
#endif

#endif /* __UHSDR_MCU_H */
