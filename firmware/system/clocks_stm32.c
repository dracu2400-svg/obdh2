/*
 * clocks_stm32.c
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
 * \brief System clocks functions implementation for STM32L476RG.
 *
 * \author Gabriel Mariano Marcelino <gabriel.mm8@gmail.com>
 * \author Claude AI (STM32 port)
 *
 * \version 1.0.0
 *
 * \date 2025/01/XX
 *
 * \addtogroup clocks
 * \{
 */

#include "stm32l4xx_hal.h"

#include "clocks.h"

/**
 * \brief Configure system clock to 80 MHz using PLL from HSI.
 *
 * System Clock Configuration:
 * - System Clock source: PLL (HSI)
 * - SYSCLK(Hz): 80000000
 * - HCLK(Hz): 80000000
 * - AHB Prescaler: 1
 * - APB1 Prescaler: 1
 * - APB2 Prescaler: 1
 * - HSI Frequency(Hz): 16000000
 * - PLL_M: 1
 * - PLL_N: 10
 * - PLL_P: 7
 * - PLL_Q: 2
 * - PLL_R: 2
 * - Flash Latency(WS): 4
 *
 * \param[in] clks is the clock configuration structure.
 *
 * \return Status code (0 = success, -1 = error).
 */
int clocks_setup(clocks_config_t clks)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Configure the main internal regulator output voltage */
    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
    {
        return -1;
    }

    /* Configure LSE Drive Capability for RTC if available */
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    /* Initializes the RCC Oscillators according to the specified parameters */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;  /* 32.768 kHz crystal for RTC */
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;

    /* PLL configuration for 80 MHz SYSCLK from 16 MHz HSI
     * VCO = (HSI / PLLM) * PLLN = (16 / 1) * 10 = 160 MHz
     * SYSCLK = VCO / PLLR = 160 / 2 = 80 MHz
     * USB/RNG = VCO / PLLQ = 160 / 2 = 80 MHz (not ideal for USB)
     * SAI = VCO / PLLP = 160 / 7 = 22.857 MHz
     */
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 10;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        return -1;
    }

    /* Initializes the CPU, AHB and APB busses clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;   /* HCLK = 80 MHz */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;    /* PCLK1 = 80 MHz */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;    /* PCLK2 = 80 MHz */

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        return -1;
    }

    /* Enable Clock Security System (CSS) to detect HSE failures */
    HAL_RCC_EnableCSS();

    return 0;
}

/**
 * \brief Read current clock frequencies.
 *
 * \return Clock configuration structure with current frequencies.
 */
clocks_config_t clocks_read(void)
{
    clocks_config_t clks;

    /* Get clock frequencies from HAL */
    clks.mclk_hz = HAL_RCC_GetSysClockFreq();    /* SYSCLK = main clock */
    clks.smclk_hz = HAL_RCC_GetPCLK2Freq();      /* Use APB2 as equivalent to SMCLK */
    clks.aclk_hz = LSE_VALUE;                    /* LSE = 32.768 kHz (equivalent to ACLK) */

    return clks;
}

/**
 * \brief NMI Handler - handles clock security system failures.
 *
 * This function is called when:
 * - Clock Security System (CSS) detects HSE failure
 * - Other non-maskable interrupts
 *
 * \return None.
 */
void NMI_Handler(void)
{
    /* Check if CSS interrupt occurred */
    if (__HAL_RCC_GET_IT(RCC_IT_CSS))
    {
        /* Clear CSS interrupt flag */
        __HAL_RCC_CLEAR_IT(RCC_IT_CSS);

        /* HSE has failed! System will automatically switch to HSI
         * Application should handle this condition appropriately:
         * - Log the error
         * - Notify ground station
         * - Attempt to restart HSE if available
         */

        /* For now, attempt to reconfigure clocks using HSI */
        clocks_config_t clks;
        clks.mclk_hz = 80000000UL;
        clks.smclk_hz = 80000000UL;
        clks.aclk_hz = 32768UL;

        /* Try to re-setup clocks (will use HSI automatically) */
        clocks_setup(clks);
    }
}

/** \} End of clocks group */
