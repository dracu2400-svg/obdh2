/*
 * rtc_stm32.c
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
 * \brief RTC driver implementation for STM32L476RG.
 *
 * \author Gabriel Mariano Marcelino <gabriel.mm8@gmail.com>
 * \author Claude AI (STM32 port)
 *
 * \version 1.0.0
 *
 * \date 2025/01/XX
 *
 * \addtogroup rtc
 * \{
 */

#include "stm32l4xx_hal.h"

#include <config/config.h>
#include <system/sys_log/sys_log.h>

#include "rtc.h"

/* RTC handle */
static RTC_HandleTypeDef hrtc;

/* RTC initialization flag */
static bool rtc_is_initialized = false;

int rtc_init(void)
{
    if (rtc_is_initialized)
    {
        return 0;  /* Already initialized */
    }

    /* Enable RTC clock */
    __HAL_RCC_RTC_ENABLE();

    /* Initialize RTC */
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
        return -1;
    }

    rtc_is_initialized = true;

    return 0;
}

rtc_calendar_t rtc_read_calendar(void)
{
    RTC_DateTypeDef sDate = {0};
    RTC_TimeTypeDef sTime = {0};
    rtc_calendar_t calendar = {0};

    if (!rtc_is_initialized)
    {
        rtc_init();
    }

    /* Get current date and time */
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    /* Map to calendar structure */
    calendar.Seconds = sTime.Seconds;
    calendar.Minutes = sTime.Minutes;
    calendar.Hours = sTime.Hours;
    calendar.DayOfWeek = sDate.WeekDay;
    calendar.DayOfMonth = sDate.Date;
    calendar.Month = sDate.Month;
    calendar.Year = sDate.Year + 2000;  /* STM32 RTC year is 0-99 */

    return calendar;
}

int rtc_set_calendar(rtc_calendar_t calendar)
{
    RTC_DateTypeDef sDate = {0};
    RTC_TimeTypeDef sTime = {0};

    if (!rtc_is_initialized)
    {
        if (rtc_init() != 0)
        {
            return -1;
        }
    }

    /* Map calendar structure to STM32 format */
    sTime.Hours = calendar.Hours;
    sTime.Minutes = calendar.Minutes;
    sTime.Seconds = calendar.Seconds;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        return -1;
    }

    sDate.WeekDay = calendar.DayOfWeek;
    sDate.Date = calendar.DayOfMonth;
    sDate.Month = calendar.Month;
    sDate.Year = calendar.Year - 2000;  /* Convert to 0-99 range */

    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        return -1;
    }

    return 0;
}

/** \} End of rtc group */
