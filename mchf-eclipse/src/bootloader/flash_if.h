/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/**
  ******************************************************************************
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    28-October-2011
  * @brief   Header file for flash_layer.c
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _FLASH_ACCESS_LAYER_H
#define _FLASH_ACCESS_LAYER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "uhsdr_mcu.h"
#include "hal_flash.h"

/* Exported types ------------------------------------------------------------*/
typedef  void (*pFunction)(void);

/* Result of a read-out-protection query. The values match the historic
 * FlagStatus/RESET/SET semantics (0 = not protected). */
typedef enum {
    FLASH_FLAG_RESET = 0,
    FLASH_FLAG_SET = 1
} flash_flag_status_t;
/* Exported constants --------------------------------------------------------*/
/* extern const char* version;
extern const char* author; */

#ifndef STM32H7
    /* Define the flash memory start address */
    #define USER_FLASH_STARTADDRESS    ((uint32_t)0x08000000) /* Flash Start Address */

    /* End of the Flash address for the largest device, dynamically sized down based on real processor flash */
    #define USER_FLASH_END_ADDRESS     ((uint32_t)0x081FFFFF)

    #define MCHF_FLASHRESERVED      (64)
#else
    /* Define the flash memory start address */
    #define USER_FLASH_STARTADDRESS    ((uint32_t)0x08000000) /* Flash Start Address */

/* End of the Flash address for the largest device, dynamically sized down based on real processor flash */
    #define USER_FLASH_END_ADDRESS     ((uint32_t)0x081FFFFF)

    #define MCHF_FLASHRESERVED      (384)
#endif

/* Define the address from where user application will be loaded. */
#define APPLICATION_ADDRESS        (USER_FLASH_STARTADDRESS + (MCHF_FLASHRESERVED * 1024))


/**
 * @brief real user flash size of microprocessor (flash size - bootloader/config flash size)
 * @returns size of user flash in bytes
 */
static inline uint32_t flashIf_userFlashSize()  {  return ((STM32_GetFlashSize() - MCHF_FLASHRESERVED)*1024); }


/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void flashIf_FlashUnlock(void);
void flashIf_FlashLock(void);
flash_flag_status_t flashIf_ReadOutProtectionStatus(void);
hal_status_t flashIf_EraseSectors(uint32_t Address, uint32_t Length);
hal_status_t flashIf_Program256Bit(uint32_t Address, uint32_t Data[HAL_FLASH_WORD_BYTES]);


#ifdef __cplusplus
}
#endif

#endif  /* _FLASH_IF_H */

/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE******/


