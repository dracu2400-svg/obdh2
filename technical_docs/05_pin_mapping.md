# OBDH2 Pin Mapping and GPIO Configuration

## Overview

This document details the pin assignments and GPIO configuration for the OBDH2 system based on the MSP430F6659 microcontroller. It includes peripheral pin mappings, hardware version detection, and special function assignments.

**Microcontroller**: MSP430F6659
**Total GPIO Pins**: 70 (GPIO_PIN_0 through GPIO_PIN_69)
**Ports**: 11 ports (P1-P11), 8 pins per port typical

## Hardware Version Detection

**Location**: firmware/system/system.c:1

### Version Detection Pins

The OBDH2 uses two GPIO pins to detect the hardware revision:

| Pin Name | GPIO Number | Port.Pin | Function |
|----------|-------------|----------|----------|
| HW_VERSION_BIT0 | GPIO_PIN_14 | TBD | Hardware version bit 0 |
| HW_VERSION_BIT1 | GPIO_PIN_15 | TBD | Hardware version bit 1 |

### Hardware Version Encoding

```
┌─────────┬───────┬────────────────────┐
│ Bit 1   │ Bit 0 │ Hardware Version   │
├─────────┼───────┼────────────────────┤
│ 0       │ 0     │ HW_VERSION_0       │
│ 0       │ 1     │ HW_VERSION_1       │
│ 1       │ 0     │ HW_VERSION_2       │
│ 1       │ 1     │ HW_VERSION_3       │
└─────────┴───────┴────────────────────┘
```

### Version Detection Code

**Location**: firmware/system/system.c:1

```c
uint8_t system_get_hw_version(void) {
    gpio_init(GPIO_PIN_14, GPIO_INPUT_PULLUP);
    gpio_init(GPIO_PIN_15, GPIO_INPUT_PULLUP);

    uint8_t bit0 = gpio_read(GPIO_PIN_14);
    uint8_t bit1 = gpio_read(GPIO_PIN_15);

    return (bit1 << 1) | bit0;
}
```

**Pin Configuration**:
- **Mode**: Input with pull-up resistor
- **Logic**: Active low (0 = set, 1 = not set)
- **Read at**: System initialization (startup task)

## SPI Pin Assignments

**Location**: firmware/drivers/spi/spi.c:1

### SPI Port 0

**Connected Devices**:
- SL_TTC2 Radio 0 (primary radio transceiver)
- SL_TTC2 Radio 1 (backup radio transceiver)
- MT25Q NOR Flash (non-volatile storage)

#### SPI Port 0 Signal Pins

| Signal | GPIO Number | Port.Pin | Function | Direction |
|--------|-------------|----------|----------|-----------|
| MOSI | GPIO_PIN_0 | TBD | Master Out Slave In | Output |
| MISO | GPIO_PIN_4 | TBD | Master In Slave Out | Input |
| CLK | GPIO_PIN_5 | TBD | SPI Clock | Output |

**Clock Speed**: 1 MHz (default)
**Mode**: SPI Mode 0 (CPOL=0, CPHA=0)

#### SPI Port 0 Chip Select Pins

| Device | CS Name | GPIO Number | Port.Pin | Active |
|--------|---------|-------------|----------|--------|
| TTC Radio 0 | SPI_CS_0 | GPIO_PIN_5 | TBD | Low |
| TTC Radio 1 | SPI_CS_1 | GPIO_PIN_6 | TBD | Low |
| MT25Q Flash | SPI_CS_2 | GPIO_PIN_28 | TBD | Low |
| Expansion | SPI_CS_3 | GPIO_PIN_45 | TBD | Low |

**Note**: Chip select pins are driven low (active) during SPI transactions.

### SPI Port 1-5

**Status**: Available for expansion
**Potential Uses**:
- Additional payload interfaces
- External sensors
- Expansion modules

**Pin assignments**: Defined in firmware/drivers/spi/spi.c:1 (implementation-specific)

## I2C Pin Assignments

**Location**: firmware/drivers/i2c/i2c.c:1

### I2C Port 0

**Connected Device**: SpaceLab EPS 2.0 (Power Subsystem)

| Signal | GPIO Number | Port.Pin | Function | Direction |
|--------|-------------|----------|----------|-----------|
| SDA | TBD | TBD | Serial Data | Bidirectional |
| SCL | TBD | TBD | Serial Clock | Output |

**Slave Address**: 0x36
**Clock Speed**: 400 kHz (Fast mode)

### I2C Port 1

**Connected Device**: ISIS Antenna Deployment System

| Signal | GPIO Number | Port.Pin | Function | Direction |
|--------|-------------|----------|----------|-----------|
| SDA | TBD | TBD | Serial Data | Bidirectional |
| SCL | TBD | TBD | Serial Clock | Output |

**Slave Address**: 0x31-0x33 (varies by antenna configuration)
**Clock Speed**: 100 kHz (Standard mode for compatibility)

### I2C Port 2

**Status**: Available for expansion
**Potential Uses**:
- External sensors (magnetometer, gyroscope)
- Additional subsystem modules

| Signal | GPIO Number | Port.Pin | Function | Direction |
|--------|-------------|----------|----------|-----------|
| SDA | TBD | TBD | Serial Data | Bidirectional |
| SCL | TBD | TBD | Serial Clock | Output |

## UART Pin Assignments

**Location**: firmware/drivers/uart/uart.c:1

### UART Port 0

**Purpose**: System debug logging, diagnostic output

| Signal | GPIO Number | Port.Pin | Function | Direction |
|--------|-------------|----------|----------|-----------|
| TX | TBD | TBD | Transmit Data | Output |
| RX | TBD | TBD | Receive Data | Input |

**Configuration**:
- **Baud Rate**: 115200 bps
- **Data Bits**: 8
- **Parity**: None
- **Stop Bits**: 1

**Usage**: firmware/system/sys_log/sys_log.c:1

### UART Port 1

**Status**: Available for expansion
**Potential Uses**:
- Payload communication
- Ground support equipment interface

| Signal | GPIO Number | Port.Pin | Function | Direction |
|--------|-------------|----------|----------|-----------|
| TX | TBD | TBD | Transmit Data | Output |
| RX | TBD | TBD | Receive Data | Input |

### UART Port 2

**Status**: Available for expansion

| Signal | GPIO Number | Port.Pin | Function | Direction |
|--------|-------------|----------|----------|-----------|
| TX | TBD | TBD | Transmit Data | Output |
| RX | TBD | TBD | Receive Data | Input |

## ADC Pin Assignments

### ADC Channels

**Peripheral**: MSP430 ADC12_A

| Channel | Signal | GPIO/Pin | Sensor | Measurement |
|---------|--------|----------|--------|-------------|
| ADC_CH_TEMP | TBD | TBD | Temperature Sensor | MCU die temperature |
| ADC_CH_VOLTAGE | TBD | TBD | Voltage Sensor | Input voltage monitoring |
| ADC_CH_CURRENT | TBD | TBD | Current Sensor | Input current monitoring |

**Resolution**: 12-bit (0-4095)
**Reference Voltage**: 2.5V or 3.3V (configurable)

**Device Layer**:
- firmware/devices/temp_sensor/temp_sensor.c:1
- firmware/devices/voltage_sensor/voltage_sensor.c:1
- firmware/devices/current_sensor/current_sensor.c:1

## LED Assignments

**Location**: firmware/devices/leds/leds.c:1

### Status LEDs

| LED Name | GPIO Number | Port.Pin | Function | Active |
|----------|-------------|----------|----------|--------|
| LED_HEARTBEAT | GPIO_PIN_20 | TBD | System alive indicator | High |
| LED_STATUS_0 | TBD | TBD | General status | High |
| LED_STATUS_1 | TBD | TBD | General status | High |

**Heartbeat LED**:
- **Frequency**: 1 Hz (1-second toggle)
- **Task**: firmware/app/tasks/heartbeat.c:1
- **Purpose**: Visual confirmation of system operation

## Watchdog Pin

**Location**: firmware/drivers/tps382x/tps382x.c:1

### External Watchdog Timer (TPS382x)

| Signal | GPIO Number | Port.Pin | Function | Active |
|--------|-------------|----------|----------|--------|
| WDI (Watchdog Input) | TBD | TBD | Watchdog reset signal | Toggle |

**Operation**:
- **Refresh Rate**: Every 500 ms
- **Task**: firmware/app/tasks/watchdog_reset.c:1
- **Timeout Period**: ~500-600 ms (if not refreshed)
- **Action on Timeout**: System reset

**Control Method**: GPIO toggle on WDI pin

## Clock Configuration Pins

**Location**: firmware/system/clocks.c:1

### External Crystal

| Signal | GPIO/Pin | Function | Frequency |
|--------|----------|----------|-----------|
| XIN | TBD | Crystal input | 32.768 kHz |
| XOUT | TBD | Crystal output | 32.768 kHz |

**Purpose**: Provides stable ACLK source for FreeRTOS tick generation and RTC

**Configuration**:
- **ACLK**: 32.768 kHz (from external crystal)
- **MCLK**: 32 MHz (from DCO with FLL)
- **SMCLK**: 32 MHz (from DCO)

## Special Function Pins

### Reset Pin

| Signal | GPIO/Pin | Function | Active |
|--------|----------|----------|--------|
| RST/NMI | TBD | System reset | Low |

**Reset Sources**:
- Power-on reset (POR)
- Brownout reset (BOR)
- Watchdog timeout
- External reset button
- Software reset

### JTAG/Debug Pins

| Signal | GPIO/Pin | Function |
|--------|----------|----------|
| TDI | TBD | Test Data In |
| TDO | TBD | Test Data Out |
| TMS | TBD | Test Mode Select |
| TCK | TBD | Test Clock |

**Purpose**: Programming and debugging interface
**Tools**: Code Composer Studio (CCS), MSP430 JTAG debugger

## GPIO Abstraction Layer

**Location**: firmware/drivers/gpio/gpio.h:1

### GPIO Pin Numbering

The firmware uses an abstract GPIO pin numbering system (GPIO_PIN_0 to GPIO_PIN_69) that maps to physical MSP430 port pins:

```c
typedef enum {
    GPIO_PIN_0 = 0,
    GPIO_PIN_1,
    GPIO_PIN_2,
    // ... up to
    GPIO_PIN_69 = 69
} gpio_pin_t;
```

### Port Mapping

**MSP430F6659 Ports**:
- **Port 1**: P1.0 - P1.7 (8 pins)
- **Port 2**: P2.0 - P2.7 (8 pins)
- **Port 3**: P3.0 - P3.7 (8 pins)
- **Port 4**: P4.0 - P4.7 (8 pins)
- **Port 5**: P5.0 - P5.7 (8 pins)
- **Port 6**: P6.0 - P6.7 (8 pins)
- **Port 7**: P7.0 - P7.7 (8 pins)
- **Port 8**: P8.0 - P8.7 (8 pins)
- **Port 9**: P9.0 - P9.7 (6 pins available)
- **Port 10**: P10.0 - P10.7 (varies)
- **Port 11**: P11.0 - P11.7 (varies)

**Total**: 70 GPIO pins (some ports have fewer than 8 pins)

### GPIO Modes

```c
typedef enum {
    GPIO_INPUT,             // High-impedance input
    GPIO_OUTPUT,            // Push-pull output
    GPIO_INPUT_PULLUP,      // Input with pull-up resistor
    GPIO_INPUT_PULLDOWN     // Input with pull-down resistor
} gpio_mode_t;
```

## Peripheral Function Multiplexing

Many MSP430 pins support multiple functions. The firmware configures pins based on enabled peripherals:

### Multiplexing Priority

1. **Primary Function**: SPI, I2C, UART (when enabled)
2. **Secondary Function**: ADC, Timer outputs
3. **GPIO**: General-purpose I/O (default)

### Pin Configuration Example

```c
// Configure pin for SPI (primary function)
gpio_init(GPIO_PIN_0, GPIO_FUNCTION_PRIMARY);  // MOSI

// Configure pin for GPIO (default function)
gpio_init(GPIO_PIN_20, GPIO_OUTPUT);  // LED
```

## Hardware-Specific Pin Maps

### ISIS Antenna Pin Map

**Location**: firmware/drivers/isis_antenna/isis_antenna_pinmap.h:1

The ISIS antenna driver may include a specific pin map file defining:
- I2C port assignment
- SDA/SCL pins
- Debug GPIO pins
- Deployment status indicators

**Example Structure**:
```c
#define ISIS_ANT_I2C_PORT       I2C_PORT_1
#define ISIS_ANT_SDA_PIN        TBD
#define ISIS_ANT_SCL_PIN        TBD
#define ISIS_ANT_STATUS_PIN     TBD  // Optional status LED
```

## Pin Configuration at Startup

**Location**: firmware/app/tasks/startup.c:1

During system initialization, the startup task configures all pins:

```c
void task_startup(void *pvParameters) {
    // 1. Hardware version detection
    gpio_init(GPIO_PIN_14, GPIO_INPUT_PULLUP);
    gpio_init(GPIO_PIN_15, GPIO_INPUT_PULLUP);

    // 2. LED initialization
    leds_init();  // Configures LED pins as outputs

    // 3. Peripheral initialization
    spi_init(SPI_PORT_0, ...);   // Configures SPI pins
    i2c_init(I2C_PORT_0, ...);   // Configures I2C pins
    uart_init(UART_PORT_0, ...); // Configures UART pins

    // 4. Watchdog pin
    watchdog_init();  // Configures watchdog GPIO

    // 5. Sensor initialization
    temp_sensor_init();     // Configures ADC channel
    voltage_sensor_init();  // Configures ADC channel
    current_sensor_init();  // Configures ADC channel

    // ... additional device initialization
}
```

## Pin State Summary

### Power-On State

After power-on reset, all MSP430 pins default to:
- **Mode**: High-impedance input
- **Pull resistors**: Disabled
- **Function**: GPIO (not peripheral function)

### Configured State

After firmware initialization:

| Pin Type | Count | Mode | Function |
|----------|-------|------|----------|
| SPI (MOSI, MISO, CLK) | 3+ | Peripheral | SPI communication |
| SPI Chip Selects | 4+ | Output | Device selection |
| I2C (SDA, SCL) | 4+ | Peripheral | I2C communication |
| UART (TX, RX) | 2+ | Peripheral | Debug logging |
| ADC Inputs | 3+ | Analog | Sensor measurements |
| LEDs | 2+ | Output | Status indication |
| Watchdog | 1 | Output | Watchdog refresh |
| HW Version | 2 | Input (pull-up) | Version detection |
| Unused | Variable | Input | Reserved for expansion |

## Pin Safety and Protection

### Input Protection

- **ESD Protection**: Built-in on all MSP430 pins
- **Overvoltage Protection**: Clamp diodes to VCC and GND
- **Maximum Voltage**: VCC + 0.3V (absolute maximum)

### Output Limits

- **Maximum Current (per pin)**: 6 mA typical, 48 mA absolute max
- **Maximum Current (total)**: 48 mA per port
- **Drive Strength**: Configurable (full or reduced drive)

### Level Shifting

**TCA4311A I2C Level Shifter** (firmware/drivers/tca4311a/):
- Bridges I2C voltage domains
- Transparent to software
- Provides voltage translation between 3.3V and 5V devices

## Pin Conflict Resolution

### Conflict Detection

- **Compile-time checks**: Verify unique pin assignments
- **Initialization validation**: Ensure no conflicting configurations

### Conflict Examples

**Invalid Configuration**:
```c
// ERROR: Same pin assigned to two functions
spi_init(SPI_PORT_0, ...);      // Uses GPIO_PIN_0 as MOSI
gpio_init(GPIO_PIN_0, OUTPUT);  // Conflicts with SPI
```

**Valid Configuration**:
```c
// OK: Different pins for different functions
spi_init(SPI_PORT_0, ...);      // Uses GPIO_PIN_0
gpio_init(GPIO_PIN_20, OUTPUT); // LED on different pin
```

## Pin Documentation Sources

### Hardware Schematics

**Location**: hardware/ directory (if available)

Detailed pin mappings should reference:
- OBDH2 hardware schematics (PDF)
- PCB layout files
- Component datasheets

### MSP430F6659 Datasheet

**Texas Instruments Document**: SLAS645 (MSP430F6659 Datasheet)

**Sections**:
- Pin Functions and Descriptions
- Port Schematics
- Electrical Characteristics
- Alternate Function Mapping

### Driver Documentation

Each driver may include pin-specific documentation:
- firmware/drivers/spi/spi.h:1
- firmware/drivers/i2c/i2c.h:1
- firmware/drivers/uart/uart.h:1
- firmware/drivers/gpio/gpio.h:1

## Configuration Tools

### Code Composer Studio (CCS)

CCS provides pin multiplexing configuration tools:
- **PinMux**: Visual pin configuration
- **Resource Explorer**: Pin assignment conflicts detection
- **Initialization Code Generation**: Auto-generate pin setup code

### Manual Configuration

Pin configuration is performed in driver initialization:

```c
// Example: Configure P1.0 as SPI MOSI
P1SEL |= BIT0;   // Select peripheral function
P1DIR |= BIT0;   // Set direction to output

// Example: Configure P1.4 as GPIO input with pull-up
P1SEL &= ~BIT4;  // Select GPIO function
P1DIR &= ~BIT4;  // Set direction to input
P1REN |= BIT4;   // Enable pull resistor
P1OUT |= BIT4;   // Select pull-up (not pull-down)
```

## Summary

The OBDH2 pin mapping provides:

1. **Hardware Version Detection**: 2 GPIO pins for hardware revision identification
2. **SPI Communication**: 3+ signal pins + 4+ chip selects for radio and flash
3. **I2C Communication**: 2 ports (EPS, Antenna) + 1 expansion port
4. **UART Communication**: 1 debug port + 2 expansion ports
5. **ADC Inputs**: 3+ channels for temperature, voltage, current sensors
6. **Status LEDs**: 2+ indicators for system operation
7. **Watchdog Control**: 1 GPIO for external watchdog refresh
8. **Clock Sources**: External 32.768 kHz crystal for RTC

**Total Pin Usage**: ~20-30 pins configured, ~40-50 pins available for expansion

**Note**: Specific MSP430 port.pin mappings (e.g., P1.0, P2.3) should be documented in hardware schematics or pinmap.h files. The GPIO abstraction layer (GPIO_PIN_0 through GPIO_PIN_69) provides software portability across hardware revisions.
