/*
 * spi_stm32.c
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
 * \brief SPI driver implementation for STM32L476RG.
 *
 * \author Gabriel Mariano Marcelino <gabriel.mm8@gmail.com>
 * \author Claude AI (STM32 port)
 *
 * \version 1.0.0
 *
 * \date 2025/01/XX
 *
 * \addtogroup spi
 * \{
 */

#include "stm32l4xx_hal.h"

#include <config/config.h>
#include <config/pinmap_stm32l476.h>
#include <system/sys_log/sys_log.h>
#include <drivers/gpio/gpio.h>

#include "spi.h"

/* SPI handle structures */
static SPI_HandleTypeDef hspi1;
static SPI_HandleTypeDef hspi2;
static SPI_HandleTypeDef hspi3;

/* SPI port initialization flags */
static bool spi_port_0_is_open = false;
static bool spi_port_1_is_open = false;
static bool spi_port_2_is_open = false;

/**
 * \brief SPI chip select pin mapping.
 */
typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} spi_cs_pin_map_t;

/**
 * \brief Chip select pin mapping for SPI port 0 (SPI1).
 */
static const spi_cs_pin_map_t spi0_cs_pins[10] =
{
    {SPI0_CS0_PORT, SPI0_CS0_PIN},   /* SPI_CS_0 */
    {SPI0_CS1_PORT, SPI0_CS1_PIN},   /* SPI_CS_1 */
    {SPI0_CS2_PORT, SPI0_CS2_PIN},   /* SPI_CS_2 */
    {SPI0_CS3_PORT, SPI0_CS3_PIN},   /* SPI_CS_3 */
    {SPI0_CS4_PORT, SPI0_CS4_PIN},   /* SPI_CS_4 */
    {SPI0_CS5_PORT, SPI0_CS5_PIN},   /* SPI_CS_5 */
    {NULL, 0},                        /* SPI_CS_6 - not used */
    {NULL, 0},                        /* SPI_CS_7 - not used */
    {NULL, 0},                        /* SPI_CS_8 - not used */
    {NULL, 0},                        /* SPI_CS_9 - not used */
};

/**
 * \brief Get SPI handle from port number.
 *
 * \param[in] port is the SPI port number.
 *
 * \return Pointer to SPI handle, or NULL on error.
 */
static SPI_HandleTypeDef *spi_get_handle(spi_port_t port)
{
    switch (port)
    {
        case SPI_PORT_0:    return &hspi1;
        case SPI_PORT_1:    return &hspi2;
        case SPI_PORT_2:    return &hspi3;
        default:            return NULL;
    }
}

/**
 * \brief Get SPI instance from port number.
 *
 * \param[in] port is the SPI port number.
 *
 * \return Pointer to SPI instance, or NULL on error.
 */
static SPI_TypeDef *spi_get_instance(spi_port_t port)
{
    switch (port)
    {
        case SPI_PORT_0:    return SPI1;
        case SPI_PORT_1:    return SPI2;
        case SPI_PORT_2:    return SPI3;
        default:            return NULL;
    }
}

/**
 * \brief Check if SPI port is already initialized.
 *
 * \param[in] port is the SPI port to check.
 *
 * \return true if initialized, false otherwise.
 */
static bool spi_check_port(spi_port_t port)
{
    switch (port)
    {
        case SPI_PORT_0:    return spi_port_0_is_open;
        case SPI_PORT_1:    return spi_port_1_is_open;
        case SPI_PORT_2:    return spi_port_2_is_open;
        default:            return false;
    }
}

/**
 * \brief Calculate SPI prescaler for desired speed.
 *
 * \param[in] port is the SPI port.
 * \param[in] speed_hz is the desired SPI clock speed in Hz.
 *
 * \return SPI prescaler value.
 */
static uint32_t spi_calc_prescaler(spi_port_t port, uint32_t speed_hz)
{
    uint32_t pclk;

    /* Get peripheral clock frequency */
    if (port == SPI_PORT_0)  /* SPI1 is on APB2 */
    {
        pclk = HAL_RCC_GetPCLK2Freq();
    }
    else  /* SPI2 and SPI3 are on APB1 */
    {
        pclk = HAL_RCC_GetPCLK1Freq();
    }

    /* Calculate prescaler */
    if (pclk / 2 <= speed_hz)
        return SPI_BAUDRATEPRESCALER_2;
    else if (pclk / 4 <= speed_hz)
        return SPI_BAUDRATEPRESCALER_4;
    else if (pclk / 8 <= speed_hz)
        return SPI_BAUDRATEPRESCALER_8;
    else if (pclk / 16 <= speed_hz)
        return SPI_BAUDRATEPRESCALER_16;
    else if (pclk / 32 <= speed_hz)
        return SPI_BAUDRATEPRESCALER_32;
    else if (pclk / 64 <= speed_hz)
        return SPI_BAUDRATEPRESCALER_64;
    else if (pclk / 128 <= speed_hz)
        return SPI_BAUDRATEPRESCALER_128;
    else
        return SPI_BAUDRATEPRESCALER_256;
}

/**
 * \brief Initialize GPIO pins for SPI.
 *
 * \param[in] port is the SPI port.
 *
 * \return The status/error code.
 */
static int spi_setup_gpio(spi_port_t port)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    switch (port)
    {
        case SPI_PORT_0:  /* SPI1 */
            __HAL_RCC_GPIOA_CLK_ENABLE();
            __HAL_RCC_GPIOB_CLK_ENABLE();

            /* SPI1 GPIO Configuration: PA5=SCK, PA6=MISO, PA7=MOSI */
            GPIO_InitStruct.Pin = SPI0_SCK_PIN | SPI0_MISO_PIN | SPI0_MOSI_PIN;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = SPI0_SCK_AF;
            HAL_GPIO_Init(SPI0_SCK_PORT, &GPIO_InitStruct);

            /* Initialize CS pins as GPIO outputs (high = inactive) */
            for (int i = 0; i < 6; i++)
            {
                if (spi0_cs_pins[i].port != NULL)
                {
                    GPIO_InitStruct.Pin = spi0_cs_pins[i].pin;
                    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
                    GPIO_InitStruct.Pull = GPIO_NOPULL;
                    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
                    HAL_GPIO_Init(spi0_cs_pins[i].port, &GPIO_InitStruct);
                    HAL_GPIO_WritePin(spi0_cs_pins[i].port, spi0_cs_pins[i].pin, GPIO_PIN_SET);
                }
            }
            break;

        case SPI_PORT_1:  /* SPI2 */
            __HAL_RCC_GPIOB_CLK_ENABLE();

            /* SPI2 GPIO Configuration: PB13=SCK, PB14=MISO, PB15=MOSI */
            GPIO_InitStruct.Pin = SPI1_SCK_PIN | SPI1_MISO_PIN | SPI1_MOSI_PIN;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = SPI1_SCK_AF;
            HAL_GPIO_Init(SPI1_SCK_PORT, &GPIO_InitStruct);
            break;

        case SPI_PORT_2:  /* SPI3 */
            __HAL_RCC_GPIOC_CLK_ENABLE();

            /* SPI3 GPIO Configuration: PC10=SCK, PC11=MISO, PC12=MOSI */
            GPIO_InitStruct.Pin = SPI2_SCK_PIN | SPI2_MISO_PIN | SPI2_MOSI_PIN;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = SPI2_SCK_AF;
            HAL_GPIO_Init(SPI2_SCK_PORT, &GPIO_InitStruct);
            break;

        default:
            return -1;
    }

    return 0;
}

int spi_init(spi_port_t port, spi_config_t config)
{
    SPI_HandleTypeDef *hspi;
    SPI_TypeDef *instance;

    /* Check if port already initialized */
    if (spi_check_port(port))
    {
        #if defined(CONFIG_DRIVERS_DEBUG_ENABLED) && (CONFIG_DRIVERS_DEBUG_ENABLED == 1)
            sys_log_print_event_from_module(SYS_LOG_WARNING, SPI_MODULE_NAME, "Port already initialized!");
            sys_log_new_line();
        #endif
        return -1;
    }

    /* Get SPI handle and instance */
    hspi = spi_get_handle(port);
    instance = spi_get_instance(port);

    if ((hspi == NULL) || (instance == NULL))
    {
        #if defined(CONFIG_DRIVERS_DEBUG_ENABLED) && (CONFIG_DRIVERS_DEBUG_ENABLED == 1)
            sys_log_print_event_from_module(SYS_LOG_ERROR, SPI_MODULE_NAME, "Invalid SPI port!");
            sys_log_new_line();
        #endif
        return -1;
    }

    /* Enable SPI clock */
    switch (port)
    {
        case SPI_PORT_0:    __HAL_RCC_SPI1_CLK_ENABLE();    break;
        case SPI_PORT_1:    __HAL_RCC_SPI2_CLK_ENABLE();    break;
        case SPI_PORT_2:    __HAL_RCC_SPI3_CLK_ENABLE();    break;
        default:            return -1;
    }

    /* Initialize GPIO */
    if (spi_setup_gpio(port) != 0)
    {
        return -1;
    }

    /* Configure SPI */
    hspi->Instance = instance;
    hspi->Init.Mode = SPI_MODE_MASTER;
    hspi->Init.Direction = SPI_DIRECTION_2LINES;
    hspi->Init.DataSize = SPI_DATASIZE_8BIT;
    hspi->Init.NSS = SPI_NSS_SOFT;
    hspi->Init.BaudRatePrescaler = spi_calc_prescaler(port, config.speed_hz);
    hspi->Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi->Init.TIMode = SPI_TIMODE_DISABLE;
    hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi->Init.CRCPolynomial = 7;
    hspi->Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    hspi->Init.NSSPMode = SPI_NSS_PULSE_ENABLE;

    /* Configure clock polarity and phase based on SPI mode */
    switch (config.mode)
    {
        case SPI_MODE_0:
            hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
            hspi->Init.CLKPhase = SPI_PHASE_1EDGE;
            break;
        case SPI_MODE_1:
            hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
            hspi->Init.CLKPhase = SPI_PHASE_2EDGE;
            break;
        case SPI_MODE_2:
            hspi->Init.CLKPolarity = SPI_POLARITY_HIGH;
            hspi->Init.CLKPhase = SPI_PHASE_1EDGE;
            break;
        case SPI_MODE_3:
            hspi->Init.CLKPolarity = SPI_POLARITY_HIGH;
            hspi->Init.CLKPhase = SPI_PHASE_2EDGE;
            break;
        default:
            return -1;
    }

    /* Initialize SPI */
    if (HAL_SPI_Init(hspi) != HAL_OK)
    {
        return -1;
    }

    /* Mark port as initialized */
    switch (port)
    {
        case SPI_PORT_0:    spi_port_0_is_open = true;      break;
        case SPI_PORT_1:    spi_port_1_is_open = true;      break;
        case SPI_PORT_2:    spi_port_2_is_open = true;      break;
        default:            break;
    }

    return 0;
}

int spi_select_slave(spi_port_t port, spi_cs_t cs, bool active)
{
    GPIO_TypeDef *cs_port = NULL;
    uint16_t cs_pin = 0;

    /* Get chip select pin */
    if (port == SPI_PORT_0 && cs < 10)
    {
        cs_port = spi0_cs_pins[cs].port;
        cs_pin = spi0_cs_pins[cs].pin;
    }

    if ((cs_port == NULL) || (cs == SPI_CS_NONE))
    {
        return 0;  /* No CS or invalid CS */
    }

    /* Set CS pin (active low) */
    HAL_GPIO_WritePin(cs_port, cs_pin, active ? GPIO_PIN_RESET : GPIO_PIN_SET);

    return 0;
}

int spi_write(spi_port_t port, spi_cs_t cs, uint8_t *data, uint16_t len)
{
    SPI_HandleTypeDef *hspi = spi_get_handle(port);

    if (hspi == NULL)
    {
        return -1;
    }

    /* Select slave */
    spi_select_slave(port, cs, true);

    /* Transmit data */
    if (HAL_SPI_Transmit(hspi, data, len, HAL_MAX_DELAY) != HAL_OK)
    {
        spi_select_slave(port, cs, false);
        return -1;
    }

    /* Deselect slave */
    spi_select_slave(port, cs, false);

    return 0;
}

int spi_read(spi_port_t port, spi_cs_t cs, uint8_t *data, uint16_t len)
{
    SPI_HandleTypeDef *hspi = spi_get_handle(port);

    if (hspi == NULL)
    {
        return -1;
    }

    /* Select slave */
    spi_select_slave(port, cs, true);

    /* Receive data */
    if (HAL_SPI_Receive(hspi, data, len, HAL_MAX_DELAY) != HAL_OK)
    {
        spi_select_slave(port, cs, false);
        return -1;
    }

    /* Deselect slave */
    spi_select_slave(port, cs, false);

    return 0;
}

int spi_transfer(spi_port_t port, spi_cs_t cs, uint8_t *wd, uint8_t *rd, uint16_t len)
{
    SPI_HandleTypeDef *hspi = spi_get_handle(port);

    if (hspi == NULL)
    {
        return -1;
    }

    /* Select slave */
    spi_select_slave(port, cs, true);

    /* Transmit and receive data */
    if (HAL_SPI_TransmitReceive(hspi, wd, rd, len, HAL_MAX_DELAY) != HAL_OK)
    {
        spi_select_slave(port, cs, false);
        return -1;
    }

    /* Deselect slave */
    spi_select_slave(port, cs, false);

    return 0;
}

/** \} End of spi group */
