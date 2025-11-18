/*
 * uart_stm32.c
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
 * \brief UART driver implementation for STM32L476RG.
 *
 * \author Gabriel Mariano Marcelino <gabriel.mm8@gmail.com>
 * \author Miguel Boing <miguelboing13@gmail.com>
 * \author Claude AI (STM32 port)
 *
 * \version 1.0.0
 *
 * \date 2025/01/XX
 *
 * \addtogroup uart
 * \{
 */

#include "stm32l4xx_hal.h"

#include <config/config.h>
#include <config/pinmap_stm32l476.h>
#include <system/sys_log/sys_log.h>

#include "uart.h"

#define UART_RX_BUFFER_SIZE     256

/* UART handle structures */
static UART_HandleTypeDef huart1;
static UART_HandleTypeDef huart2;
static UART_HandleTypeDef huart3;

/* UART port initialization flags */
static bool uart_port_0_is_open = false;
static bool uart_port_1_is_open = false;
static bool uart_port_2_is_open = false;

/* RX buffers */
static uint8_t uart0_rx_buffer[UART_RX_BUFFER_SIZE];
static uint8_t uart1_rx_buffer[UART_RX_BUFFER_SIZE];
static uint8_t uart2_rx_buffer[UART_RX_BUFFER_SIZE];

/* RX buffer pointers */
static volatile uint16_t uart0_rx_head = 0;
static volatile uint16_t uart0_rx_tail = 0;
static volatile uint16_t uart1_rx_head = 0;
static volatile uint16_t uart1_rx_tail = 0;
static volatile uint16_t uart2_rx_head = 0;
static volatile uint16_t uart2_rx_tail = 0;

/* Single byte RX for interrupt */
static uint8_t uart0_rx_byte;
static uint8_t uart1_rx_byte;
static uint8_t uart2_rx_byte;

/**
 * \brief Get UART handle from port number.
 *
 * \param[in] port is the UART port number.
 *
 * \return Pointer to UART handle, or NULL on error.
 */
static UART_HandleTypeDef *uart_get_handle(uart_port_t port)
{
    switch (port)
    {
        case UART_PORT_0:    return &huart1;
        case UART_PORT_1:    return &huart2;
        case UART_PORT_2:    return &huart3;
        default:             return NULL;
    }
}

/**
 * \brief Get UART instance from port number.
 *
 * \param[in] port is the UART port number.
 *
 * \return Pointer to UART instance, or NULL on error.
 */
static USART_TypeDef *uart_get_instance(uart_port_t port)
{
    switch (port)
    {
        case UART_PORT_0:    return USART1;
        case UART_PORT_1:    return USART2;
        case UART_PORT_2:    return USART3;
        default:             return NULL;
    }
}

/**
 * \brief Check if UART port is already initialized.
 *
 * \param[in] port is the UART port to check.
 *
 * \return true if initialized, false otherwise.
 */
static bool uart_check_port(uart_port_t port)
{
    switch (port)
    {
        case UART_PORT_0:    return uart_port_0_is_open;
        case UART_PORT_1:    return uart_port_1_is_open;
        case UART_PORT_2:    return uart_port_2_is_open;
        default:             return false;
    }
}

/**
 * \brief Initialize GPIO pins for UART.
 *
 * \param[in] port is the UART port.
 *
 * \return The status/error code.
 */
static int uart_setup_gpio(uart_port_t port)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    switch (port)
    {
        case UART_PORT_0:  /* USART1 */
            __HAL_RCC_GPIOA_CLK_ENABLE();

            /* USART1 GPIO Configuration: PA9=TX, PA10=RX */
            GPIO_InitStruct.Pin = UART0_TX_PIN;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = UART0_TX_AF;
            HAL_GPIO_Init(UART0_TX_PORT, &GPIO_InitStruct);

            GPIO_InitStruct.Pin = UART0_RX_PIN;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = UART0_RX_AF;
            HAL_GPIO_Init(UART0_RX_PORT, &GPIO_InitStruct);
            break;

        case UART_PORT_1:  /* USART2 */
            __HAL_RCC_GPIOA_CLK_ENABLE();

            /* USART2 GPIO Configuration: PA2=TX, PA3=RX */
            GPIO_InitStruct.Pin = UART1_TX_PIN;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = UART1_TX_AF;
            HAL_GPIO_Init(UART1_TX_PORT, &GPIO_InitStruct);

            GPIO_InitStruct.Pin = UART1_RX_PIN;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = UART1_RX_AF;
            HAL_GPIO_Init(UART1_RX_PORT, &GPIO_InitStruct);
            break;

        case UART_PORT_2:  /* USART3 */
            __HAL_RCC_GPIOC_CLK_ENABLE();

            /* USART3 GPIO Configuration: PC4=TX, PC5=RX */
            GPIO_InitStruct.Pin = UART2_TX_PIN;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = UART2_TX_AF;
            HAL_GPIO_Init(UART2_TX_PORT, &GPIO_InitStruct);

            GPIO_InitStruct.Pin = UART2_RX_PIN;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = UART2_RX_AF;
            HAL_GPIO_Init(UART2_RX_PORT, &GPIO_InitStruct);
            break;

        default:
            return -1;
    }

    return 0;
}

int uart_init(uart_port_t port, uart_config_t config)
{
    UART_HandleTypeDef *huart;
    USART_TypeDef *instance;

    /* Check if port already initialized */
    if (uart_check_port(port))
    {
        #if defined(CONFIG_DRIVERS_DEBUG_ENABLED) && (CONFIG_DRIVERS_DEBUG_ENABLED == 1)
            sys_log_print_event_from_module(SYS_LOG_WARNING, UART_MODULE_NAME, "Port already initialized!");
            sys_log_new_line();
        #endif
        return -1;
    }

    /* Get UART handle and instance */
    huart = uart_get_handle(port);
    instance = uart_get_instance(port);

    if ((huart == NULL) || (instance == NULL))
    {
        #if defined(CONFIG_DRIVERS_DEBUG_ENABLED) && (CONFIG_DRIVERS_DEBUG_ENABLED == 1)
            sys_log_print_event_from_module(SYS_LOG_ERROR, UART_MODULE_NAME, "Invalid UART port!");
            sys_log_new_line();
        #endif
        return -1;
    }

    /* Enable UART clock */
    switch (port)
    {
        case UART_PORT_0:    __HAL_RCC_USART1_CLK_ENABLE();    break;
        case UART_PORT_1:    __HAL_RCC_USART2_CLK_ENABLE();    break;
        case UART_PORT_2:    __HAL_RCC_USART3_CLK_ENABLE();    break;
        default:             return -1;
    }

    /* Initialize GPIO */
    if (uart_setup_gpio(port) != 0)
    {
        return -1;
    }

    /* Configure UART */
    huart->Instance = instance;
    huart->Init.BaudRate = config.baudrate;
    huart->Init.WordLength = (config.data_bits == 8) ? UART_WORDLENGTH_8B : UART_WORDLENGTH_9B;
    huart->Init.StopBits = (config.stop_bits == UART_ONE_STOP_BIT) ? UART_STOPBITS_1 : UART_STOPBITS_2;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;
    huart->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;

    /* Configure parity */
    switch (config.parity)
    {
        case UART_NO_PARITY:
            huart->Init.Parity = UART_PARITY_NONE;
            break;
        case UART_ODD_PARITY:
            huart->Init.Parity = UART_PARITY_ODD;
            break;
        case UART_EVEN_PARITY:
            huart->Init.Parity = UART_PARITY_EVEN;
            break;
        default:
            huart->Init.Parity = UART_PARITY_NONE;
            break;
    }

    huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    /* Initialize UART */
    if (HAL_UART_Init(huart) != HAL_OK)
    {
        return -1;
    }

    /* Mark port as initialized */
    switch (port)
    {
        case UART_PORT_0:    uart_port_0_is_open = true;      break;
        case UART_PORT_1:    uart_port_1_is_open = true;      break;
        case UART_PORT_2:    uart_port_2_is_open = true;      break;
        default:             break;
    }

    return 0;
}

int uart_write(uart_port_t port, uint8_t *data, uint16_t len)
{
    UART_HandleTypeDef *huart = uart_get_handle(port);

    if (huart == NULL)
    {
        return -1;
    }

    /* Transmit data */
    if (HAL_UART_Transmit(huart, data, len, HAL_MAX_DELAY) != HAL_OK)
    {
        return -1;
    }

    return 0;
}

int uart_read(uart_port_t port, uint8_t *data, uint16_t len)
{
    UART_HandleTypeDef *huart = uart_get_handle(port);

    if (huart == NULL)
    {
        return -1;
    }

    /* Receive data */
    if (HAL_UART_Receive(huart, data, len, HAL_MAX_DELAY) != HAL_OK)
    {
        return -1;
    }

    return 0;
}

int uart_rx_enable(uart_port_t port)
{
    UART_HandleTypeDef *huart = uart_get_handle(port);
    uint8_t *rx_byte;

    if (huart == NULL)
    {
        return -1;
    }

    /* Get RX byte pointer for this port */
    switch (port)
    {
        case UART_PORT_0:    rx_byte = &uart0_rx_byte;    break;
        case UART_PORT_1:    rx_byte = &uart1_rx_byte;    break;
        case UART_PORT_2:    rx_byte = &uart2_rx_byte;    break;
        default:             return -1;
    }

    /* Start interrupt-based reception */
    if (HAL_UART_Receive_IT(huart, rx_byte, 1) != HAL_OK)
    {
        return -1;
    }

    return 0;
}

int uart_rx_disable(uart_port_t port)
{
    UART_HandleTypeDef *huart = uart_get_handle(port);

    if (huart == NULL)
    {
        return -1;
    }

    /* Abort ongoing reception */
    HAL_UART_AbortReceive_IT(huart);

    return 0;
}

uint16_t uart_read_available(uart_port_t port)
{
    volatile uint16_t *head, *tail;

    /* Get buffer pointers */
    switch (port)
    {
        case UART_PORT_0:
            head = &uart0_rx_head;
            tail = &uart0_rx_tail;
            break;
        case UART_PORT_1:
            head = &uart1_rx_head;
            tail = &uart1_rx_tail;
            break;
        case UART_PORT_2:
            head = &uart2_rx_head;
            tail = &uart2_rx_tail;
            break;
        default:
            return 0;
    }

    /* Calculate available bytes */
    if (*head >= *tail)
    {
        return (*head - *tail);
    }
    else
    {
        return (UART_RX_BUFFER_SIZE - *tail + *head);
    }
}

int uart_flush(uart_port_t port)
{
    /* Reset buffer pointers */
    switch (port)
    {
        case UART_PORT_0:
            uart0_rx_head = 0;
            uart0_rx_tail = 0;
            break;
        case UART_PORT_1:
            uart1_rx_head = 0;
            uart1_rx_tail = 0;
            break;
        case UART_PORT_2:
            uart2_rx_head = 0;
            uart2_rx_tail = 0;
            break;
        default:
            return -1;
    }

    return 0;
}

/**
 * \brief UART RX complete callback (called by HAL).
 *
 * \param[in] huart is the UART handle.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t *buffer;
    volatile uint16_t *head;
    uint8_t *rx_byte;
    uart_port_t port;

    /* Determine which UART triggered the interrupt */
    if (huart->Instance == USART1)
    {
        port = UART_PORT_0;
        buffer = uart0_rx_buffer;
        head = &uart0_rx_head;
        rx_byte = &uart0_rx_byte;
    }
    else if (huart->Instance == USART2)
    {
        port = UART_PORT_1;
        buffer = uart1_rx_buffer;
        head = &uart1_rx_head;
        rx_byte = &uart1_rx_byte;
    }
    else if (huart->Instance == USART3)
    {
        port = UART_PORT_2;
        buffer = uart2_rx_buffer;
        head = &uart2_rx_head;
        rx_byte = &uart2_rx_byte;
    }
    else
    {
        return;
    }

    /* Store received byte in buffer */
    buffer[*head] = *rx_byte;
    *head = (*head + 1) % UART_RX_BUFFER_SIZE;

    /* Re-enable interrupt reception */
    HAL_UART_Receive_IT(huart, rx_byte, 1);
}

/** \} End of uart group */
