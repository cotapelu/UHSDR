/**
 * @brief Bootloader minimal HAL support functions
 * 
 * Provides required weak function implementations for HAL
 * when USE_FULL_ASSERT is enabled.
 * 
 * This file uses only standard C types to be MCU-agnostic.
 */

#include <stdint.h>
#include <stdio.h>

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    
    /* Infinite loop */
    while (1)
    {
    }
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
#if !defined(STM32H7) && !defined(STM32H743xx)
void Error_Handler(void)
{
    /* User can add his own implementation to report the HAL error return state */
    while (1)
    {
    }
}
#endif

void _Error_Handler(char *file, int line)
{
    (void)file;
    (void)line;
#if !defined(STM32H7) && !defined(STM32H743xx)
    Error_Handler();
#else
    /* On H7, uhsdr_fault.c already provides Error_Handler */
    while (1)
    {
    }
#endif
}