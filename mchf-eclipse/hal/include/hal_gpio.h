/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     hal_gpio.h                                                      **
 **  Description:   Abstract GPIO API for STM32 HAL shim layer                      **
 **                 Product code includes this header; NO vendor HAL includes.      **
 **  Last Modified: 2026-08-21                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __HAL_GPIO_H
#define __HAL_GPIO_H

#include "hal_common.h"

/* -------------------------------------------------------------------------
 * GPIO mode
 * ------------------------------------------------------------------------- */
typedef enum {
    HAL_GPIO_MODE_INPUT = 0x00,
    HAL_GPIO_MODE_OUTPUT_PP = 0x01,
    HAL_GPIO_MODE_AF = 0x02,
    HAL_GPIO_MODE_ANALOG = 0x03,
} hal_gpio_mode_t;

/* -------------------------------------------------------------------------
 * GPIO pull-up / pull-down
 * ------------------------------------------------------------------------- */
typedef enum {
    HAL_GPIO_PULL_NONE = 0x00,
    HAL_GPIO_PULL_UP = 0x01,
    HAL_GPIO_PULL_DOWN = 0x02,
} hal_gpio_pull_t;

/* -------------------------------------------------------------------------
 * GPIO speed
 * ------------------------------------------------------------------------- */
typedef enum {
    HAL_GPIO_SPEED_LOW = 0x00,
    HAL_GPIO_SPEED_MEDIUM = 0x01,
    HAL_GPIO_SPEED_HIGH = 0x02,
    HAL_GPIO_SPEED_VERY_HIGH = 0x03,
} hal_gpio_speed_t;

/* -------------------------------------------------------------------------
 * GPIO configuration (vendor-neutral)
 * ------------------------------------------------------------------------- */
typedef struct {
    uint32_t pin;
    hal_gpio_mode_t mode;
    hal_gpio_pull_t pull;
    hal_gpio_speed_t speed;
} hal_gpio_config_t;

/* -------------------------------------------------------------------------
 * Opaque port handle (maps to GPIO_TypeDef* in backend)
 * ------------------------------------------------------------------------- */
typedef void* hal_gpio_port_t;

/* -------------------------------------------------------------------------
 * Pin state
 * ------------------------------------------------------------------------- */
typedef enum {
    HAL_GPIO_PIN_RESET = 0,
    HAL_GPIO_PIN_SET = 1,
} hal_gpio_pin_state_t;

/* -------------------------------------------------------------------------
 * Abstract GPIO API
 * ------------------------------------------------------------------------- */
hal_status_t hal_gpio_init(hal_gpio_port_t port, const hal_gpio_config_t* config);
hal_status_t hal_gpio_deinit(hal_gpio_port_t port, uint32_t pin);
hal_gpio_pin_state_t hal_gpio_read_pin(hal_gpio_port_t port, uint32_t pin);
hal_status_t hal_gpio_write_pin(hal_gpio_port_t port, uint32_t pin, hal_gpio_pin_state_t state);
hal_status_t hal_gpio_toggle_pin(hal_gpio_port_t port, uint32_t pin);

#endif /* __HAL_GPIO_H */
