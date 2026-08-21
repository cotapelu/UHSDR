/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_gpio_stm32.c                                                **
 **  Description:   STM32 backend for hal_gpio.h                                    **
 **                 ONLY this file includes vendor HAL GPIO headers.                **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "hal_gpio.h"

#include <stddef.h>

#if defined(STM32F407xx)
#include "stm32f4xx_hal.h"
#elif defined(STM32F767xx)
#include "stm32f7xx_hal.h"
#elif defined(STM32H743xx)
#include "stm32h7xx_hal.h"
#endif

/* -------------------------------------------------------------------------
 * Mapping helpers
 * ------------------------------------------------------------------------- */
static uint32_t hal_gpio_mode_to_hal(hal_gpio_mode_t mode)
{
    switch (mode)
    {
        case HAL_GPIO_MODE_INPUT:
            return GPIO_MODE_INPUT;
        case HAL_GPIO_MODE_OUTPUT_PP:
            return GPIO_MODE_OUTPUT_PP;
        case HAL_GPIO_MODE_AF:
            return GPIO_MODE_AF_PP;
        case HAL_GPIO_MODE_ANALOG:
            return GPIO_MODE_ANALOG;
        default:
            return GPIO_MODE_INPUT;
    }
}

static uint32_t hal_gpio_pull_to_hal(hal_gpio_pull_t pull)
{
    switch (pull)
    {
        case HAL_GPIO_PULL_NONE:
            return GPIO_NOPULL;
        case HAL_GPIO_PULL_UP:
            return GPIO_PULLUP;
        case HAL_GPIO_PULL_DOWN:
            return GPIO_PULLDOWN;
        default:
            return GPIO_NOPULL;
    }
}

static uint32_t hal_gpio_speed_to_hal(hal_gpio_speed_t speed)
{
    switch (speed)
    {
        case HAL_GPIO_SPEED_LOW:
            return GPIO_SPEED_FREQ_LOW;
        case HAL_GPIO_SPEED_MEDIUM:
            return GPIO_SPEED_FREQ_MEDIUM;
        case HAL_GPIO_SPEED_HIGH:
            return GPIO_SPEED_FREQ_HIGH;
        case HAL_GPIO_SPEED_VERY_HIGH:
            return GPIO_SPEED_FREQ_VERY_HIGH;
        default:
            return GPIO_SPEED_FREQ_LOW;
    }
}

/* -------------------------------------------------------------------------
 * API implementation
 * ------------------------------------------------------------------------- */
hal_status_t hal_gpio_init(hal_gpio_port_t port, const hal_gpio_config_t* config)
{
    if (port == NULL || config == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = config->pin;
    GPIO_InitStruct.Mode = hal_gpio_mode_to_hal(config->mode);
    GPIO_InitStruct.Pull = hal_gpio_pull_to_hal(config->pull);
    GPIO_InitStruct.Speed = hal_gpio_speed_to_hal(config->speed);

    HAL_GPIO_Init((GPIO_TypeDef*)port, &GPIO_InitStruct);

    return HAL_OK;
}

hal_status_t hal_gpio_deinit(hal_gpio_port_t port, uint32_t pin)
{
    if (port == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    HAL_GPIO_DeInit((GPIO_TypeDef*)port, pin);

    return HAL_OK;
}

hal_gpio_pin_state_t hal_gpio_read_pin(hal_gpio_port_t port, uint32_t pin)
{
    GPIO_PinState state = HAL_GPIO_ReadPin((GPIO_TypeDef*)port, pin);
    return (state == GPIO_PIN_SET) ? HAL_GPIO_PIN_SET : HAL_GPIO_PIN_RESET;
}

hal_status_t hal_gpio_write_pin(hal_gpio_port_t port, uint32_t pin, hal_gpio_pin_state_t state)
{
    if (port == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    HAL_GPIO_WritePin((GPIO_TypeDef*)port, pin, (GPIO_PinState)state);

    return HAL_OK;
}

hal_status_t hal_gpio_toggle_pin(hal_gpio_port_t port, uint32_t pin)
{
    if (port == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }

    HAL_GPIO_TogglePin((GPIO_TypeDef*)port, pin);

    return HAL_OK;
}
