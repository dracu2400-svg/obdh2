/*
 * wdt_stm32.c
 *
 * Copyright (C) 2019, SpaceLab.
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
 * \brief Internal watchdog timer driver implementation for STM32L476RG.
 *
 * \author Gabriel Mariano Marcelino <gabriel.mm8@gmail.com>
 * \author Claude AI (STM32 port)
 *
 * \version 1.0.0
 *
 * \date 2025/01/XX
 *
 * \addtogroup wdt
 * \{
 */

#include "stm32l4xx_hal.h"

#include <config/config.h>
#include <system/sys_log/sys_log.h>

#include "wdt.h"

/* IWDG handle */
static IWDG_HandleTypeDef hiwdg;

/* WDT initialization flag */
static bool wdt_is_initialized = false;

/**
 * \brief Map generic clock divider to STM32 IWDG prescaler.
 *
 * \param[in] clk_div is the generic clock divider.
 *
 * \return STM32 IWDG prescaler value.
 */
static uint32_t wdt_map_prescaler(uint8_t clk_div)
{
    switch (clk_div)
    {
        case WDT_CLK_DIV_64:        return IWDG_PRESCALER_64;
        case WDT_CLK_DIV_512:       return IWDG_PRESCALER_128;
        case WDT_CLK_DIV_8192:      return IWDG_PRESCALER_256;
        default:                    return IWDG_PRESCALER_32;
    }
}

int wdt_init(wdt_config_t config)
{
    if (wdt_is_initialized)
    {
        return 0;  /* Already initialized */
    }

    /* Configure IWDG */
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = wdt_map_prescaler(config.clk_div);
    hiwdg.Init.Reload = 4095;  /* Maximum reload value (12-bit) */
    hiwdg.Init.Window = IWDG_WINDOW_DISABLE;

    /* Initialize IWDG */
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
    {
        #if defined(CONFIG_DRIVERS_DEBUG_ENABLED) && (CONFIG_DRIVERS_DEBUG_ENABLED == 1)
            sys_log_print_event_from_module(SYS_LOG_ERROR, WDT_MODULE_NAME, "Failed to initialize!");
            sys_log_new_line();
        #endif
        return -1;
    }

    wdt_is_initialized = true;

    return 0;
}

void wdt_reset(void)
{
    if (!wdt_is_initialized)
    {
        return;
    }

    /* Refresh IWDG counter */
    HAL_IWDG_Refresh(&hiwdg);
}

/** \} End of wdt group */
