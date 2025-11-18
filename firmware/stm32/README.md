# STM32L476RG Port Files

This directory contains the STM32L476RG-specific files for the OBDH 2.0 firmware port.

## Directory Contents

- `STM32L476RGTx_FLASH.ld` - Linker script for STM32L476RG
- `startup_stm32l476xx.s` - Startup code and interrupt vector table
- `system_stm32l4xx.c/h` - System initialization
- `stm32l4xx_it.c/h` - Interrupt handlers
- `stm32l4xx_hal_msp.c` - HAL MSP initialization callbacks
- `stm32l4xx_hal_conf.h` - HAL library configuration

## Required: STM32 HAL Library

**IMPORTANT**: This directory is missing the STM32 HAL library files which are required to build the firmware. These files are not included in the repository due to their size and licensing.

### Required Directory Structure

You need to add the following directories and files:

```
firmware/stm32/
├── STM32L4xx_HAL_Driver/
│   ├── Inc/
│   │   ├── stm32l4xx_hal.h
│   │   ├── stm32l4xx_hal_adc.h
│   │   ├── stm32l4xx_hal_adc_ex.h
│   │   ├── stm32l4xx_hal_cortex.h
│   │   ├── stm32l4xx_hal_dma.h
│   │   ├── stm32l4xx_hal_flash.h
│   │   ├── stm32l4xx_hal_flash_ex.h
│   │   ├── stm32l4xx_hal_gpio.h
│   │   ├── stm32l4xx_hal_gpio_ex.h
│   │   ├── stm32l4xx_hal_i2c.h
│   │   ├── stm32l4xx_hal_i2c_ex.h
│   │   ├── stm32l4xx_hal_iwdg.h
│   │   ├── stm32l4xx_hal_pwr.h
│   │   ├── stm32l4xx_hal_pwr_ex.h
│   │   ├── stm32l4xx_hal_rcc.h
│   │   ├── stm32l4xx_hal_rcc_ex.h
│   │   ├── stm32l4xx_hal_rtc.h
│   │   ├── stm32l4xx_hal_rtc_ex.h
│   │   ├── stm32l4xx_hal_spi.h
│   │   ├── stm32l4xx_hal_tim.h
│   │   ├── stm32l4xx_hal_tim_ex.h
│   │   ├── stm32l4xx_hal_uart.h
│   │   ├── stm32l4xx_hal_uart_ex.h
│   │   ├── stm32l4xx_hal_wwdg.h
│   │   └── Legacy/
│   │       └── stm32_hal_legacy.h
│   └── Src/
│       ├── stm32l4xx_hal.c
│       ├── stm32l4xx_hal_adc.c
│       ├── stm32l4xx_hal_adc_ex.c
│       ├── stm32l4xx_hal_cortex.c
│       ├── stm32l4xx_hal_dma.c
│       ├── stm32l4xx_hal_flash.c
│       ├── stm32l4xx_hal_flash_ex.c
│       ├── stm32l4xx_hal_flash_ramfunc.c
│       ├── stm32l4xx_hal_gpio.c
│       ├── stm32l4xx_hal_i2c.c
│       ├── stm32l4xx_hal_i2c_ex.c
│       ├── stm32l4xx_hal_iwdg.c
│       ├── stm32l4xx_hal_pwr.c
│       ├── stm32l4xx_hal_pwr_ex.c
│       ├── stm32l4xx_hal_rcc.c
│       ├── stm32l4xx_hal_rcc_ex.c
│       ├── stm32l4xx_hal_rtc.c
│       ├── stm32l4xx_hal_rtc_ex.c
│       ├── stm32l4xx_hal_spi.c
│       ├── stm32l4xx_hal_tim.c
│       ├── stm32l4xx_hal_tim_ex.c
│       ├── stm32l4xx_hal_uart.c
│       ├── stm32l4xx_hal_uart_ex.c
│       └── stm32l4xx_hal_wwdg.c
└── CMSIS/
    ├── Device/ST/STM32L4xx/Include/
    │   ├── stm32l476xx.h
    │   ├── stm32l4xx.h
    │   └── system_stm32l4xx.h
    └── Include/
        ├── cmsis_compiler.h
        ├── cmsis_gcc.h
        ├── cmsis_version.h
        ├── core_cm4.h
        └── mpu_armv7.h
```

### How to Obtain the HAL Library

#### Option 1: Using STM32CubeMX (Recommended)

1. Download and install [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html)
2. Create a new project for STM32L476RG
3. Select only the peripherals you need (or select all for completeness)
4. Generate code with the following settings:
   - Toolchain: Makefile
   - Copy only the necessary library files: Yes
5. Navigate to the generated project directory
6. Copy the `Drivers/STM32L4xx_HAL_Driver` directory to `firmware/stm32/STM32L4xx_HAL_Driver`
7. Copy the `Drivers/CMSIS` directory to `firmware/stm32/CMSIS`

#### Option 2: Download STM32CubeL4 Package

1. Download the [STM32CubeL4](https://www.st.com/en/embedded-software/stm32cubel4.html) firmware package
2. Extract the archive
3. Navigate to `STM32Cube_FW_L4_VX.X.X/Drivers/`
4. Copy `STM32L4xx_HAL_Driver` to `firmware/stm32/STM32L4xx_HAL_Driver`
5. Copy `CMSIS/Device/ST/STM32L4xx/Include` to `firmware/stm32/CMSIS/Device/ST/STM32L4xx/Include`
6. Copy `CMSIS/Include` to `firmware/stm32/CMSIS/Include`

#### Option 3: Clone from GitHub

```bash
cd firmware/stm32

# Clone STM32CubeL4 repository
git clone https://github.com/STMicroelectronics/STM32CubeL4.git temp_cube

# Copy required files
mkdir -p STM32L4xx_HAL_Driver CMSIS
cp -r temp_cube/Drivers/STM32L4xx_HAL_Driver/* STM32L4xx_HAL_Driver/
cp -r temp_cube/Drivers/CMSIS/Device CMSIS/
cp -r temp_cube/Drivers/CMSIS/Include CMSIS/

# Clean up
rm -rf temp_cube
```

### Verification

After installing the HAL library, verify the structure:

```bash
cd firmware/stm32
ls -la STM32L4xx_HAL_Driver/Inc/
ls -la STM32L4xx_HAL_Driver/Src/
ls -la CMSIS/Device/ST/STM32L4xx/Include/
ls -la CMSIS/Include/
```

You should see all the header and source files listed above.

### Building

Once the HAL library is installed:

```bash
cd firmware
make -f Makefile.stm32 clean
make -f Makefile.stm32 -j$(nproc)
```

If the build fails with missing header errors, ensure the HAL library is properly installed.

## License

The STM32 HAL library is provided by STMicroelectronics under their own license terms. Please refer to the LICENSE file in the HAL library directory for details.

The OBDH 2.0 firmware is licensed under GPL v3. See the main project LICENSE file for details.
