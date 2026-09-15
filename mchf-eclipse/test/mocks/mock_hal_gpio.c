/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     mock_hal_gpio.c                                                 **
 **  Description:   Mock HAL GPIO implementation for host unit testing              **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "mock_hal_gpio.h"
#include <string.h>

/* Global mock state */
mock_gpio_port_state_t mock_gpio_ports[MOCK_GPIO_MAX_PINS];
uint32_t mock_gpio_last_read_port;
uint32_t mock_gpio_last_read_pin;
uint32_t mock_gpio_last_write_port;
uint32_t mock_gpio_last_write_pin;
hal_gpio_pin_state_t mock_gpio_last_write_state;

void mock_gpio_reset(void)
{
    memset(mock_gpio_ports, 0, sizeof(mock_gpio_ports));
    mock_gpio_last_read_port = 0;
    mock_gpio_last_read_pin = 0;
    mock_gpio_last_write_port = 0;
    mock_gpio_last_write_pin = 0;
    mock_gpio_last_write_state = HAL_GPIO_PIN_RESET;
}

void mock_gpio_set_input(uint32_t port_idx, uint32_t pin)
{
    if (port_idx < MOCK_GPIO_MAX_PINS)
    {
        mock_gpio_ports[port_idx].pin_mode = HAL_GPIO_MODE_INPUT;
        mock_gpio_ports[port_idx].pin_state &= ~pin;
    }
}

void mock_gpio_set_output(uint32_t port_idx, uint32_t pin, uint32_t state)
{
    if (port_idx < MOCK_GPIO_MAX_PINS)
    {
        mock_gpio_ports[port_idx].pin_mode = HAL_GPIO_MODE_OUTPUT_PP;
        if (state)
        {
            mock_gpio_ports[port_idx].pin_state |= pin;
        }
        else
        {
            mock_gpio_ports[port_idx].pin_state &= ~pin;
        }
    }
}

hal_gpio_pin_state_t mock_gpio_get_pin(uint32_t port_idx, uint32_t pin)
{
    if (port_idx < MOCK_GPIO_MAX_PINS)
    {
        return (mock_gpio_ports[port_idx].pin_state & pin) ? HAL_GPIO_PIN_SET : HAL_GPIO_PIN_RESET;
    }
    return HAL_GPIO_PIN_RESET;
}

hal_status_t hal_gpio_init(hal_gpio_port_t port, const hal_gpio_config_t* config)
{
    uint32_t port_idx = (uint32_t)(uintptr_t)port;
    if (port_idx >= MOCK_GPIO_MAX_PINS || config == NULL)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_gpio_ports[port_idx].pin_mode = config->mode;
    mock_gpio_ports[port_idx].pin_pull = config->pull;
    mock_gpio_ports[port_idx].pin_speed = config->speed;
    mock_gpio_ports[port_idx].init_count++;
    return HAL_STATUS_OK;
}

hal_status_t hal_gpio_deinit(hal_gpio_port_t port, uint32_t pin)
{
    (void)pin;
    uint32_t port_idx = (uint32_t)(uintptr_t)port;
    if (port_idx >= MOCK_GPIO_MAX_PINS)
    {
        return HAL_STATUS_ERR_INVALID_PARAM;
    }
    mock_gpio_ports[port_idx].deinit_count++;
    return HAL_STATUS_OK;
}

hal_gpio_pin_state_t hal_gpio_read_pin(hal_gpio_port_t port, uint32_t pin)
{
    uint32_t port_idx = (uint32_t)(uintptr_t)port;
    mock_gpio_last_read_port = port_idx;
    mock_gpio_last_read_pin = pin;
    if (port_idx < MOCK_GPIO_MAX_PINS)
    {
        return (mock_gpio_ports[port_idx].pin_state & pin) ? HAL_GPIO_PIN_SET : HAL_GPIO_PIN_RESET;
    }
    return HAL_GPIO_PIN_RESET;
}

hal_status_t hal_gpio_write_pin(hal_gpio_port_t port, uint32_t pin, hal_gpio_pin_state_t state)
{
    uint32_t port_idx = (uint32_t)(uintptr_t)port;
    mock_gpio_last_write_port = port_idx;
    mock_gpio_last_write_pin = pin;
    mock_gpio_last_write_state = state;
    if (port_idx < MOCK_GPIO_MAX_PINS)
    {
        if (state == HAL_GPIO_PIN_SET)
        {
            mock_gpio_ports[port_idx].pin_state |= pin;
        }
        else
        {
            mock_gpio_ports[port_idx].pin_state &= ~pin;
        }
        return HAL_STATUS_OK;
    }
    return HAL_STATUS_ERR_INVALID_PARAM;
}

hal_status_t hal_gpio_toggle_pin(hal_gpio_port_t port, uint32_t pin)
{
    uint32_t port_idx = (uint32_t)(uintptr_t)port;
    if (port_idx < MOCK_GPIO_MAX_PINS)
    {
        if (mock_gpio_ports[port_idx].pin_state & pin)
        {
            mock_gpio_ports[port_idx].pin_state &= ~pin;
        }
        else
        {
            mock_gpio_ports[port_idx].pin_state |= pin;
        }
        return HAL_STATUS_OK;
    }
    return HAL_STATUS_ERR_INVALID_PARAM;
}
