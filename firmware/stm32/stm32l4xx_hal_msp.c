/**
  ******************************************************************************
  * @file    stm32l4xx_hal_msp.c
  * @brief   This file provides code for the MSP Initialization
  *          and de-Initialization codes.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  /* System interrupt init*/
  /* PendSV_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);
}

/**
  * @brief SPI MSP Initialization
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
  /* Peripheral clock enable and GPIO configuration will be done in driver layer */
}

/**
  * @brief SPI MSP De-Initialization
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{
  /* Peripheral de-initialization will be done in driver layer */
}

/**
  * @brief I2C MSP Initialization
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  /* Peripheral clock enable and GPIO configuration will be done in driver layer */
}

/**
  * @brief I2C MSP De-Initialization
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  /* Peripheral de-initialization will be done in driver layer */
}

/**
  * @brief UART MSP Initialization
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  /* Peripheral clock enable and GPIO configuration will be done in driver layer */
}

/**
  * @brief UART MSP De-Initialization
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
  /* Peripheral de-initialization will be done in driver layer */
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
