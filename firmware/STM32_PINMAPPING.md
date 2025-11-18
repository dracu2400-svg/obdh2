# OBDH 2.0 STM32L476RG Pin Mapping Documentation

## Overview

This document provides a complete reference for the STM32L476RG pin assignments used in the OBDH 2.0 firmware port. The pin mapping is defined in `config/pinmap_stm32l476.h` and maps all peripherals from the original MSP430F6659 design to the STM32L476RG microcontroller.

**Target MCU:** STM32L476RG (LQFP64 package)
- 64 pins total
- 51 GPIO pins available
- Multiple alternate function options per pin

---

## Pin Mapping Strategy

The pin mapping follows these principles:

1. **Peripheral Grouping**: Related signals are grouped on the same GPIO port when possible
2. **Alternate Functions**: Uses default/recommended AF assignments from STM32 datasheet
3. **Compatibility**: Maintains logical organization similar to original MSP430 design
4. **Expandability**: Leaves room for future additions and modifications

---

## Complete Pin Assignment Table

### STM32L476RG Pin Overview (LQFP64)

| STM32 Pin | GPIO | Function | Peripheral | Signal | Notes |
|-----------|------|----------|------------|--------|-------|
| 1 | - | VBAT | Power | Battery backup | RTC backup power |
| 2 | PC13 | GPIO | WKUP2 | Wake-up input | Optional wake source |
| 3 | PC14 | OSC32_IN | RTC | LSE crystal in | 32.768 kHz |
| 4 | PC15 | OSC32_OUT | RTC | LSE crystal out | 32.768 kHz |
| 5 | - | VSS | Power | Ground | |
| 6 | - | VDD | Power | +3.3V | |
| 7 | PH0 | OSC_IN | Clock | HSE crystal in | 8 MHz (optional) |
| 8 | PH1 | OSC_OUT | Clock | HSE crystal out | 8 MHz (optional) |
| 9 | - | NRST | Reset | Hardware reset | |
| 10 | PC0 | ADC_IN1 | ADC | Current sensor 1 | ADC1_IN1 |
| 11 | PC1 | ADC_IN2 | ADC | Current sensor 2 | ADC1_IN2 |
| 12 | PC2 | ADC_IN3 | ADC | Voltage sensor 1 | ADC1_IN3 |
| 13 | PC3 | ADC_IN4 | ADC | Voltage sensor 2 | ADC1_IN4 |
| 14 | - | VSSA | Power | Analog ground | |
| 15 | - | VDDA | Power | Analog +3.3V | |
| 16 | PA0 | ADC_IN5 | ADC | Temperature 1 | ADC1_IN5 |
| 17 | PA1 | ADC_IN6 | ADC | Temperature 2 | ADC1_IN6 |
| 18 | PA2 | USART2_TX | UART | UART0 TX | AF7 |
| 19 | PA3 | USART2_RX | UART | UART0 RX | AF7 |
| 20 | - | VSS | Power | Ground | |
| 21 | - | VDD | Power | +3.3V | |
| 22 | PA4 | SPI1_NSS | SPI | SPI0 CS0 | Manual CS |
| 23 | PA5 | SPI1_SCK | SPI | SPI0 SCK | AF5 |
| 24 | PA6 | SPI1_MISO | SPI | SPI0 MISO | AF5 |
| 25 | PA7 | SPI1_MOSI | SPI | SPI0 MOSI | AF5 |
| 26 | PC4 | ADC_IN13 | ADC | ADC channel 13 | ADC1_IN13 |
| 27 | PC5 | ADC_IN14 | ADC | ADC channel 14 | ADC1_IN14 |
| 28 | PB0 | GPIO | GPIO | System LED | General purpose |
| 29 | PB1 | GPIO | GPIO | Fault LED | General purpose |
| 30 | PB2 | GPIO | GPIO | Beacon control | General purpose |
| 31 | - | VSS | Power | Ground | |
| 32 | - | VDD | Power | +3.3V | |
| 33 | PB10 | I2C2_SCL | I2C | I2C0 SCL | AF4 |
| 34 | PB11 | I2C2_SDA | I2C | I2C0 SDA | AF4 |
| 35 | - | VSS | Power | Ground | |
| 36 | - | VDD | Power | +3.3V | |
| 37 | PB12 | SPI2_NSS | SPI | SPI1 CS0 | Manual CS |
| 38 | PB13 | SPI2_SCK | SPI | SPI1 SCK | AF5 |
| 39 | PB14 | SPI2_MISO | SPI | SPI1 MISO | AF5 |
| 40 | PB15 | SPI2_MOSI | SPI | SPI1 MOSI | AF5 |
| 41 | PC6 | USART6_TX | UART | UART1 TX | AF8 |
| 42 | PC7 | USART6_RX | UART | UART1 RX | AF8 |
| 43 | PC8 | GPIO | SPI | SPI0 CS1 | Manual CS |
| 44 | PC9 | GPIO | SPI | SPI0 CS2 | Manual CS |
| 45 | PA8 | GPIO | SPI | SPI0 CS3 | Manual CS |
| 46 | PA9 | USART1_TX | UART | UART2 TX | AF7 |
| 47 | PA10 | USART1_RX | UART | UART2 RX | AF7 |
| 48 | PA11 | GPIO | SPI | SPI1 CS1 | Manual CS |
| 49 | PA12 | GPIO | SPI | SPI1 CS2 | Manual CS |
| 50 | PA13 | SWDIO | Debug | SWD data | AF0 |
| 51 | - | VSS | Power | Ground | |
| 52 | - | VDD | Power | +3.3V | |
| 53 | PA14 | SWCLK | Debug | SWD clock | AF0 |
| 54 | PA15 | GPIO | SPI | SPI1 CS3 | Manual CS |
| 55 | PC10 | SPI3_SCK | SPI | SPI2 SCK | AF6 |
| 56 | PC11 | SPI3_MISO | SPI | SPI2 MISO | AF6 |
| 57 | PC12 | SPI3_MOSI | SPI | SPI2 MOSI | AF6 |
| 58 | PD2 | GPIO | SPI | SPI2 CS0 | Manual CS |
| 59 | PB3 | GPIO | I2C | I2C pull-up ctrl | General purpose |
| 60 | PB4 | GPIO | GPIO | General purpose | Available |
| 61 | PB5 | GPIO | GPIO | General purpose | Available |
| 62 | PB6 | I2C1_SCL | I2C | I2C1 SCL | AF4 |
| 63 | PB7 | I2C1_SDA | I2C | I2C1 SDA | AF4 |
| 64 | - | BOOT0 | Config | Boot mode select | Pull-down |

---

## Peripheral Pin Assignments

### SPI Interfaces

#### SPI0 (SPI1 peripheral)
| Signal | STM32 Pin | GPIO | AF | Device |
|--------|-----------|------|-----|--------|
| SCK | 23 | PA5 | AF5 | Clock |
| MISO | 24 | PA6 | AF5 | Master In |
| MOSI | 25 | PA7 | AF5 | Master Out |
| CS0 | 22 | PA4 | GPIO | Flash MT25Q |
| CS1 | 43 | PC8 | GPIO | FRAM CY15x102QN |
| CS2 | 44 | PC9 | GPIO | Radio Si4463 |
| CS3 | 45 | PA8 | GPIO | Reserved |

**Speed**: Up to 40 MHz (PCLK2/2)

#### SPI1 (SPI2 peripheral)
| Signal | STM32 Pin | GPIO | AF | Device |
|--------|-----------|------|-----|--------|
| SCK | 38 | PB13 | AF5 | Clock |
| MISO | 39 | PB14 | AF5 | Master In |
| MOSI | 40 | PB15 | AF5 | Master Out |
| CS0 | 37 | PB12 | GPIO | Reserved |
| CS1 | 48 | PA11 | GPIO | Reserved |
| CS2 | 49 | PA12 | GPIO | Reserved |
| CS3 | 54 | PA15 | GPIO | Reserved |

**Speed**: Up to 40 MHz (PCLK1)

#### SPI2 (SPI3 peripheral)
| Signal | STM32 Pin | GPIO | AF | Device |
|--------|-----------|------|-----|--------|
| SCK | 55 | PC10 | AF6 | Clock |
| MISO | 56 | PC11 | AF6 | Master In |
| MOSI | 57 | PC12 | AF6 | Master Out |
| CS0 | 58 | PD2 | GPIO | Reserved |
| CS1 | - | - | - | Not mapped |

**Speed**: Up to 40 MHz (PCLK1)

---

### I2C Interfaces

#### I2C0 (I2C2 peripheral)
| Signal | STM32 Pin | GPIO | AF | Speed |
|--------|-----------|------|-----|-------|
| SCL | 33 | PB10 | AF4 | Up to 1 MHz |
| SDA | 34 | PB11 | AF4 | Up to 1 MHz |

**Devices**: EPS (SL-EPS2), Sensors

#### I2C1 (I2C1 peripheral)
| Signal | STM32 Pin | GPIO | AF | Speed |
|--------|-----------|------|-----|-------|
| SCL | 62 | PB6 | AF4 | Up to 1 MHz |
| SDA | 63 | PB7 | AF4 | Up to 1 MHz |

**Devices**: Additional sensors, expansion

#### I2C2 (I2C3 peripheral)
| Signal | STM32 Pin | GPIO | AF | Speed |
|--------|-----------|------|-----|-------|
| SCL | - | PB8 | AF4 | Reserved |
| SDA | - | PB9 | AF4 | Reserved |

**Note**: I2C3 pins are not available on LQFP64 package

---

### UART Interfaces

#### UART0 (USART2)
| Signal | STM32 Pin | GPIO | AF | Purpose |
|--------|-----------|------|-----|---------|
| TX | 18 | PA2 | AF7 | Console output |
| RX | 19 | PA3 | AF7 | Console input |

**Baud Rate**: Up to 5 Mbps
**Usage**: Primary console/debug interface

#### UART1 (USART6)
| Signal | STM32 Pin | GPIO | AF | Purpose |
|--------|-----------|------|-----|---------|
| TX | 41 | PC6 | AF8 | Command interface |
| RX | 42 | PC7 | AF8 | Command interface |

**Baud Rate**: Up to 5 Mbps
**Usage**: Command and telemetry

#### UART2 (USART1)
| Signal | STM32 Pin | GPIO | AF | Purpose |
|--------|-----------|------|-----|---------|
| TX | 46 | PA9 | AF7 | Reserved |
| RX | 47 | PA10 | AF7 | Reserved |

**Baud Rate**: Up to 10 Mbps
**Usage**: Reserved for future use

---

### ADC Channels

The STM32L476RG has a 12-bit ADC with 16 external channels.

| ADC Channel | STM32 Pin | GPIO | Sensor Type | Signal |
|-------------|-----------|------|-------------|--------|
| ADC1_IN1 | 10 | PC0 | Current | Current sensor 1 |
| ADC1_IN2 | 11 | PC1 | Current | Current sensor 2 |
| ADC1_IN3 | 12 | PC2 | Voltage | Voltage sensor 1 |
| ADC1_IN4 | 13 | PC3 | Voltage | Voltage sensor 2 |
| ADC1_IN5 | 16 | PA0 | Temperature | Temperature 1 |
| ADC1_IN6 | 17 | PA1 | Temperature | Temperature 2 |
| ADC1_IN13 | 26 | PC4 | General | ADC channel 13 |
| ADC1_IN14 | 27 | PC5 | General | ADC channel 14 |

**Resolution**: 12-bit (0-4095)
**Reference**: 3.3V (VDDA)
**Sample Rate**: Up to 5.33 Msps

---

### GPIO Assignments

#### LEDs and Indicators
| Function | STM32 Pin | GPIO | Active |
|----------|-----------|------|--------|
| System LED | 28 | PB0 | High |
| Fault LED | 29 | PB1 | High |
| Beacon LED | 30 | PB2 | High |

#### Control Signals
| Function | STM32 Pin | GPIO | Direction |
|----------|-----------|------|-----------|
| I2C Pull-up Control | 59 | PB3 | Output |
| General Purpose 1 | 60 | PB4 | I/O |
| General Purpose 2 | 61 | PB5 | I/O |

---

### Special Function Pins

#### Clock Sources
| Function | STM32 Pin | GPIO | Frequency |
|----------|-----------|------|-----------|
| LSE (RTC) IN | 3 | PC14 | 32.768 kHz |
| LSE (RTC) OUT | 4 | PC15 | 32.768 kHz |
| HSE IN | 7 | PH0 | 8 MHz (optional) |
| HSE OUT | 8 | PH1 | 8 MHz (optional) |

**Note**: Current configuration uses internal HSI16 (16 MHz) with PLL

#### Debug Interface (SWD)
| Function | STM32 Pin | GPIO | AF |
|----------|-----------|------|-----|
| SWDIO | 50 | PA13 | AF0 |
| SWCLK | 53 | PA14 | AF0 |

**Note**: JTAG is not used; SWD provides debug and programming access

#### Reset and Boot
| Function | STM32 Pin | Default |
|----------|-----------|---------|
| NRST | 9 | External pull-up |
| BOOT0 | 64 | Pull-down (boot from Flash) |

---

## Pin Conflicts and Constraints

### USB Pins
The STM32L476RG has USB OTG capability on PA11/PA12:
- PA11: USB_DM (conflicts with SPI1_CS1)
- PA12: USB_DP (conflicts with SPI1_CS2)

**Current Usage**: Assigned as GPIO for SPI chip selects
**To Enable USB**: Remap SPI1_CS1 and SPI1_CS2 to other pins

### JTAG vs SWD
- **SWD** (Serial Wire Debug): Uses PA13 (SWDIO) and PA14 (SWCLK) - **ACTIVE**
- **JTAG**: Requires 5 pins (PA13, PA14, PA15, PB3, PB4) - **NOT USED**

PA15, PB3, and PB4 are freed for GPIO use by disabling JTAG.

### Timer Outputs
Many GPIO pins have timer alternate functions that are not currently mapped:
- TIM1, TIM2, TIM3, TIM4, TIM5 channels available
- Can be used for PWM if needed in future

---

## Power Supply Pins

| Pin Type | Pins | Voltage | Notes |
|----------|------|---------|-------|
| VDD | 6, 21, 32, 36, 52 | 3.3V | Digital power |
| VSS | 5, 20, 31, 35, 51 | GND | Digital ground |
| VDDA | 15 | 3.3V | Analog power |
| VSSA | 14 | GND | Analog ground |
| VBAT | 1 | 1.65-3.6V | RTC backup battery |

**Decoupling**: Each VDD/VSS pair should have 100nF ceramic capacitor
**Analog Power**: VDDA should have dedicated LC filter from VDD

---

## Modification Guidelines

### Changing Pin Assignments

To modify pin assignments, edit `firmware/config/pinmap_stm32l476.h`:

1. **Verify Alternate Function**: Check STM32L476RG datasheet Table 17
2. **Update Pin Definition**: Change GPIO port and pin number
3. **Update AF Number**: Ensure correct alternate function selected
4. **Check Conflicts**: Verify pin not already assigned
5. **Update Documentation**: Reflect changes in this document

### Example: Moving UART0 from USART2 to LPUART1

```c
/* Original (USART2 on PA2/PA3) */
#define UART0_TX_PORT               GPIOA
#define UART0_TX_PIN                GPIO_PIN_2
#define UART0_TX_AF                 GPIO_AF7_USART2

/* Modified (LPUART1 on PB10/PB11) - CONFLICTS with I2C! */
#define UART0_TX_PORT               GPIOB
#define UART0_TX_PIN                GPIO_PIN_10
#define UART0_TX_AF                 GPIO_AF8_LPUART1
```

**Warning**: This creates a conflict with I2C0! Must resolve before use.

---

## Pin Availability Summary

### Used Pins: 38/51 GPIO pins
- SPI: 13 pins (3 x SCK/MISO/MOSI + 10 CS)
- I2C: 4 pins (2 x SCL/SDA)
- UART: 6 pins (3 x TX/RX)
- ADC: 8 pins (analog inputs)
- GPIO: 5 pins (LEDs, control)
- Debug: 2 pins (SWDIO, SWCLK)

### Available Pins: 13 GPIO pins
These pins are not currently assigned and available for expansion:
- PB4, PB5 (if JTAG disabled - currently freed)
- Additional PC and PD pins depending on package

### Reserved Pins: 26 pins
- Power: 12 pins (VDD, VSS, VDDA, VSSA, VBAT)
- Clock: 4 pins (LSE, HSE)
- Reset/Boot: 2 pins (NRST, BOOT0)
- Used GPIO: 38 pins

---

## Hardware Design Recommendations

### PCB Layout

1. **SPI Buses**
   - Keep traces short (< 10 cm for 40 MHz operation)
   - Route SCK away from sensitive analog signals
   - Use ground plane underneath

2. **I2C Buses**
   - External pull-up resistors: 4.7kΩ for 100 kHz, 2.2kΩ for 400 kHz
   - Keep traces < 20 cm for reliable operation
   - Avoid routing near switching power supplies

3. **ADC Inputs**
   - Separate VDDA and VSSA from digital supplies
   - Use LC filter (10μH + 1μF) on VDDA
   - Keep analog traces away from digital signals
   - Add 100nF capacitor close to each ADC input

4. **Crystal Circuits**
   - LSE: 32.768 kHz crystal with 10pF load capacitors
   - HSE (optional): 8 MHz crystal with 20pF load capacitors
   - Keep crystal traces short and symmetric

### External Components

1. **Decoupling Capacitors**
   - 100nF ceramic at each VDD pin
   - 10μF tantalum/ceramic at main power input
   - 1μF ceramic at VDDA

2. **Pull-up/Pull-down Resistors**
   - BOOT0: 10kΩ pull-down to GND
   - NRST: 10kΩ pull-up to VDD (optional, internal pull-up available)
   - I2C: 2.2kΩ - 4.7kΩ pull-up to VDD

3. **Protection**
   - ESD protection on all external connectors
   - Series resistors on SWD lines (100Ω) if exposed

---

## Alternate Function Reference

### Quick AF Lookup

| Peripheral | AF Number | Typical Pins |
|------------|-----------|--------------|
| SPI1 | AF5 | PA4-PA7 |
| SPI2 | AF5 | PB12-PB15 |
| SPI3 | AF6 | PC10-PC12 |
| I2C1 | AF4 | PB6-PB7 |
| I2C2 | AF4 | PB10-PB11 |
| USART1 | AF7 | PA9-PA10, PB6-PB7 |
| USART2 | AF7 | PA2-PA3 |
| USART6 | AF8 | PC6-PC7 |
| TIM1 | AF1 | PA8-PA11, PE9-PE14 |
| TIM2 | AF1 | PA0-PA3, PA15, PB3 |
| ADC | Analog | PA0-PA7, PC0-PC5 |

For complete alternate function mapping, refer to STM32L476RG datasheet Table 17.

---

## References

- **STM32L476RG Datasheet**: [DS10198](https://www.st.com/resource/en/datasheet/stm32l476rg.pdf)
- **STM32L476RG Reference Manual**: [RM0351](https://www.st.com/resource/en/reference_manual/rm0351-stm32l47xxx-stm32l48xxx-stm32l49xxx-and-stm32l4axxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- **Pin Configuration**: `firmware/config/pinmap_stm32l476.h`
- **Porting Guide**: `firmware/STM32_PORTING_GUIDE.md`

---

**Document Version:** 1.0
**Last Updated:** 2025-01-18
**Author:** OBDH 2.0 STM32 Port Team
