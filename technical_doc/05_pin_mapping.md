# OBDH 2.0 Pin Mapping and GPIO Configuration

## Overview

This document provides the complete pin mapping for the MSP430F6659 microcontroller used in the OBDH 2.0 system. It details the assignment of GPIO pins, peripheral functions, and electrical characteristics.

**Microcontroller:** Texas Instruments MSP430F6659
**Package:** 100-pin LQFP
**I/O Pins:** Up to 74 GPIO pins available
**Special Functions:** Multiplexed with UART, SPI, I2C, ADC, Timers

---

## Pin Mapping Philosophy

The MSP430F6659 uses **multiplexed pins**, where each physical pin can serve multiple functions:
1. **Primary Function:** GPIO (General Purpose Input/Output)
2. **Secondary Function:** Peripheral module (UART, SPI, I2C, etc.)
3. **Tertiary Function:** Timer, ADC, or other specialized function

**Pin Configuration:** Controlled by Port Selection registers (PxSEL) and Direction registers (PxDIR)

---

## GPIO Port Abstraction

**Driver:** `drivers/gpio/gpio.c/h`

**Abstraction Layer:**
The firmware abstracts 47 GPIO pins into a simple enumeration:

```c
typedef enum {
    GPIO_PIN_0 = 0,   // Port P1.0
    GPIO_PIN_1,       // Port P1.1
    // ...
    GPIO_PIN_7,       // Port P1.7
    GPIO_PIN_8,       // Port P2.0
    // ...
    GPIO_PIN_15,      // Port P2.7
    GPIO_PIN_16,      // Port P3.0
    // ...
    GPIO_PIN_46       // Port P6.6
} gpio_pin_t;
```

**Mapping:**
- `GPIO_PIN_0-7` → Port P1 (P1.0 - P1.7)
- `GPIO_PIN_8-15` → Port P2 (P2.0 - P2.7)
- `GPIO_PIN_16-23` → Port P3 (P3.0 - P3.7)
- `GPIO_PIN_24-31` → Port P4 (P4.0 - P4.7)
- `GPIO_PIN_32-39` → Port P5 (P5.0 - P5.7)
- `GPIO_PIN_40-46` → Port P6 (P6.0 - P6.6)

---

## Detailed Pin Mapping

### Port 1 (P1.0 - P1.7)

| Pin # | MSP430 Pin | GPIO Abstraction | Primary Function | Secondary Function | OBDH Usage | Direction |
|-------|------------|------------------|------------------|--------------------|------------|-----------|
| 64 | P1.0 | GPIO_PIN_0 | GPIO | TA0.1 (Timer) | LED 0 / Heartbeat | Output |
| 63 | P1.1 | GPIO_PIN_1 | GPIO | TA0.2 (Timer) | LED 1 / Activity | Output |
| 62 | P1.2 | GPIO_PIN_2 | GPIO | TA0.3 (Timer) | LED 2 / Error | Output |
| 61 | P1.3 | GPIO_PIN_3 | GPIO | TA0.4 (Timer) | Status Pin | Output |
| 60 | P1.4 | GPIO_PIN_4 | GPIO | UCB0SIMO (SPI) | Not used (GPIO) | Input |
| 59 | P1.5 | GPIO_PIN_5 | GPIO | UCB0SOMI (SPI) | Not used (GPIO) | Input |
| 58 | P1.6 | GPIO_PIN_6 | GPIO | UCB0CLK (SPI) | Not used (GPIO) | Input |
| 57 | P1.7 | GPIO_PIN_7 | GPIO | UCB0STE (SPI) | Not used (GPIO) | Input |

**Notes:**
- P1.0-P1.2: Used for status LEDs
- P1.4-P1.7: Reserved for future SPI expansion or GPIO

---

### Port 2 (P2.0 - P2.7)

| Pin # | MSP430 Pin | GPIO Abstraction | Primary Function | Secondary Function | OBDH Usage | Direction |
|-------|------------|------------------|------------------|--------------------|------------|-----------|
| 10 | P2.0 | GPIO_PIN_8 | GPIO | TA1.1 (Timer) | Antenna Deploy 1 | Output |
| 11 | P2.1 | GPIO_PIN_9 | GPIO | TA1.2 (Timer) | Antenna Deploy 2 | Output |
| 12 | P2.2 | GPIO_PIN_10 | GPIO | TA2.1 (Timer) | Antenna Deploy 3 | Output |
| 13 | P2.3 | GPIO_PIN_11 | GPIO | TA2.2 (Timer) | Antenna Deploy 4 | Output |
| 14 | P2.4 | GPIO_PIN_12 | GPIO | TB0.1 (Timer) | Module Enable 1 | Output |
| 15 | P2.5 | GPIO_PIN_13 | GPIO | TB0.2 (Timer) | Module Enable 2 | Output |
| 16 | P2.6 | GPIO_PIN_14 | GPIO | ACLK Output | Reserved | Input |
| 17 | P2.7 | GPIO_PIN_15 | GPIO | TB0.0 (Timer) | Reserved | Input |

**Notes:**
- P2.0-P2.3: Antenna deployment burn control (typically controlled via ISIS module, but GPIO backup available)
- P2.4-P2.5: Module power enable signals

---

### Port 3 (P3.0 - P3.7)

| Pin # | MSP430 Pin | GPIO Abstraction | Primary Function | Secondary Function | OBDH Usage | Direction |
|-------|------------|------------------|------------------|--------------------|------------|-----------|
| 18 | P3.0 | GPIO_PIN_16 | GPIO | UCA0TXD (UART) | Debug UART TX | Peripheral |
| 19 | P3.1 | GPIO_PIN_17 | GPIO | UCA0RXD (UART) | Debug UART RX | Peripheral |
| 20 | P3.2 | GPIO_PIN_18 | GPIO | UCA0SIMO (SPI) | Reserved | Input |
| 21 | P3.3 | GPIO_PIN_19 | GPIO | UCA0SOMI (SPI) | Reserved | Input |
| 22 | P3.4 | GPIO_PIN_20 | GPIO | UCA0CLK (SPI) | Reserved | Input |
| 23 | P3.5 | GPIO_PIN_21 | GPIO | UCA0STE (SPI) | Reserved | Input |
| 24 | P3.6 | GPIO_PIN_22 | GPIO | UCB0SIMO (SPI) | NOR Flash MOSI | Peripheral |
| 25 | P3.7 | GPIO_PIN_23 | GPIO | UCB0SOMI (SPI) | NOR Flash MISO | Peripheral |

**Notes:**
- P3.0-P3.1: UART0 for debug/logging (115200 bps)
- P3.6-P3.7: SPI for NOR Flash communication (along with P1.6-P1.7 or other SPI pins)

---

### Port 4 (P4.0 - P4.7)

| Pin # | MSP430 Pin | GPIO Abstraction | Primary Function | Secondary Function | OBDH Usage | Direction |
|-------|------------|------------------|------------------|--------------------|------------|-----------|
| 26 | P4.0 | GPIO_PIN_24 | GPIO | A0 (ADC) | Voltage Sense | Peripheral (ADC) |
| 27 | P4.1 | GPIO_PIN_25 | GPIO | A1 (ADC) | Current Sense | Peripheral (ADC) |
| 28 | P4.2 | GPIO_PIN_26 | GPIO | A2 (ADC) | Temperature Ext | Peripheral (ADC) |
| 29 | P4.3 | GPIO_PIN_27 | GPIO | A3 (ADC) | Reserved ADC | Peripheral (ADC) |
| 30 | P4.4 | GPIO_PIN_28 | GPIO | A4 (ADC) | Reserved ADC | Peripheral (ADC) |
| 31 | P4.5 | GPIO_PIN_29 | GPIO | A5 (ADC) | Reserved ADC | Peripheral (ADC) |
| 32 | P4.6 | GPIO_PIN_30 | GPIO | A6 (ADC) | Reserved ADC | Peripheral (ADC) |
| 33 | P4.7 | GPIO_PIN_31 | GPIO | A7 (ADC) | Reserved ADC | Peripheral (ADC) |

**Notes:**
- P4.0-P4.7: ADC inputs for analog sensors
- P4.0: Input voltage measurement (via voltage divider)
- P4.1: Input current measurement (via current sense circuit)
- P4.2: External temperature sensor (if analog)

---

### Port 5 (P5.0 - P5.7)

| Pin # | MSP430 Pin | GPIO Abstraction | Primary Function | Secondary Function | OBDH Usage | Direction |
|-------|------------|------------------|------------------|--------------------|------------|-----------|
| 34 | P5.0 | GPIO_PIN_32 | GPIO | A8 (ADC) | Reserved | Input |
| 35 | P5.1 | GPIO_PIN_33 | GPIO | A9 (ADC) | Reserved | Input |
| 36 | P5.2 | GPIO_PIN_34 | GPIO | XT2IN | Crystal Osc (32MHz) | Peripheral |
| 37 | P5.3 | GPIO_PIN_35 | GPIO | XT2OUT | Crystal Osc (32MHz) | Peripheral |
| 38 | P5.4 | GPIO_PIN_36 | GPIO | UCB1SIMO (SPI) | I2C SDA (I2C Mode) | Peripheral |
| 39 | P5.5 | GPIO_PIN_37 | GPIO | UCB1SOMI (SPI) | I2C SCL (I2C Mode) | Peripheral |
| 40 | P5.6 | GPIO_PIN_38 | GPIO | UCB1CLK (SPI) | Reserved | Input |
| 41 | P5.7 | GPIO_PIN_39 | GPIO | UCB1STE (SPI) | Reserved | Input |

**Notes:**
- P5.2-P5.3: External crystal oscillator (XT2) for 32 MHz main clock
- P5.4-P5.5: I2C Port 1 (UCB1) for EPS, TTC, sensors
  - P5.4: SDA (Serial Data)
  - P5.5: SCL (Serial Clock)

---

### Port 6 (P6.0 - P6.6)

| Pin # | MSP430 Pin | GPIO Abstraction | Primary Function | Secondary Function | OBDH Usage | Direction |
|-------|------------|------------------|------------------|--------------------|------------|-----------|
| 42 | P6.0 | GPIO_PIN_40 | GPIO | A0 (ADC) | CS NOR Flash | Output |
| 43 | P6.1 | GPIO_PIN_41 | GPIO | A1 (ADC) | CS FRAM | Output |
| 44 | P6.2 | GPIO_PIN_42 | GPIO | A2 (ADC) | CS Antenna | Output |
| 45 | P6.3 | GPIO_PIN_43 | GPIO | A3 (ADC) | Reserved CS | Output |
| 46 | P6.4 | GPIO_PIN_44 | GPIO | A4 (ADC) | Interrupt In | Input |
| 47 | P6.5 | GPIO_PIN_45 | GPIO | A5 (ADC) | Reserved | Input |
| 48 | P6.6 | GPIO_PIN_46 | GPIO | A6 (ADC) | Reserved | Input |

**Notes:**
- P6.0-P6.3: Chip Select (CS) pins for SPI devices
  - P6.0: NOR Flash CS (active low)
  - P6.1: FRAM CS (active low)
  - P6.2: ISIS Antenna CS (active low, if SPI mode)
- P6.4: Interrupt input from external devices

---

## UART Pin Assignments

### UART 0 (eUSCI_A0) - Debug/Logging

| Signal | MSP430 Pin | Pin # | Direction | Usage |
|--------|------------|-------|-----------|-------|
| TXD | P3.0 (UCA0TXD) | 18 | Output | Debug UART TX |
| RXD | P3.1 (UCA0RXD) | 19 | Input | Debug UART RX |

**Configuration:** 115200 bps, 8N1
**Voltage:** 3.3V CMOS
**Connection:** USB-to-UART adapter or FTDI cable

---

### UART 1 (eUSCI_A1) - EDC Payload

| Signal | MSP430 Pin | Pin # | Direction | Usage |
|--------|------------|-------|-----------|-------|
| TXD | P4.4 (UCA1TXD) | 30 | Output | EDC UART TX |
| RXD | P4.5 (UCA1RXD) | 31 | Input | EDC UART RX |

**Configuration:** 115200 bps (configurable), 8N1
**Voltage:** 3.3V CMOS

---

### UART 2 (eUSCI_A2) - Reserved

| Signal | MSP430 Pin | Pin # | Direction | Usage |
|--------|------------|-------|-----------|-------|
| TXD | P9.4 (UCA2TXD) | TBD | Output | Reserved |
| RXD | P9.5 (UCA2RXD) | TBD | Input | Reserved |

---

## SPI Pin Assignments

### SPI 0 (eUSCI_B0) - NOR Flash & FRAM

| Signal | MSP430 Pin | Pin # | Direction | Usage |
|--------|------------|-------|-----------|-------|
| MOSI (SIMO) | P3.6 (UCB0SIMO) | 24 | Output | SPI Master Out |
| MISO (SOMI) | P3.7 (UCB0SOMI) | 25 | Input | SPI Master In |
| SCK (CLK) | P1.6 (UCB0CLK) | 58 | Output | SPI Clock |
| CS NOR | P6.0 (GPIO) | 42 | Output | NOR Flash CS (low active) |
| CS FRAM | P6.1 (GPIO) | 43 | Output | FRAM CS (low active) |

**Clock Speed:** 1 MHz (configurable up to 16 MHz)
**Mode:** SPI Mode 0 (CPOL=0, CPHA=0)
**Voltage:** 3.3V CMOS

---

### SPI 1 (eUSCI_B1) - Reserved / Antenna (SPI Mode)

| Signal | MSP430 Pin | Pin # | Direction | Usage |
|--------|------------|-------|-----------|-------|
| MOSI (SIMO) | P5.4 (UCB1SIMO) | 38 | Output | SPI Master Out |
| MISO (SOMI) | P5.5 (UCB1SOMI) | 39 | Input | SPI Master In |
| SCK (CLK) | P5.6 (UCB1CLK) | 40 | Output | SPI Clock |
| CS Antenna | P6.2 (GPIO) | 44 | Output | Antenna CS (low active) |

**Note:** UCB1 can also be configured as I2C (see I2C section below)

---

## I2C Pin Assignments

### I2C 0 (eUSCI_B1) - EPS, TTC, Sensors

| Signal | MSP430 Pin | Pin # | Function | Usage |
|--------|------------|-------|----------|-------|
| SDA | P5.4 (UCB1SDA) | 38 | Bidirectional | I2C Data |
| SCL | P5.5 (UCB1SCL) | 39 | Output | I2C Clock |

**Configuration:**
- **Clock Speed:** 100 kHz (standard) or 400 kHz (fast mode)
- **Pull-ups:** External 4.7 kΩ resistors to 3.3V
- **Voltage:** 3.3V

**Connected Devices:**
- EPS (SpaceLab EPS v2): I2C Address 0x36
- TTC 0 (SpaceLab TTC v2): I2C Address 0x40
- TTC 1 (SpaceLab TTC v2): I2C Address 0x50
- Temperature Sensors: I2C Address 0x48-0x4F
- ISIS Antenna (I2C Mode): I2C Address 0x31-0x33

---

### I2C 1 (eUSCI_B2) - Reserved

| Signal | MSP430 Pin | Pin # | Function | Usage |
|--------|------------|-------|----------|-------|
| SDA | P9.1 (UCB2SDA) | TBD | Bidirectional | Reserved |
| SCL | P9.2 (UCB2SCL) | TBD | Output | Reserved |

---

## ADC Pin Assignments

### ADC Channels (ADC12_A Module)

| Channel | MSP430 Pin | Pin # | Usage | Input Range |
|---------|------------|-------|-------|-------------|
| A0 | P4.0 (A0) | 26 | Input Voltage (via divider) | 0-6.6V (2:1 divider) |
| A1 | P4.1 (A1) | 27 | Input Current (via sense) | 0-2A (typical) |
| A2 | P4.2 (A2) | 28 | External Temperature | 0-3.3V |
| A3-A7 | P4.3-P4.7 | 29-33 | Reserved ADC | 0-3.3V |
| A8-A9 | P5.0-P5.1 | 34-35 | Reserved ADC | 0-3.3V |
| A10 | Internal | - | Internal Temperature | MSP430 µC temp |

**Reference Voltage:** 2.5V internal reference
**Resolution:** 12-bit (0-4095)

---

## Special Function Pins

### JTAG Interface

| Signal | MSP430 Pin | Pin # | Function |
|--------|------------|-------|----------|
| TDI | TDI | 3 | JTAG Data In |
| TDO | TDO/TDI | 4 | JTAG Data Out |
| TMS | TMS | 5 | JTAG Mode Select |
| TCK | TCK | 6 | JTAG Clock |
| RST | RST/NMI/SBWTDIO | 83 | Reset / JTAG |
| TEST | TEST/SBWTCK | 84 | Test / JTAG Clock |

**Programmer:** Texas Instruments MSP-FET or compatible
**Connection:** 14-pin JTAG header (standard TI layout)

---

### Clock Pins

| Signal | MSP430 Pin | Pin # | Function |
|--------|------------|-------|----------|
| XT1IN | P7.0 (XT1IN) | 74 | 32.768 kHz Crystal In |
| XT1OUT | P7.1 (XT1OUT) | 75 | 32.768 kHz Crystal Out |
| XT2IN | P5.2 (XT2IN) | 36 | 32 MHz Crystal In |
| XT2OUT | P5.3 (XT2OUT) | 37 | 32 MHz Crystal Out |

**Crystals:**
- **XT1:** 32.768 kHz (for RTC and ACLK)
- **XT2:** 32 MHz (for MCLK and SMCLK via FLL)

**Load Capacitors:**
- XT1: 12 pF (typical)
- XT2: 18 pF (typical)

---

### Power Pins

| Signal | Pin # | Function | Voltage |
|--------|-------|----------|---------|
| DVCC | Multiple | Digital Power Supply | 3.3V |
| DVSS | Multiple | Digital Ground | 0V (GND) |
| AVCC | 9 | Analog Power Supply | 3.3V |
| AVSS | 8 | Analog Ground | 0V (GND) |

**Decoupling Capacitors:**
- 100 nF ceramic capacitor per DVCC pin (close to IC)
- 10 µF tantalum capacitor per power domain
- 100 nF ceramic + 10 µF tantalum for AVCC

---

## Chip Select Summary

| Device | CS Pin | MSP430 Pin | Pin # | Active Level |
|--------|--------|------------|-------|--------------|
| NOR Flash (MT25Q) | CS_NOR | P6.0 | 42 | Low |
| FRAM (CY15x102QN) | CS_FRAM | P6.1 | 43 | Low |
| ISIS Antenna (SPI) | CS_ANT | P6.2 | 44 | Low |
| Reserved | CS_RES | P6.3 | 45 | Low |

**Usage:**
```c
// Select NOR Flash
gpio_clear(GPIO_PIN_40);  // CS low
spi_transfer(...);
gpio_set(GPIO_PIN_40);    // CS high

// Select FRAM
gpio_clear(GPIO_PIN_41);  // CS low
spi_transfer(...);
gpio_set(GPIO_PIN_41);    // CS high
```

---

## LED Assignments

| LED | GPIO Pin | MSP430 Pin | Pin # | Usage |
|-----|----------|------------|-------|-------|
| LED 0 | GPIO_PIN_0 | P1.0 | 64 | Heartbeat (1 Hz blink) |
| LED 1 | GPIO_PIN_1 | P1.1 | 63 | Communication Activity |
| LED 2 | GPIO_PIN_2 | P1.2 | 62 | Error Indicator |

**Driver:** Active high (LED on when GPIO high)
**Current Limiting:** 330Ω-1kΩ resistor in series
**Current:** ~2-10 mA per LED

---

## Interrupt Pins

| Signal | GPIO Pin | MSP430 Pin | Pin # | Source | Trigger |
|--------|----------|------------|-------|--------|---------|
| INT_EXT | GPIO_PIN_44 | P6.4 | 46 | External Device | Falling Edge |

**Configuration:**
```c
// Enable interrupt on P6.4 (falling edge)
gpio_enable_interrupt(GPIO_PIN_44, GPIO_INT_FALLING_EDGE, interrupt_handler);

void interrupt_handler(void) {
    // Handle interrupt
    // Clear interrupt flag
    gpio_clear_interrupt(GPIO_PIN_44);
}
```

---

## Pin Configuration Examples

### Configure GPIO Output (LED)

```c
#include "gpio/gpio.h"

// Configure P1.0 as output
gpio_set_mode(GPIO_PIN_0, GPIO_MODE_OUTPUT);

// Set high (LED on)
gpio_set(GPIO_PIN_0);

// Set low (LED off)
gpio_clear(GPIO_PIN_0);

// Toggle
gpio_toggle(GPIO_PIN_0);
```

---

### Configure GPIO Input with Pull-up

```c
// Configure P6.4 as input with pull-up
gpio_set_mode(GPIO_PIN_44, GPIO_MODE_INPUT);
gpio_set_pull(GPIO_PIN_44, GPIO_PULL_UP);

// Read state
uint8_t state = gpio_read(GPIO_PIN_44);
```

---

### Configure UART Pins

```c
// UCA0 (UART 0) on P3.0 (TX) and P3.1 (RX)
// Set pin function to peripheral (UART)
P3SEL |= BIT0 | BIT1;  // Enable UART function

// Initialize UART
uart_init(UART_PORT_0, 115200);
```

---

### Configure SPI Pins

```c
// UCB0 (SPI 0) on P3.6 (MOSI), P3.7 (MISO), P1.6 (SCK)
// Set pin function to peripheral (SPI)
P3SEL |= BIT6 | BIT7;
P1SEL |= BIT6;

// CS pins as GPIO output
gpio_set_mode(GPIO_PIN_40, GPIO_MODE_OUTPUT);  // NOR Flash CS
gpio_set_mode(GPIO_PIN_41, GPIO_MODE_OUTPUT);  // FRAM CS
gpio_set(GPIO_PIN_40);  // CS high (inactive)
gpio_set(GPIO_PIN_41);  // CS high (inactive)

// Initialize SPI
spi_init(SPI_PORT_0, 1000000);  // 1 MHz
```

---

### Configure I2C Pins

```c
// UCB1 (I2C 0) on P5.4 (SDA) and P5.5 (SCL)
// Set pin function to peripheral (I2C)
P5SEL |= BIT4 | BIT5;

// Initialize I2C
i2c_init(I2C_PORT_0, 100000);  // 100 kHz
```

---

### Configure ADC Pins

```c
// P4.0 as ADC input (A0)
// Set pin function to peripheral (ADC)
P4SEL |= BIT0;

// Initialize ADC
adc_init();

// Read channel A0
uint16_t raw_value = adc_read_channel(ADC_CHANNEL_0);
```

---

## Electrical Characteristics

### GPIO Specifications

| Parameter | Min | Typ | Max | Unit |
|-----------|-----|-----|-----|------|
| Output High Voltage (VOH) | 2.4 | 3.0 | 3.3 | V |
| Output Low Voltage (VOL) | 0 | 0.1 | 0.4 | V |
| Input High Voltage (VIH) | 2.0 | - | 3.6 | V |
| Input Low Voltage (VIL) | -0.3 | - | 0.8 | V |
| Output Current (per pin) | - | - | 6 | mA |
| Input Leakage Current | - | - | ±50 | nA |

**Note:** Maximum total current for all GPIO pins: 48 mA

---

### Pull-up/Pull-down Resistors

| Type | Resistance | Usage |
|------|------------|-------|
| Internal Pull-up | 20-50 kΩ | Weak pull-up for inputs |
| Internal Pull-down | 20-50 kΩ | Weak pull-down for inputs |
| External Pull-up (I2C) | 4.7 kΩ | Strong pull-up for I2C bus |

---

## Pin Mapping Summary Table

| Port | Pin Range | Primary Function | OBDH Usage |
|------|-----------|------------------|------------|
| P1 | P1.0-P1.7 | GPIO, Timer | LEDs, Status |
| P2 | P2.0-P2.7 | GPIO, Timer | Antenna Deploy, Module Control |
| P3 | P3.0-P3.7 | UART, SPI | Debug UART, SPI (NOR/FRAM) |
| P4 | P4.0-P4.7 | ADC, UART | Voltage/Current/Temp Sensing |
| P5 | P5.0-P5.7 | I2C, SPI, Crystal | I2C (EPS/TTC), 32 MHz Crystal |
| P6 | P6.0-P6.6 | GPIO, ADC | SPI Chip Selects, Interrupts |
| P7 | P7.0-P7.1 | Crystal | 32.768 kHz Crystal (RTC) |

---

## Related Documentation

- `00_functionalities_of_system.md` - System overview
- `01_firmware_architecture.md` - Firmware architecture
- `02_datapath.md` - Data structures
- `03_interfaces.md` - Communication interfaces
- `04_tasks.md` - Task descriptions
- `06_hardware_components_and_architecture.md` - Hardware details
