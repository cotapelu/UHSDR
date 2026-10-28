/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     uhsdr_boot_hw.c                                                 **
 **  Description:   Bootloader board level hardware access (LEDs and buttons)      **
 **                 All vendor HAL access goes through the HAL shim layer.         **
 **  Last Modified: 2026-10-28                                                      **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#include "uhsdr_boot_hw.h"
#include "hal_clock.h"
#include "hal_gpio.h"
#include "hal_nvic.h"


static GPIO_TypeDef* GPIO_PORT[LEDn] = {LEDGREEN_GPIO_PORT, LEDRED_GPIO_PORT, POWER_ON_GPIO_PORT, BACKLIGHT_ON_GPIO_PORT, TP_CS_PIO};
static const uint16_t GPIO_PIN[LEDn] = {LEDGREEN_PIN, LEDRED_PIN, POWER_ON_PIN, BACKLIGHT_ON_PIN, TP_CS};

static GPIO_TypeDef* BUTTON_PORT[BUTTONn] = {BANDM_BUTTON_GPIO_PORT,POWER_BUTTON_GPIO_PORT, BANDP_BUTTON_GPIO_PORT};

static const uint16_t BUTTON_PIN[BUTTONn] = {BANDM_BUTTON_PIN,POWER_BUTTON_PIN, BANDP_BUTTON_PIN};

static const uint8_t BUTTON_IRQn[BUTTONn] = {BANDM_BUTTON_EXTI_IRQn,POWER_BUTTON_EXTI_IRQn,BANDP_BUTTON_EXTI_IRQn};

void mchfBl_PortClock(hal_gpio_port_t port)
{
    /* Enable the GPIO port clock through the HAL shim; the shim knows which
     * RCC bit belongs to the given port on the actual MCU family. */
    (void)hal_clock_gpio_enable(port);
}

void mchfBl_LEDInit(Led_TypeDef Led)
{
    hal_gpio_config_t pinConfig;

    mchfBl_PortClock(GPIO_PORT[Led]);
    /* Enable the GPIO_LED Clock */

    /* Configure the GPIO_LED pin */
    pinConfig.pin   = GPIO_PIN[Led];
    pinConfig.mode  = HAL_GPIO_MODE_OUTPUT_PP;
    pinConfig.pull  = HAL_GPIO_PULL_UP;
    pinConfig.speed = HAL_GPIO_SPEED_HIGH;

    (void)hal_gpio_init(GPIO_PORT[Led], &pinConfig);
}

void mchfBl_PinOn(Led_TypeDef Led)
{
    GPIO_SetBits(GPIO_PORT[Led],GPIO_PIN[Led]);
}

void mchfBl_PinOff(Led_TypeDef Led)
{
    GPIO_ResetBits(GPIO_PORT[Led],GPIO_PIN[Led]);
}

void mchfBl_PinToggle(Led_TypeDef Led)
{
    GPIO_ToggleBits(GPIO_PORT[Led],GPIO_PIN[Led]);
}

void mchfBl_ButtonInit(Button_TypeDef Button, ButtonMode_TypeDef Button_Mode)
{
    hal_gpio_config_t pinConfig;

    mchfBl_PortClock(BUTTON_PORT[Button]);

    /* Configure Button pin as input */
    pinConfig.pin   = BUTTON_PIN[Button];
    pinConfig.mode  = HAL_GPIO_MODE_INPUT;
    pinConfig.pull  = HAL_GPIO_PULL_UP;
    pinConfig.speed = HAL_GPIO_SPEED_HIGH;

    if (Button_Mode == BUTTON_MODE_EXTI)
    {
        /* Configure Button EXTI line */
        pinConfig.mode = HAL_GPIO_MODE_EVT_FALLING;

        /* Enable and set Button EXTI Interrupt to the lowest priority */
        (void)hal_nvic_enable_irq(BUTTON_IRQn[Button]);
        (void)hal_nvic_set_priority(BUTTON_IRQn[Button], 15, 0);
    }
    (void)hal_gpio_init(BUTTON_PORT[Button], &pinConfig);
}

uint32_t mchfBl_ButtonGetState(Button_TypeDef Button)
{
    return (hal_gpio_read_pin(BUTTON_PORT[Button], BUTTON_PIN[Button]) == HAL_GPIO_PIN_RESET) ? 0U : 1U;
}

void mcHF_PowerOff()
{
    hal_gpio_config_t pinConfig;

    pinConfig.pin   = GPIO_PIN[PWR_HOLD];
    pinConfig.mode  = HAL_GPIO_MODE_INPUT;
    pinConfig.pull  = HAL_GPIO_PULL_UP;
    pinConfig.speed = HAL_GPIO_SPEED_HIGH;

    (void)hal_gpio_init(GPIO_PORT[PWR_HOLD], &pinConfig);

    while(1) { asm("nop"); }
    // never reached
}
