/*
 * main_stm32.c
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
 * \brief Main file for STM32L476RG.
 *
 * \author Gabriel Mariano Marcelino <gabriel.mm8@gmail.com>
 * \author Claude AI (STM32 port)
 *
 * \version 1.0.0
 *
 * \date 2025/01/XX
 *
 * \defgroup main Main file
 * \{
 */

#include "stm32l4xx_hal.h"

#include <FreeRTOS.h>
#include <task.h>

#include "devices/watchdog/watchdog.h"
#include "system/clocks.h"
#include "app/tasks/tasks.h"

/**
 * \brief Main entry point.
 *
 * \return None (never returns).
 */
int main(void)
{
    int err = 0;

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* System clocks configuration */
    clocks_config_t clk_conf = {0};

    clk_conf.mclk_hz    = 80000000UL;   /* 80 MHz SYSCLK */
    clk_conf.smclk_hz   = 80000000UL;   /* 80 MHz APB2 */
    clk_conf.aclk_hz    = 32768UL;      /* 32.768 kHz LSE for RTC */

    err = clocks_setup(clk_conf);

    if (err != 0)
    {
        /* Clock configuration failed - blink error LED or halt */
        while(1)
        {
            /* Error indication */
        }
    }

    /* Enable instruction and data caches */
    __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
    __HAL_FLASH_DATA_CACHE_ENABLE();
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();

    /* Watchdog device initialization */
    err = watchdog_init();

    if (err != 0)
    {
        /* Watchdog initialization failed - may continue without it */
    }

    /* Create all the tasks */
    create_tasks();

    /* Start the scheduler */
    vTaskStartScheduler();

    /* Will only get here if there was insufficient memory to create the idle and/or timer task */
    while(1)
    {
        /* Should never reach here */
    }
}

/** \} End of main group */
