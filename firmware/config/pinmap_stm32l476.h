/*
 * pinmap_stm32l476.h
 *
 * Copyright The OBDH 2.0 Contributors.
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
 * \brief STM32L476RG Pin mapping for OBDH 2.0
 *
 * \author Port by Claude AI
 *
 * \version 1.0.0
 *
 * \date 2025/01/XX
 *
 * \defgroup pinmap_stm32l476 STM32L476RG Pinmap
 * \{
 */

#ifndef PINMAP_STM32L476_H_
#define PINMAP_STM32L476_H_

#include "stm32l4xx_hal.h"

/* ========================================================================
 * SPI Pin Definitions
 * ======================================================================== */

/* SPI1 - Port 0 (Main SPI Bus) */
#define SPI0_SCK_PORT               GPIOA
#define SPI0_SCK_PIN                GPIO_PIN_5
#define SPI0_SCK_AF                 GPIO_AF5_SPI1

#define SPI0_MISO_PORT              GPIOA
#define SPI0_MISO_PIN               GPIO_PIN_6
#define SPI0_MISO_AF                GPIO_AF5_SPI1

#define SPI0_MOSI_PORT              GPIOA
#define SPI0_MOSI_PIN               GPIO_PIN_7
#define SPI0_MOSI_AF                GPIO_AF5_SPI1

/* SPI1 Chip Select pins */
#define SPI0_CS0_PORT               GPIOB
#define SPI0_CS0_PIN                GPIO_PIN_0

#define SPI0_CS1_PORT               GPIOB
#define SPI0_CS1_PIN                GPIO_PIN_1

#define SPI0_CS2_PORT               GPIOB
#define SPI0_CS2_PIN                GPIO_PIN_2

#define SPI0_CS3_PORT               GPIOB
#define SPI0_CS3_PIN                GPIO_PIN_10

#define SPI0_CS4_PORT               GPIOB
#define SPI0_CS4_PIN                GPIO_PIN_11

#define SPI0_CS5_PORT               GPIOB
#define SPI0_CS5_PIN                GPIO_PIN_12

/* SPI2 - Port 1 */
#define SPI1_SCK_PORT               GPIOB
#define SPI1_SCK_PIN                GPIO_PIN_13
#define SPI1_SCK_AF                 GPIO_AF5_SPI2

#define SPI1_MISO_PORT              GPIOB
#define SPI1_MISO_PIN               GPIO_PIN_14
#define SPI1_MISO_AF                GPIO_AF5_SPI2

#define SPI1_MOSI_PORT              GPIOB
#define SPI1_MOSI_PIN               GPIO_PIN_15
#define SPI1_MOSI_AF                GPIO_AF5_SPI2

/* SPI3 - Port 2 */
#define SPI2_SCK_PORT               GPIOC
#define SPI2_SCK_PIN                GPIO_PIN_10
#define SPI2_SCK_AF                 GPIO_AF6_SPI3

#define SPI2_MISO_PORT              GPIOC
#define SPI2_MISO_PIN               GPIO_PIN_11
#define SPI2_MISO_AF                GPIO_AF6_SPI3

#define SPI2_MOSI_PORT              GPIOC
#define SPI2_MOSI_PIN               GPIO_PIN_12
#define SPI2_MOSI_AF                GPIO_AF6_SPI3

/* ========================================================================
 * I2C Pin Definitions
 * ======================================================================== */

/* I2C1 - Port 0 */
#define I2C0_SCL_PORT               GPIOB
#define I2C0_SCL_PIN                GPIO_PIN_6
#define I2C0_SCL_AF                 GPIO_AF4_I2C1

#define I2C0_SDA_PORT               GPIOB
#define I2C0_SDA_PIN                GPIO_PIN_7
#define I2C0_SDA_AF                 GPIO_AF4_I2C1

/* I2C2 - Port 1 */
#define I2C1_SCL_PORT               GPIOB
#define I2C1_SCL_PIN                GPIO_PIN_10
#define I2C1_SCL_AF                 GPIO_AF4_I2C2

#define I2C1_SDA_PORT               GPIOB
#define I2C1_SDA_PIN                GPIO_PIN_11
#define I2C1_SDA_AF                 GPIO_AF4_I2C2

/* I2C3 - Port 2 */
#define I2C2_SCL_PORT               GPIOC
#define I2C2_SCL_PIN                GPIO_PIN_0
#define I2C2_SCL_AF                 GPIO_AF4_I2C3

#define I2C2_SDA_PORT               GPIOC
#define I2C2_SDA_PIN                GPIO_PIN_1
#define I2C2_SDA_AF                 GPIO_AF4_I2C3

/* ========================================================================
 * UART Pin Definitions
 * ======================================================================== */

/* USART1 - Port 0 (Debug/Console) */
#define UART0_TX_PORT               GPIOA
#define UART0_TX_PIN                GPIO_PIN_9
#define UART0_TX_AF                 GPIO_AF7_USART1

#define UART0_RX_PORT               GPIOA
#define UART0_RX_PIN                GPIO_PIN_10
#define UART0_RX_AF                 GPIO_AF7_USART1

/* USART2 - Port 1 */
#define UART1_TX_PORT               GPIOA
#define UART1_TX_PIN                GPIO_PIN_2
#define UART1_TX_AF                 GPIO_AF7_USART2

#define UART1_RX_PORT               GPIOA
#define UART1_RX_PIN                GPIO_PIN_3
#define UART1_RX_AF                 GPIO_AF7_USART2

/* USART3 - Port 2 */
#define UART2_TX_PORT               GPIOC
#define UART2_TX_PIN                GPIO_PIN_4
#define UART2_TX_AF                 GPIO_AF7_USART3

#define UART2_RX_PORT               GPIOC
#define UART2_RX_PIN                GPIO_PIN_5
#define UART2_RX_AF                 GPIO_AF7_USART3

/* ========================================================================
 * LED Pin Definitions
 * ======================================================================== */

#define LED_SYSTEM_PORT             GPIOA
#define LED_SYSTEM_PIN              GPIO_PIN_0

#define LED_FAULT_PORT              GPIOA
#define LED_FAULT_PIN               GPIO_PIN_1

#define LED_BEACON_PORT             GPIOA
#define LED_BEACON_PIN              GPIO_PIN_4

/* ========================================================================
 * ADC Pin Definitions
 * ======================================================================== */

#define ADC_CURRENT_SENSOR_PORT     GPIOC
#define ADC_CURRENT_SENSOR_PIN      GPIO_PIN_2
#define ADC_CURRENT_SENSOR_CHANNEL  ADC_CHANNEL_3

#define ADC_VOLTAGE_SENSOR_PORT     GPIOC
#define ADC_VOLTAGE_SENSOR_PIN      GPIO_PIN_3
#define ADC_VOLTAGE_SENSOR_CHANNEL  ADC_CHANNEL_4

#define ADC_TEMP_SENSOR_PORT        GPIOA
#define ADC_TEMP_SENSOR_PIN         GPIO_PIN_8
#define ADC_TEMP_SENSOR_CHANNEL     ADC_CHANNEL_9

/* ========================================================================
 * Watchdog and Reset Pin Definitions
 * ======================================================================== */

#define WDT_KICK_PORT               GPIOC
#define WDT_KICK_PIN                GPIO_PIN_6

#define RESET_SUPERVISOR_PORT       GPIOC
#define RESET_SUPERVISOR_PIN        GPIO_PIN_7

/* ========================================================================
 * Antenna Deployment Pin Definitions
 * ======================================================================== */

#define ANTENNA_DEPLOY_1_PORT       GPIOC
#define ANTENNA_DEPLOY_1_PIN        GPIO_PIN_8

#define ANTENNA_DEPLOY_2_PORT       GPIOC
#define ANTENNA_DEPLOY_2_PIN        GPIO_PIN_9

#define ANTENNA_STATUS_PORT         GPIOC
#define ANTENNA_STATUS_PIN          GPIO_PIN_13

/* ========================================================================
 * EPS Interface Pin Definitions
 * ======================================================================== */

#define EPS_POWER_EN_PORT           GPIOC
#define EPS_POWER_EN_PIN            GPIO_PIN_14

#define EPS_FAULT_PORT              GPIOC
#define EPS_FAULT_PIN               GPIO_PIN_15

/* ========================================================================
 * Payload Interface Pin Definitions
 * ======================================================================== */

#define PAYLOAD_EN_PORT             GPIOB
#define PAYLOAD_EN_PIN              GPIO_PIN_3

#define PAYLOAD_STATUS_PORT         GPIOB
#define PAYLOAD_STATUS_PIN          GPIO_PIN_4

/* ========================================================================
 * External Flash and Memory Pin Definitions
 * ======================================================================== */

/* FRAM CS */
#define FRAM_CS_PORT                SPI0_CS0_PORT
#define FRAM_CS_PIN                 SPI0_CS0_PIN

/* NOR Flash CS */
#define NOR_CS_PORT                 SPI0_CS1_PORT
#define NOR_CS_PIN                  SPI0_CS1_PIN

/* NOR Flash Reset */
#define NOR_RESET_PORT              GPIOB
#define NOR_RESET_PIN               GPIO_PIN_5

/* ========================================================================
 * Radio/TTC Pin Definitions (SPI-based)
 * ======================================================================== */

#define RADIO_CS_PORT               SPI0_CS2_PORT
#define RADIO_CS_PIN                SPI0_CS2_PIN

#define RADIO_SDN_PORT              GPIOB
#define RADIO_SDN_PIN               GPIO_PIN_8

#define RADIO_GPIO0_PORT            GPIOB
#define RADIO_GPIO0_PIN             GPIO_PIN_9

#define RADIO_NIRQ_PORT             GPIOA
#define RADIO_NIRQ_PIN              GPIO_PIN_11

/* ========================================================================
 * Helper Macros
 * ======================================================================== */

/**
 * \brief Enable GPIO clock for a port
 */
#define GPIO_CLK_ENABLE(port) \
    do { \
        if ((port) == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE(); \
        else if ((port) == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE(); \
        else if ((port) == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE(); \
        else if ((port) == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE(); \
        else if ((port) == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE(); \
        else if ((port) == GPIOH) __HAL_RCC_GPIOH_CLK_ENABLE(); \
    } while(0)

#endif /* PINMAP_STM32L476_H_ */

/**
 * \} End of pinmap_stm32l476 group
 */
