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
#ifndef __MCHF_MCU_H
#define __MCHF_MCU_H

#include "uhsdr_board_config.h"

typedef enum {
    CPU_NONE = 0,
    CPU_STM32F4 = 1,
    CPU_STM32F7 = 2,
    CPU_STM32H7 = 3,
} mchf_cpu_t;

inline static mchf_cpu_t MchfHW_Cpu()
{
    mchf_cpu_t retval = CPU_NONE;
#if defined(STM32F4)
    retval = CPU_STM32F4;
#elif  defined(STM32F7)
    retval = CPU_STM32F7;
#elif  defined(STM32H7)
    retval = CPU_STM32H7;
#endif
    return retval;
}

#define STM32_GetRevision()     (*(uint16_t *) (UID_BASE + 2))
#define STM32_GetSignature()    ((*(uint16_t *) (DBGMCU_BASE)) & 0x0FFF)
#define STM32_UUID ((uint32_t *)UID_BASE)

/* -------------------------------------------------------------------------
 * Device identifier and revision from the DBGMCU IDCODE register.
 *
 * This is the MCU abstraction equivalent of the vendor HAL_GetDEVID() /
 * HAL_GetREVID() macros and uses the same register fields on every supported
 * family, so no vendor HAL include is needed in product code.
 * ------------------------------------------------------------------------- */
inline static uint16_t MCU_GetDeviceId(void)
{
    return (uint16_t)((*(uint32_t *)DBGMCU_BASE) & 0x00000FFF);
}

inline static uint16_t MCU_GetRevision(void)
{
    return (uint16_t)((*(uint32_t *)DBGMCU_BASE) >> 16);
}

/* -------------------------------------------------------------------------
 * Remaps address 0x00000000 to the ST system memory (the factory bootloader
 * in ROM) instead of the user flash, so that a jump to 0 jumps into the
 * system bootloader (DFU entry).
 *
 * This is the MCU abstraction equivalent of the vendor
 * __HAL_REMAPMEMORY_SYSTEMFLASH() macro and uses the same register bits on
 * the STM32F4, so no vendor HAL include is needed in product code.
 * ------------------------------------------------------------------------- */
#if defined(STM32F4)
inline static void MCU_JumpToSystemFlash(void)
{
    SYSCFG->MEMRMP &= ~(uint32_t)SYSCFG_MEMRMP_MEM_MODE;
    SYSCFG->MEMRMP |= (uint32_t)SYSCFG_MEMRMP_MEM_MODE_0;
}
#endif

#if defined(STM32F4) || defined(STM32F7)
    #define STM32_GetFlashSize()    (*(uint16_t *) (FLASHSIZE_BASE))
#elif defined(STM32H7)
    #define STM32_GetFlashSize()    (FLASH_SIZE/1024)
    #define FLASHSIZE_BASE 0x1FF1E880
    #define SRAM2_BASE 0x38000000

#endif

#if defined(STM32H7)
    inline static void GPIO_SetBits(GPIO_TypeDef *PORT, uint32_t PINS) { (PORT)->BSRRL = (PINS); }
    inline static void GPIO_ResetBits(GPIO_TypeDef *PORT, uint32_t PINS) { (PORT)->BSRRH = (PINS); }
#elif defined(STM32F7) || defined(STM32F4)
    inline static void GPIO_SetBits(GPIO_TypeDef *PORT, uint32_t PINS) { (PORT)->BSRR = (PINS); }
    inline static void GPIO_ResetBits(GPIO_TypeDef *PORT, uint32_t PINS) { (PORT)->BSRR = (PINS) << 16U; }
#endif

inline static void GPIO_ToggleBits(GPIO_TypeDef *PORT, uint32_t PINS) { (PORT)->ODR ^= (PINS); }
inline static void GPIO_ReadInputDataBit(GPIO_TypeDef *PORT, uint32_t PINS) { (PORT)->IDR = (PINS); }

/* Cache maintenance for DMA buffers on F7/H7 */
#if defined(STM32F7) || defined(STM32H7)
#define DMA_BUFFER_CLEAN(addr, len)      SCB_CleanDCache_by_Addr((uint32_t *)(addr), (len))
#define DMA_BUFFER_INVALIDATE(addr, len) SCB_InvalidateDCache_by_Addr((uint32_t *)(addr), (len))
#else
#define DMA_BUFFER_CLEAN(addr, len)
#define DMA_BUFFER_INVALIDATE(addr, len)
#endif

/* F7/H7 use HAL SPI driver */
#if defined(STM32F7) || defined(STM32H7)
#define USE_SPI_HAL
#endif

/* SPI prescalers for LCD/touchscreen (per-MCU optimum) */
#if defined(STM32F4)
#define SPI_PRESCALE_LCD_DEFAULT (SPI_BAUDRATEPRESCALER_4)
#define SPI_PRESCALE_LCD_HIGH    (SPI_BAUDRATEPRESCALER_2)
#define SPI_PRESCALE_TS_DEFAULT  (SPI_BAUDRATEPRESCALER_64)
#elif defined(STM32F7)
#define SPI_PRESCALE_LCD_DEFAULT (SPI_BAUDRATEPRESCALER_8)
#define SPI_PRESCALE_LCD_HIGH    (SPI_BAUDRATEPRESCALER_4)
#define SPI_PRESCALE_TS_DEFAULT  (SPI_BAUDRATEPRESCALER_128)
#elif defined(STM32H7)
#define SPI_PRESCALE_LCD_DEFAULT (SPI_BAUDRATEPRESCALER_8)
#define SPI_PRESCALE_LCD_HIGH    (SPI_BAUDRATEPRESCALER_4)
#define SPI_PRESCALE_TS_DEFAULT  (SPI_BAUDRATEPRESCALER_32)
#endif

#endif
