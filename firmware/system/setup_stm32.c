/*
 * setup_stm32.c
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
 * \brief System setup implementation for STM32L476RG.
 *
 * \author Gabriel Mariano Marcelino <gabriel.mm8@gmail.com>
 * \author Claude AI (STM32 port)
 *
 * \version 1.0.0
 *
 * \date 2025/01/XX
 *
 * \defgroup setup System setup
 * \ingroup system
 * \{
 */

#include "stm32l4xx_hal.h"

#include <FreeRTOS.h>
#include <task.h>

#include <system/clocks.h>
#include <system/sys_log/sys_log.h>

#include "setup.h"

/**
 * \brief System setup initialization.
 *
 * This function initializes all system peripherals and clocks.
 *
 * \return The status/error code.
 */
int system_setup(void)
{
    clocks_config_t clks;

    /* Initialize HAL */
    HAL_Init();

    /* Configure system clocks */
    clks.mclk_hz = 80000000UL;      /* 80 MHz SYSCLK */
    clks.smclk_hz = 80000000UL;     /* 80 MHz APB2 */
    clks.aclk_hz = 32768UL;         /* 32.768 kHz LSE */

    if (clocks_setup(clks) != 0)
    {
        return -1;
    }

    /* Enable instruction cache */
    __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();

    /* Enable data cache */
    __HAL_FLASH_DATA_CACHE_ENABLE();

    /* Enable prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();

    return 0;
}

/** \} End of setup group */
