/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     mock_hal_gpio.h                                                 **
 **  Description:   Mock HAL GPIO for host unit testing                             **
 **  Last Modified: 2026-08-29                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __MOCK_HAL_GPIO_H
#define __MOCK_HAL_GPIO_H

#include "hal_gpio.h"
#include <stdint.h>

/* -------------------------------------------------------------------------
 * Mock GPIO state tracking
 * ------------------------------------------------------------------------- */
#define MOCK_GPIO_MAX_PINS 32

typedef struct {
    uint32_t pin_state;         /* Current output pin state (bitmask) */
    uint32_t pin_mode;          /* Pin mode configuration */
    uint32_t pin_pull;          /* Pull-up/pull-down config */
    uint32_t pin_speed;         /* Speed config */
    uint32_t init_count;        /* Number of times init called */
    uint32_t deinit_count;      /* Number of times deinit called */
} mock_gpio_port_state_t;

/* Global mock state */
extern mock_gpio_port_state_t mock_gpio_ports[];
extern uint32_t mock_gpio_last_read_port;
extern uint32_t mock_gpio_last_read_pin;
extern uint32_t mock_gpio_last_write_port;
extern uint32_t mock_gpio_last_write_pin;
extern hal_gpio_pin_state_t mock_gpio_last_write_state;

/* -------------------------------------------------------------------------
 * Mock helper functions for tests
 * ------------------------------------------------------------------------- */
void mock_gpio_reset(void);
void mock_gpio_set_input(uint32_t port_idx, uint32_t pin);
void mock_gpio_set_output(uint32_t port_idx, uint32_t pin, uint32_t state);
hal_gpio_pin_state_t mock_gpio_get_pin(uint32_t port_idx, uint32_t pin);

/* -------------------------------------------------------------------------
 * Mock HAL GPIO API (mirrors hal_gpio.h)
 * ------------------------------------------------------------------------- */
hal_status_t hal_gpio_init(hal_gpio_port_t port, const hal_gpio_config_t* config);
hal_status_t hal_gpio_deinit(hal_gpio_port_t port, uint32_t pin);
hal_gpio_pin_state_t hal_gpio_read_pin(hal_gpio_port_t port, uint32_t pin);
hal_status_t hal_gpio_write_pin(hal_gpio_port_t port, uint32_t pin, hal_gpio_pin_state_t state);
hal_status_t hal_gpio_toggle_pin(hal_gpio_port_t port, uint32_t pin);

#endif /* __MOCK_HAL_GPIO_H */
