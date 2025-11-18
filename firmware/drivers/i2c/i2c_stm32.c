/*
 * i2c_stm32.c
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
 * \brief I2C driver implementation for STM32L476RG.
 *
 * \author Gabriel Mariano Marcelino <gabriel.mm8@gmail.com>
 * \author Claude AI (STM32 port)
 *
 * \version 1.0.0
 *
 * \date 2025/01/XX
 *
 * \addtogroup i2c
 * \{
 */

#include "stm32l4xx_hal.h"

#include <config/config.h>
#include <config/pinmap_stm32l476.h>
#include <system/sys_log/sys_log.h>

#include "i2c.h"

/* I2C handle structures */
static I2C_HandleTypeDef hi2c1;
static I2C_HandleTypeDef hi2c2;
static I2C_HandleTypeDef hi2c3;

/* I2C port initialization flags */
static bool i2c_port_0_is_open = false;
static bool i2c_port_1_is_open = false;
static bool i2c_port_2_is_open = false;

/**
 * \brief Get I2C handle from port number.
 *
 * \param[in] port is the I2C port number.
 *
 * \return Pointer to I2C handle, or NULL on error.
 */
static I2C_HandleTypeDef *i2c_get_handle(i2c_port_t port)
{
    switch (port)
    {
        case I2C_PORT_0:    return &hi2c1;
        case I2C_PORT_1:    return &hi2c2;
        case I2C_PORT_2:    return &hi2c3;
        default:            return NULL;
    }
}

/**
 * \brief Get I2C instance from port number.
 *
 * \param[in] port is the I2C port number.
 *
 * \return Pointer to I2C instance, or NULL on error.
 */
static I2C_TypeDef *i2c_get_instance(i2c_port_t port)
{
    switch (port)
    {
        case I2C_PORT_0:    return I2C1;
        case I2C_PORT_1:    return I2C2;
        case I2C_PORT_2:    return I2C3;
        default:            return NULL;
    }
}

/**
 * \brief Check if I2C port is already initialized.
 *
 * \param[in] port is the I2C port to check.
 *
 * \return true if initialized, false otherwise.
 */
static bool i2c_check_port(i2c_port_t port)
{
    switch (port)
    {
        case I2C_PORT_0:    return i2c_port_0_is_open;
        case I2C_PORT_1:    return i2c_port_1_is_open;
        case I2C_PORT_2:    return i2c_port_2_is_open;
        default:            return false;
    }
}

/**
 * \brief Calculate I2C timing for desired speed.
 *
 * \param[in] speed_hz is the desired I2C clock speed in Hz.
 *
 * \return I2C timing value.
 *
 * \note These timing values are calculated for PCLK1 = 80 MHz
 *       Using STM32CubeMX I2C timing calculator
 */
static uint32_t i2c_calc_timing(uint32_t speed_hz)
{
    /* Standard Mode (100 kHz) - Rise time = 1000ns, Fall time = 300ns */
    if (speed_hz <= 100000)
    {
        return 0x30A0A7FB;  /* 100 kHz @ 80 MHz PCLK1 */
    }
    /* Fast Mode (400 kHz) - Rise time = 300ns, Fall time = 300ns */
    else if (speed_hz <= 400000)
    {
        return 0x10909CEC;  /* 400 kHz @ 80 MHz PCLK1 */
    }
    /* Fast Mode Plus (1 MHz) - Rise time = 120ns, Fall time = 120ns */
    else
    {
        return 0x00702991;  /* 1 MHz @ 80 MHz PCLK1 */
    }
}

/**
 * \brief Initialize GPIO pins for I2C.
 *
 * \param[in] port is the I2C port.
 *
 * \return The status/error code.
 */
static int i2c_setup_gpio(i2c_port_t port)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    switch (port)
    {
        case I2C_PORT_0:  /* I2C1 */
            __HAL_RCC_GPIOB_CLK_ENABLE();

            /* I2C1 GPIO Configuration: PB6=SCL, PB7=SDA */
            GPIO_InitStruct.Pin = I2C0_SCL_PIN | I2C0_SDA_PIN;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = I2C0_SCL_AF;
            HAL_GPIO_Init(I2C0_SCL_PORT, &GPIO_InitStruct);
            break;

        case I2C_PORT_1:  /* I2C2 */
            __HAL_RCC_GPIOB_CLK_ENABLE();

            /* I2C2 GPIO Configuration: PB10=SCL, PB11=SDA */
            GPIO_InitStruct.Pin = I2C1_SCL_PIN | I2C1_SDA_PIN;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = I2C1_SCL_AF;
            HAL_GPIO_Init(I2C1_SCL_PORT, &GPIO_InitStruct);
            break;

        case I2C_PORT_2:  /* I2C3 */
            __HAL_RCC_GPIOC_CLK_ENABLE();

            /* I2C3 GPIO Configuration: PC0=SCL, PC1=SDA */
            GPIO_InitStruct.Pin = I2C2_SCL_PIN | I2C2_SDA_PIN;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = I2C2_SCL_AF;
            HAL_GPIO_Init(I2C2_SCL_PORT, &GPIO_InitStruct);
            break;

        default:
            return -1;
    }

    return 0;
}

int i2c_init(i2c_port_t port, i2c_config_t config)
{
    I2C_HandleTypeDef *hi2c;
    I2C_TypeDef *instance;

    /* Check if port already initialized */
    if (i2c_check_port(port))
    {
        #if defined(CONFIG_DRIVERS_DEBUG_ENABLED) && (CONFIG_DRIVERS_DEBUG_ENABLED == 1)
            sys_log_print_event_from_module(SYS_LOG_WARNING, I2C_MODULE_NAME, "Port already initialized!");
            sys_log_new_line();
        #endif
        return -1;
    }

    /* Get I2C handle and instance */
    hi2c = i2c_get_handle(port);
    instance = i2c_get_instance(port);

    if ((hi2c == NULL) || (instance == NULL))
    {
        #if defined(CONFIG_DRIVERS_DEBUG_ENABLED) && (CONFIG_DRIVERS_DEBUG_ENABLED == 1)
            sys_log_print_event_from_module(SYS_LOG_ERROR, I2C_MODULE_NAME, "Invalid I2C port!");
            sys_log_new_line();
        #endif
        return -1;
    }

    /* Enable I2C clock */
    switch (port)
    {
        case I2C_PORT_0:    __HAL_RCC_I2C1_CLK_ENABLE();    break;
        case I2C_PORT_1:    __HAL_RCC_I2C2_CLK_ENABLE();    break;
        case I2C_PORT_2:    __HAL_RCC_I2C3_CLK_ENABLE();    break;
        default:            return -1;
    }

    /* Initialize GPIO */
    if (i2c_setup_gpio(port) != 0)
    {
        return -1;
    }

    /* Configure I2C */
    hi2c->Instance = instance;
    hi2c->Init.Timing = i2c_calc_timing(config.speed_hz);
    hi2c->Init.OwnAddress1 = 0;
    hi2c->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c->Init.OwnAddress2 = 0;
    hi2c->Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    /* Initialize I2C */
    if (HAL_I2C_Init(hi2c) != HAL_OK)
    {
        return -1;
    }

    /* Configure Analog filter */
    if (HAL_I2CEx_ConfigAnalogFilter(hi2c, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        return -1;
    }

    /* Configure Digital filter */
    if (HAL_I2CEx_ConfigDigitalFilter(hi2c, 0) != HAL_OK)
    {
        return -1;
    }

    /* Mark port as initialized */
    switch (port)
    {
        case I2C_PORT_0:    i2c_port_0_is_open = true;      break;
        case I2C_PORT_1:    i2c_port_1_is_open = true;      break;
        case I2C_PORT_2:    i2c_port_2_is_open = true;      break;
        default:            break;
    }

    return 0;
}

int i2c_write(i2c_port_t port, i2c_slave_adr_t adr, uint8_t *data, uint16_t len)
{
    I2C_HandleTypeDef *hi2c = i2c_get_handle(port);

    if (hi2c == NULL)
    {
        return -1;
    }

    /* Transmit data to slave */
    if (HAL_I2C_Master_Transmit(hi2c, (uint16_t)(adr << 1), data, len, I2C_SLAVE_TIMEOUT) != HAL_OK)
    {
        return -1;
    }

    return 0;
}

int i2c_read(i2c_port_t port, i2c_slave_adr_t adr, uint8_t *data, uint16_t len)
{
    I2C_HandleTypeDef *hi2c = i2c_get_handle(port);

    if (hi2c == NULL)
    {
        return -1;
    }

    /* Receive data from slave */
    if (HAL_I2C_Master_Receive(hi2c, (uint16_t)(adr << 1), data, len, I2C_SLAVE_TIMEOUT) != HAL_OK)
    {
        return -1;
    }

    return 0;
}

/** \} End of i2c group */
