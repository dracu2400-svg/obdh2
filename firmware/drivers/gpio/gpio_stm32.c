/*
 * gpio_stm32.c
 *
 * Copyright (C) 2021, SpaceLab.
 *
 * This file is part of OBDH 2.0.
 *
 * OBDH 2.0 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OBDH 2.0 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OBDH 2.0. If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * \brief GPIO driver implementation for STM32L476RG.
 *
 * \author Gabriel Mariano Marcelino <gabriel.mm8@gmail.com>
 * \author Claude AI (STM32 port)
 *
 * \version 1.0.0
 *
 * \date 2025/01/XX
 *
 * \addtogroup gpio
 * \{
 */

#include "stm32l4xx_hal.h"

#include <config/config.h>
#include <system/sys_log/sys_log.h>

#include "gpio.h"

/**
 * \brief GPIO pin mapping structure for STM32.
 */
typedef struct
{
    GPIO_TypeDef *port;     /**< STM32 GPIO port (GPIOA, GPIOB, etc.) */
    uint16_t pin;           /**< STM32 GPIO pin number (GPIO_PIN_0, etc.) */
} stm32_gpio_map_t;

/**
 * \brief Mapping table from generic GPIO_PIN_X to STM32 GPIO ports and pins.
 *
 * This table maps 70 generic GPIO pins to specific STM32L476RG pins.
 * Adjust this mapping based on your hardware design.
 */
static const stm32_gpio_map_t gpio_pin_map[70] =
{
    /* GPIO_PIN_0 to GPIO_PIN_15 -> GPIOA pins */
    {GPIOA, GPIO_PIN_0},    /* GPIO_PIN_0 -> PA0 */
    {GPIOA, GPIO_PIN_1},    /* GPIO_PIN_1 -> PA1 */
    {GPIOA, GPIO_PIN_2},    /* GPIO_PIN_2 -> PA2 */
    {GPIOA, GPIO_PIN_3},    /* GPIO_PIN_3 -> PA3 */
    {GPIOA, GPIO_PIN_4},    /* GPIO_PIN_4 -> PA4 */
    {GPIOA, GPIO_PIN_5},    /* GPIO_PIN_5 -> PA5 */
    {GPIOA, GPIO_PIN_6},    /* GPIO_PIN_6 -> PA6 */
    {GPIOA, GPIO_PIN_7},    /* GPIO_PIN_7 -> PA7 */
    {GPIOA, GPIO_PIN_8},    /* GPIO_PIN_8 -> PA8 */
    {GPIOA, GPIO_PIN_9},    /* GPIO_PIN_9 -> PA9 */
    {GPIOA, GPIO_PIN_10},   /* GPIO_PIN_10 -> PA10 */
    {GPIOA, GPIO_PIN_11},   /* GPIO_PIN_11 -> PA11 */
    {GPIOA, GPIO_PIN_12},   /* GPIO_PIN_12 -> PA12 */
    {GPIOA, GPIO_PIN_13},   /* GPIO_PIN_13 -> PA13 */
    {GPIOA, GPIO_PIN_14},   /* GPIO_PIN_14 -> PA14 */
    {GPIOA, GPIO_PIN_15},   /* GPIO_PIN_15 -> PA15 */

    /* GPIO_PIN_16 to GPIO_PIN_31 -> GPIOB pins */
    {GPIOB, GPIO_PIN_0},    /* GPIO_PIN_16 -> PB0 */
    {GPIOB, GPIO_PIN_1},    /* GPIO_PIN_17 -> PB1 */
    {GPIOB, GPIO_PIN_2},    /* GPIO_PIN_18 -> PB2 */
    {GPIOB, GPIO_PIN_3},    /* GPIO_PIN_19 -> PB3 */
    {GPIOB, GPIO_PIN_4},    /* GPIO_PIN_20 -> PB4 */
    {GPIOB, GPIO_PIN_5},    /* GPIO_PIN_21 -> PB5 */
    {GPIOB, GPIO_PIN_6},    /* GPIO_PIN_22 -> PB6 */
    {GPIOB, GPIO_PIN_7},    /* GPIO_PIN_23 -> PB7 */
    {GPIOB, GPIO_PIN_8},    /* GPIO_PIN_24 -> PB8 */
    {GPIOB, GPIO_PIN_9},    /* GPIO_PIN_25 -> PB9 */
    {GPIOB, GPIO_PIN_10},   /* GPIO_PIN_26 -> PB10 */
    {GPIOB, GPIO_PIN_11},   /* GPIO_PIN_27 -> PB11 */
    {GPIOB, GPIO_PIN_12},   /* GPIO_PIN_28 -> PB12 */
    {GPIOB, GPIO_PIN_13},   /* GPIO_PIN_29 -> PB13 */
    {GPIOB, GPIO_PIN_14},   /* GPIO_PIN_30 -> PB14 */
    {GPIOB, GPIO_PIN_15},   /* GPIO_PIN_31 -> PB15 */

    /* GPIO_PIN_32 to GPIO_PIN_47 -> GPIOC pins */
    {GPIOC, GPIO_PIN_0},    /* GPIO_PIN_32 -> PC0 */
    {GPIOC, GPIO_PIN_1},    /* GPIO_PIN_33 -> PC1 */
    {GPIOC, GPIO_PIN_2},    /* GPIO_PIN_34 -> PC2 */
    {GPIOC, GPIO_PIN_3},    /* GPIO_PIN_35 -> PC3 */
    {GPIOC, GPIO_PIN_4},    /* GPIO_PIN_36 -> PC4 */
    {GPIOC, GPIO_PIN_5},    /* GPIO_PIN_37 -> PC5 */
    {GPIOC, GPIO_PIN_6},    /* GPIO_PIN_38 -> PC6 */
    {GPIOC, GPIO_PIN_7},    /* GPIO_PIN_39 -> PC7 */
    {GPIOC, GPIO_PIN_8},    /* GPIO_PIN_40 -> PC8 */
    {GPIOC, GPIO_PIN_9},    /* GPIO_PIN_41 -> PC9 */
    {GPIOC, GPIO_PIN_10},   /* GPIO_PIN_42 -> PC10 */
    {GPIOC, GPIO_PIN_11},   /* GPIO_PIN_43 -> PC11 */
    {GPIOC, GPIO_PIN_12},   /* GPIO_PIN_44 -> PC12 */
    {GPIOC, GPIO_PIN_13},   /* GPIO_PIN_45 -> PC13 */
    {GPIOC, GPIO_PIN_14},   /* GPIO_PIN_46 -> PC14 */
    {GPIOC, GPIO_PIN_15},   /* GPIO_PIN_47 -> PC15 */

    /* GPIO_PIN_48 to GPIO_PIN_63 -> GPIOD pins */
    {GPIOD, GPIO_PIN_0},    /* GPIO_PIN_48 -> PD0 */
    {GPIOD, GPIO_PIN_1},    /* GPIO_PIN_49 -> PD1 */
    {GPIOD, GPIO_PIN_2},    /* GPIO_PIN_50 -> PD2 */
    {GPIOD, GPIO_PIN_3},    /* GPIO_PIN_51 -> PD3 */
    {GPIOD, GPIO_PIN_4},    /* GPIO_PIN_52 -> PD4 */
    {GPIOD, GPIO_PIN_5},    /* GPIO_PIN_53 -> PD5 */
    {GPIOD, GPIO_PIN_6},    /* GPIO_PIN_54 -> PD6 */
    {GPIOD, GPIO_PIN_7},    /* GPIO_PIN_55 -> PD7 */
    {GPIOD, GPIO_PIN_8},    /* GPIO_PIN_56 -> PD8 */
    {GPIOD, GPIO_PIN_9},    /* GPIO_PIN_57 -> PD9 */
    {GPIOD, GPIO_PIN_10},   /* GPIO_PIN_58 -> PD10 */
    {GPIOD, GPIO_PIN_11},   /* GPIO_PIN_59 -> PD11 */
    {GPIOD, GPIO_PIN_12},   /* GPIO_PIN_60 -> PD12 */
    {GPIOD, GPIO_PIN_13},   /* GPIO_PIN_61 -> PD13 */
    {GPIOD, GPIO_PIN_14},   /* GPIO_PIN_62 -> PD14 */
    {GPIOD, GPIO_PIN_15},   /* GPIO_PIN_63 -> PD15 */

    /* GPIO_PIN_64 to GPIO_PIN_69 -> GPIOE pins */
    {GPIOE, GPIO_PIN_0},    /* GPIO_PIN_64 -> PE0 */
    {GPIOE, GPIO_PIN_1},    /* GPIO_PIN_65 -> PE1 */
    {GPIOE, GPIO_PIN_2},    /* GPIO_PIN_66 -> PE2 */
    {GPIOE, GPIO_PIN_3},    /* GPIO_PIN_67 -> PE3 */
    {GPIOE, GPIO_PIN_4},    /* GPIO_PIN_68 -> PE4 */
    {GPIOE, GPIO_PIN_5},    /* GPIO_PIN_69 -> PE5 */
};

/**
 * \brief Get STM32 GPIO port and pin from generic GPIO pin number.
 *
 * \param[in] pin is the generic GPIO pin number.
 * \param[out] port is a pointer to store the STM32 GPIO port.
 * \param[out] stm32_pin is a pointer to store the STM32 GPIO pin.
 *
 * \return 0 on success, -1 on invalid pin.
 */
static int gpio_get_stm32_mapping(gpio_pin_t pin, GPIO_TypeDef **port, uint16_t *stm32_pin)
{
    if (pin >= 70)
    {
        #if defined(CONFIG_DRIVERS_DEBUG_ENABLED) && (CONFIG_DRIVERS_DEBUG_ENABLED == 1)
            sys_log_print_event_from_module(SYS_LOG_ERROR, GPIO_MODULE_NAME, "Invalid pin number!");
            sys_log_new_line();
        #endif /* CONFIG_DRIVERS_DEBUG_ENABLED */
        return -1;
    }

    *port = gpio_pin_map[pin].port;
    *stm32_pin = gpio_pin_map[pin].pin;

    return 0;
}

/**
 * \brief Enable GPIO clock for a specific port.
 *
 * \param[in] port is the STM32 GPIO port.
 */
static void gpio_enable_clock(GPIO_TypeDef *port)
{
    if (port == GPIOA)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    else if (port == GPIOB)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    else if (port == GPIOC)
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    else if (port == GPIOD)
    {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
    else if (port == GPIOE)
    {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    }
    else if (port == GPIOH)
    {
        __HAL_RCC_GPIOH_CLK_ENABLE();
    }
}

int gpio_init(gpio_pin_t pin, gpio_config_t config)
{
    GPIO_TypeDef *port;
    uint16_t stm32_pin;
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Get STM32 port and pin mapping */
    if (gpio_get_stm32_mapping(pin, &port, &stm32_pin) != 0)
    {
        return -1;
    }

    /* Enable GPIO clock */
    gpio_enable_clock(port);

    /* Configure GPIO pin */
    GPIO_InitStruct.Pin = stm32_pin;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    if (config.mode == GPIO_MODE_OUTPUT)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    }
    else if (config.mode == GPIO_MODE_INPUT)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    }
    else
    {
        #if defined(CONFIG_DRIVERS_DEBUG_ENABLED) && (CONFIG_DRIVERS_DEBUG_ENABLED == 1)
            sys_log_print_event_from_module(SYS_LOG_ERROR, GPIO_MODULE_NAME, "Invalid mode during initialization!");
            sys_log_new_line();
        #endif /* CONFIG_DRIVERS_DEBUG_ENABLED */
        return -1;
    }

    HAL_GPIO_Init(port, &GPIO_InitStruct);

    return 0;
}

int gpio_set_state(gpio_pin_t pin, bool level)
{
    GPIO_TypeDef *port;
    uint16_t stm32_pin;

    /* Get STM32 port and pin mapping */
    if (gpio_get_stm32_mapping(pin, &port, &stm32_pin) != 0)
    {
        return -1;
    }

    /* Set GPIO state */
    HAL_GPIO_WritePin(port, stm32_pin, level ? GPIO_PIN_SET : GPIO_PIN_RESET);

    return 0;
}

int gpio_get_state(gpio_pin_t pin)
{
    GPIO_TypeDef *port;
    uint16_t stm32_pin;

    /* Get STM32 port and pin mapping */
    if (gpio_get_stm32_mapping(pin, &port, &stm32_pin) != 0)
    {
        return -1;
    }

    /* Read GPIO state */
    return (int)HAL_GPIO_ReadPin(port, stm32_pin);
}

int gpio_toggle(gpio_pin_t pin)
{
    GPIO_TypeDef *port;
    uint16_t stm32_pin;

    /* Get STM32 port and pin mapping */
    if (gpio_get_stm32_mapping(pin, &port, &stm32_pin) != 0)
    {
        return -1;
    }

    /* Toggle GPIO state */
    HAL_GPIO_TogglePin(port, stm32_pin);

    return 0;
}

/** \} End of gpio group */
