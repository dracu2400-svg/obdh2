/*
 * flash_stm32.c
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
 * \brief Flash driver implementation for STM32L476RG.
 *
 * \author Gabriel Mariano Marcelino <gabriel.mm8@gmail.com>
 * \author Claude AI (STM32 port)
 *
 * \version 1.0.0
 *
 * \date 2025/01/XX
 *
 * \addtogroup flash
 * \{
 */

#include "stm32l4xx_hal.h"

#include <config/config.h>
#include <system/sys_log/sys_log.h>

#include "flash.h"

#define FLASH_MODULE_NAME       "FLASH"

/* Flash initialization flag */
static bool flash_is_initialized = false;

int flash_init(void)
{
    if (flash_is_initialized)
    {
        return 0;  /* Already initialized */
    }

    /* No specific initialization needed for internal flash on STM32 */
    /* HAL_FLASH functions can be called directly */

    flash_is_initialized = true;

    return 0;
}

void flash_write(uint8_t *data, uint16_t len)
{
    /* This function needs to be implemented based on specific requirements
     * STM32L4 Flash can only be written in 64-bit (double word) units
     * For now, this is a placeholder */
    (void)data;
    (void)len;
}

void flash_write_single(uint8_t data, uint8_t *addr)
{
    uint32_t address = (uint32_t)addr;
    uint64_t data_dword;

    /* Unlock Flash */
    HAL_FLASH_Unlock();

    /* Prepare data (64-bit write required) */
    data_dword = (uint64_t)data;

    /* Write to Flash */
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data_dword);

    /* Lock Flash */
    HAL_FLASH_Lock();
}

uint8_t flash_read_single(uint8_t *addr)
{
    /* Direct memory read */
    return *addr;
}

void flash_write_long(uint32_t data, uint32_t *addr)
{
    uint32_t address = (uint32_t)addr;
    uint64_t data_dword;

    /* Unlock Flash */
    HAL_FLASH_Unlock();

    /* Prepare data (64-bit write required) */
    data_dword = (uint64_t)data;

    /* Write to Flash */
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data_dword);

    /* Lock Flash */
    HAL_FLASH_Lock();
}

uint32_t flash_read_long(uint32_t *addr)
{
    /* Direct memory read */
    return *addr;
}

void flash_erase(uint32_t *region)
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError;
    uint32_t address = (uint32_t)region;

    /* Unlock Flash */
    HAL_FLASH_Unlock();

    /* Fill EraseInit structure */
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;

    /* Determine which bank and page */
    if (address < 0x08080000)
    {
        EraseInitStruct.Banks = FLASH_BANK_1;
        EraseInitStruct.Page = (address - FLASH_BASE) / FLASH_PAGE_SIZE;
    }
    else
    {
        EraseInitStruct.Banks = FLASH_BANK_2;
        EraseInitStruct.Page = (address - 0x08080000) / FLASH_PAGE_SIZE;
    }

    EraseInitStruct.NbPages = 1;

    /* Erase the page */
    HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);

    /* Lock Flash */
    HAL_FLASH_Lock();
}

/** \} End of flash group */
