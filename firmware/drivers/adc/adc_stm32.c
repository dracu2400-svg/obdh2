/*
 * adc_stm32.c
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
 * \brief ADC driver implementation for STM32L476RG.
 *
 * \author Gabriel Mariano Marcelino <gabriel.mm8@gmail.com>
 * \author Claude AI (STM32 port)
 *
 * \version 1.0.0
 *
 * \date 2025/01/XX
 *
 * \addtogroup adc
 * \{
 */

#include "stm32l4xx_hal.h"

#include <config/config.h>
#include <system/sys_log/sys_log.h>

#include "adc.h"

/* ADC handle */
static ADC_HandleTypeDef hadc1;

/* ADC initialization flag */
static bool adc_is_initialized = false;

/**
 * \brief Map ADC port to STM32 ADC channel.
 *
 * \param[in] port is the ADC port.
 *
 * \return STM32 ADC channel number, or 0xFFFFFFFF on error.
 */
static uint32_t adc_get_channel(adc_port_t port)
{
    switch (port)
    {
        case ADC_PORT_0:    return ADC_CHANNEL_0;
        case ADC_PORT_1:    return ADC_CHANNEL_1;
        case ADC_PORT_2:    return ADC_CHANNEL_2;
        case ADC_PORT_3:    return ADC_CHANNEL_3;
        case ADC_PORT_4:    return ADC_CHANNEL_4;
        case ADC_PORT_5:    return ADC_CHANNEL_5;
        case ADC_PORT_6:    return ADC_CHANNEL_6;
        case ADC_PORT_7:    return ADC_CHANNEL_7;
        case ADC_PORT_8:    return ADC_CHANNEL_8;
        case ADC_PORT_9:    return ADC_CHANNEL_9;
        case ADC_PORT_10:   return ADC_CHANNEL_10;
        case ADC_PORT_11:   return ADC_CHANNEL_11;
        case ADC_PORT_12:   return ADC_CHANNEL_12;
        case ADC_PORT_13:   return ADC_CHANNEL_13;
        case ADC_PORT_14:   return ADC_CHANNEL_14;
        case ADC_PORT_15:   return ADC_CHANNEL_15;
        default:            return 0xFFFFFFFF;
    }
}

int adc_init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    if (adc_is_initialized)
    {
        return 0;  /* Already initialized */
    }

    /* Enable ADC clock */
    __HAL_RCC_ADC_CLK_ENABLE();

    /* Configure the global features of the ADC */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.OversamplingMode = DISABLE;

    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        return -1;
    }

    /* Run ADC calibration */
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
    {
        return -1;
    }

    adc_is_initialized = true;

    return 0;
}

int adc_read(adc_port_t port, uint16_t *val)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    uint32_t channel;

    if (!adc_is_initialized)
    {
        if (adc_init() != 0)
        {
            return -1;
        }
    }

    /* Get ADC channel */
    channel = adc_get_channel(port);
    if (channel == 0xFFFFFFFF)
    {
        #if defined(CONFIG_DRIVERS_DEBUG_ENABLED) && (CONFIG_DRIVERS_DEBUG_ENABLED == 1)
            sys_log_print_event_from_module(SYS_LOG_ERROR, ADC_MODULE_NAME, "Invalid ADC port!");
            sys_log_new_line();
        #endif
        return -1;
    }

    /* Configure for the selected ADC channel */
    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        return -1;
    }

    /* Start ADC conversion */
    if (HAL_ADC_Start(&hadc1) != HAL_OK)
    {
        return -1;
    }

    /* Wait for conversion to complete */
    if (HAL_ADC_PollForConversion(&hadc1, ADC_TIMEOUT_MS) != HAL_OK)
    {
        return -1;
    }

    /* Read converted value */
    *val = (uint16_t)HAL_ADC_GetValue(&hadc1);

    /* Stop ADC */
    HAL_ADC_Stop(&hadc1);

    return 0;
}

float adc_temp_get_mref(void)
{
    /* Temperature sensor calibration values from STM32L4 datasheet
     * These are device-specific and should be read from calibration area
     * For now, return typical values */
    return 1.43f;  /* Typical V_SENSE at 30°C in Volts */
}

float adc_temp_get_nref(void)
{
    /* Temperature coefficient in mV/°C */
    return 4.3f;  /* Typical Avg_Slope in mV/°C */
}

void adc_delay_ms(uint16_t ms)
{
    HAL_Delay(ms);
}

/** \} End of adc group */
