# OBDH2 Communication Interfaces

## Overview

The OBDH2 system utilizes multiple communication interfaces to interact with satellite subsystems, sensors, and external modules. This document provides detailed specifications for each interface type, including UART, SPI, I2C, and GPIO.

## Interface Summary

| Interface | Count | Speed | Primary Use |
|-----------|-------|-------|-------------|
| UART | 3 ports | Up to 115200 bps | Debug logging, expansion |
| SPI | 6 ports | 1 MHz default | TTC radios, NOR flash, payloads |
| I2C | 3 ports | 100/400 kHz | EPS, antenna, sensors |
| GPIO | 70 pins | N/A | Control signals, version detection |
| ADC | Multiple channels | 12-bit | Sensor measurements |

## UART Interface

**Location**: firmware/drivers/uart/uart.c:1

### Hardware

- **Peripheral**: MSP430 USCI_A modules
- **Supported Ports**: UART_PORT_0, UART_PORT_1, UART_PORT_2
- **Data Bits**: 8 (fixed)
- **Stop Bits**: Configurable (1 or 2)
- **Parity**: Configurable (None, Even, Odd)

### Configuration

```c
// UART initialization
int uart_init(uart_port_t port, uart_config_t config);

typedef struct {
    uint32_t baudrate;      // Baud rate (bps)
    uint8_t data_bits;      // Data bits (8)
    uint8_t parity;         // UART_PARITY_NONE/EVEN/ODD
    uint8_t stop_bits;      // 1 or 2
} uart_config_t;
```

### Supported Baud Rates

| Baud Rate | Use Case |
|-----------|----------|
| 9600 | Low-speed communication |
| 19200 | Standard communication |
| 38400 | Medium-speed communication |
| 57600 | High-speed communication |
| 115200 | Debug logging (default) |

### UART Port Assignments

#### UART_PORT_0: System Logging

**Configuration**:
- Baud Rate: 115200 bps
- Data: 8 bits
- Parity: None
- Stop: 1 bit

**Purpose**: System debug output, logging messages

**Usage**: firmware/system/sys_log/sys_log.c:1
```c
sys_log(SYS_LOG_INFO, "System initialized");
```

**Output Format**:
```
[INFO] System initialized
[ERROR] Device communication failed
[DEBUG] Task execution: 42 ms
```

#### UART_PORT_1 & UART_PORT_2: Expansion

**Status**: Available for future use
**Potential Uses**:
- Payload communication
- Additional ground support equipment
- Inter-satellite links

### UART API

```c
// Initialize UART port
int uart_init(uart_port_t port, uart_config_t config);

// Write data
int uart_write(uart_port_t port, uint8_t *data, uint16_t len);

// Read data (non-blocking)
int uart_read(uart_port_t port, uint8_t *data, uint16_t len);

// Check data availability
int uart_rx_available(uart_port_t port);

// Flush RX buffer
void uart_flush(uart_port_t port);
```

### Example Usage

```c
// Configure UART for 115200 bps
uart_config_t config = {
    .baudrate = 115200,
    .data_bits = 8,
    .parity = UART_PARITY_NONE,
    .stop_bits = 1
};

uart_init(UART_PORT_0, config);

// Transmit data
uint8_t msg[] = "Hello, Ground Station!";
uart_write(UART_PORT_0, msg, sizeof(msg));

// Receive data
uint8_t rx_buf[64];
if (uart_rx_available(UART_PORT_0)) {
    int len = uart_read(UART_PORT_0, rx_buf, sizeof(rx_buf));
}
```

### Features

- **Interrupt-Driven RX**: Incoming data buffered via interrupts
- **Polling TX**: Blocking transmission
- **RX Buffer**: Configurable buffer size per port
- **Availability Check**: Query bytes in RX buffer before reading

## SPI Interface

**Location**: firmware/drivers/spi/spi.c:1

### Hardware

- **Peripheral**: MSP430 USCI_A/B modules (SPI mode)
- **Supported Ports**: SPI_PORT_0 through SPI_PORT_5 (6 ports)
- **Mode**: Master only
- **Clock Speed**: Configurable (default 1 MHz)
- **Data Bits**: 8 bits per transfer

### SPI Modes

| Mode | Clock Polarity (CPOL) | Clock Phase (CPHA) | Description |
|------|----------------------|-------------------|-------------|
| 0 | 0 | 0 | Idle low, sample on rising edge |
| 1 | 0 | 1 | Idle low, sample on falling edge |
| 2 | 1 | 0 | Idle high, sample on falling edge |
| 3 | 1 | 1 | Idle high, sample on rising edge |

**Default**: MODE_0 (CPOL=0, CPHA=0)

### Chip Select Management

**Multiple Slaves per Port**: Up to 10 chip selects (CS_0 to CS_9)

**Location**: firmware/drivers/spi/spi.c:1

Chip select mapping to GPIO pins:
```c
typedef enum {
    SPI_CS_0 = GPIO_PIN_5,
    SPI_CS_1 = GPIO_PIN_6,
    SPI_CS_2 = GPIO_PIN_28,
    SPI_CS_3 = GPIO_PIN_45,
    // Additional CS pins for other ports
} spi_cs_t;
```

**CS Control**:
- Active low (driven low during transaction)
- Automatically controlled by driver
- Manual control available for custom protocols

### SPI Port Assignments

#### SPI_PORT_0: TTC Radios and NOR Flash

**Connected Devices**:
- **SL_TTC2 Radio 0**: Primary radio transceiver
- **SL_TTC2 Radio 1**: Backup radio transceiver
- **MT25Q NOR Flash**: Non-volatile data storage

**Speed**: 1 MHz
**Mode**: MODE_0

**Device Chip Selects**:
- TTC Radio 0: CS_0
- TTC Radio 1: CS_1
- MT25Q Flash: CS_2

#### SPI_PORT_1 - SPI_PORT_5: Expansion

**Status**: Available for payload and additional modules
**Potential Uses**:
- Payload communication
- Additional storage devices
- External sensors

### SPI API

```c
// Initialize SPI port
int spi_init(spi_port_t port, spi_config_t config);

// Transfer data (full duplex)
int spi_transfer(spi_port_t port, spi_cs_t cs,
                 uint8_t *tx_data, uint8_t *rx_data, uint16_t len);

// Write only (TX)
int spi_write(spi_port_t port, spi_cs_t cs,
              uint8_t *data, uint16_t len);

// Read only (RX, transmit dummy bytes)
int spi_read(spi_port_t port, spi_cs_t cs,
             uint8_t *data, uint16_t len);

// Configuration structure
typedef struct {
    uint32_t speed;         // Clock speed (Hz)
    uint8_t mode;           // SPI mode (0-3)
    uint8_t bit_order;      // MSB_FIRST or LSB_FIRST
} spi_config_t;
```

### Example Usage: TTC Communication

```c
// Initialize SPI port for TTC
spi_config_t spi_cfg = {
    .speed = 1000000,       // 1 MHz
    .mode = SPI_MODE_0,
    .bit_order = SPI_MSB_FIRST
};

spi_init(SPI_PORT_0, spi_cfg);

// Send command to TTC Radio 0
uint8_t cmd[] = {0x20, 0x00};  // Read register command
uint8_t response[2];

spi_transfer(SPI_PORT_0, SPI_CS_0, cmd, response, sizeof(cmd));
```

### Example Usage: MT25Q Flash

```c
// Read flash memory
uint8_t read_cmd[] = {0x03, 0x00, 0x00, 0x00};  // Read from addr 0
uint8_t flash_data[256];

// Send command (4 bytes)
spi_write(SPI_PORT_0, SPI_CS_2, read_cmd, sizeof(read_cmd));

// Read data (256 bytes)
spi_read(SPI_PORT_0, SPI_CS_2, flash_data, sizeof(flash_data));
```

### Features

- **Full-Duplex Communication**: Simultaneous TX and RX
- **Multi-Slave Support**: Multiple chip selects per port
- **Configurable Speed**: Adjustable clock frequency
- **Mode Selection**: Support for all four SPI modes
- **Atomic Transactions**: CS asserted for entire transfer

## I2C Interface

**Location**: firmware/drivers/i2c/i2c.c:1

### Hardware

- **Peripheral**: MSP430 USCI_B modules (I2C mode)
- **Supported Ports**: I2C_PORT_0, I2C_PORT_1, I2C_PORT_2
- **Mode**: Master only
- **Addressing**: 7-bit slave addresses
- **Clock Speeds**: 100 kHz (Standard), 400 kHz (Fast)

### Speed Modes

| Mode | Speed | Description |
|------|-------|-------------|
| Standard | 100 kHz | Low-speed I2C |
| Fast | 400 kHz | High-speed I2C (default) |

### I2C Port Assignments

#### I2C_PORT_0: EPS Module

**Connected Device**: SpaceLab EPS 2.0 (SL_EPS2)
**Slave Address**: 0x36
**Speed**: 400 kHz (Fast mode)

**Purpose**: Power subsystem telemetry and control

**Data Exchanged**:
- Battery voltage/current
- Solar panel voltages/currents
- EPS temperature
- Module status
- Heater control commands

**Driver**: firmware/drivers/sl_eps2/sl_eps2.c:1

#### I2C_PORT_1: Antenna Module

**Connected Device**: ISIS Antenna System
**Slave Address**: Varies (typically 0x31-0x33)
**Speed**: 100 kHz (Standard mode for compatibility)

**Purpose**: Antenna deployment control and status

**Data Exchanged**:
- Deployment status
- Antenna temperature
- Burn wire control
- Deployment counter

**Driver**: firmware/drivers/isis_antenna/isis_antenna.c:1

#### I2C_PORT_2: Expansion

**Status**: Available for sensors and additional modules
**Potential Uses**:
- External sensors (magnetometer, gyroscope)
- Payload interfaces
- Inter-module communication

### I2C API

```c
// Initialize I2C port
int i2c_init(i2c_port_t port, i2c_config_t config);

// Write data to slave
int i2c_write(i2c_port_t port, uint8_t slave_addr,
              uint8_t *data, uint16_t len);

// Read data from slave
int i2c_read(i2c_port_t port, uint8_t slave_addr,
             uint8_t *data, uint16_t len);

// Write then read (register access pattern)
int i2c_write_read(i2c_port_t port, uint8_t slave_addr,
                   uint8_t *tx_data, uint16_t tx_len,
                   uint8_t *rx_data, uint16_t rx_len);

// Configuration structure
typedef struct {
    uint32_t speed;         // Clock speed (Hz): 100000 or 400000
} i2c_config_t;
```

### Example Usage: EPS Communication

**Location**: firmware/drivers/sl_eps2/sl_eps2.c:1

```c
// Initialize I2C for EPS
i2c_config_t i2c_cfg = {
    .speed = 400000         // 400 kHz Fast mode
};

i2c_init(I2C_PORT_0, i2c_cfg);

// Read EPS register
#define SL_EPS2_ADDR    0x36
#define REG_BATTERY_V   0x10

uint8_t reg_addr = REG_BATTERY_V;
uint16_t battery_voltage;

// Write register address, then read 2 bytes
i2c_write_read(I2C_PORT_0, SL_EPS2_ADDR,
               &reg_addr, 1,
               (uint8_t*)&battery_voltage, 2);
```

### Example Usage: Antenna Deployment

**Location**: firmware/drivers/isis_antenna/isis_antenna.c:1

```c
// Deploy antenna (send command)
#define ISIS_ANT_ADDR       0x31
#define CMD_DEPLOY_ALL      0xA5

uint8_t deploy_cmd[] = {CMD_DEPLOY_ALL, 0x00, 0x0A};  // 10s burn

i2c_write(I2C_PORT_1, ISIS_ANT_ADDR, deploy_cmd, sizeof(deploy_cmd));

// Read deployment status
uint8_t status[2];
i2c_read(I2C_PORT_1, ISIS_ANT_ADDR, status, sizeof(status));
```

### Features

- **Multi-Master Support**: Ready for multi-master (currently master-only)
- **7-bit Addressing**: Standard I2C addressing scheme
- **Clock Stretching**: Slave can hold SCL low
- **Error Detection**: ACK/NACK handling
- **Repeated Start**: Supported for register read operations

### I2C Device Addresses

| Device | Address | Port | Speed |
|--------|---------|------|-------|
| SL_EPS2 | 0x36 | I2C_PORT_0 | 400 kHz |
| ISIS Antenna | 0x31-0x33 | I2C_PORT_1 | 100 kHz |
| TCA4311A (Level Shifter) | N/A (transparent) | Various | 400 kHz |

## GPIO Interface

**Location**: firmware/drivers/gpio/gpio.c:1

### Pin Count

**Total GPIO Pins**: 70 (GPIO_PIN_0 through GPIO_PIN_69)

**MSP430F6659 Ports**:
- Port 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 (varies by port)
- Each port: 8 pins (P1.0-P1.7, P2.0-P2.7, etc.)

### Pin Modes

| Mode | Description |
|------|-------------|
| INPUT | High-impedance input |
| OUTPUT | Push-pull output |
| INPUT_PULLUP | Input with internal pull-up |
| INPUT_PULLDOWN | Input with internal pull-down |

### GPIO API

```c
// Initialize pin
int gpio_init(gpio_pin_t pin, gpio_mode_t mode);

// Set pin high
void gpio_set_high(gpio_pin_t pin);

// Set pin low
void gpio_set_low(gpio_pin_t pin);

// Toggle pin
void gpio_toggle(gpio_pin_t pin);

// Read pin state
uint8_t gpio_read(gpio_pin_t pin);

// Configure interrupt
int gpio_set_interrupt(gpio_pin_t pin, gpio_edge_t edge,
                       void (*callback)(void));

// Edge types for interrupts
typedef enum {
    GPIO_EDGE_RISING,
    GPIO_EDGE_FALLING,
    GPIO_EDGE_BOTH
} gpio_edge_t;
```

### Example Usage: Hardware Version Detection

**Location**: firmware/system/system.c:1

```c
// Read hardware version from GPIO pins
#define HW_VERSION_BIT0     GPIO_PIN_14
#define HW_VERSION_BIT1     GPIO_PIN_15

gpio_init(HW_VERSION_BIT0, GPIO_INPUT_PULLUP);
gpio_init(HW_VERSION_BIT1, GPIO_INPUT_PULLUP);

uint8_t bit0 = gpio_read(HW_VERSION_BIT0);
uint8_t bit1 = gpio_read(HW_VERSION_BIT1);

uint8_t hw_version = (bit1 << 1) | bit0;
// 0x0 = HW_VERSION_0
// 0x1 = HW_VERSION_1
// 0x2 = HW_VERSION_2
// 0x3 = HW_VERSION_3
```

### Example Usage: LED Control

**Location**: firmware/devices/leds/leds.c:1

```c
#define LED_HEARTBEAT       GPIO_PIN_20

// Initialize LED
gpio_init(LED_HEARTBEAT, GPIO_OUTPUT);
gpio_set_low(LED_HEARTBEAT);  // LED off

// Toggle LED (heartbeat)
while (1) {
    gpio_toggle(LED_HEARTBEAT);
    vTaskDelay(pdMS_TO_TICKS(1000));  // 1 Hz blink
}
```

### Special GPIO Functions

#### SPI Chip Select Pins

**SPI_PORT_0 CS Assignments**:
- GPIO_PIN_5: SPI_CS_0 (TTC Radio 0)
- GPIO_PIN_6: SPI_CS_1 (TTC Radio 1)
- GPIO_PIN_28: SPI_CS_2 (MT25Q Flash)
- GPIO_PIN_45: SPI_CS_3 (Expansion)

#### Hardware Detection Pins

- GPIO_PIN_14: Hardware version bit 0
- GPIO_PIN_15: Hardware version bit 1

### GPIO Interrupts

**Supported**: Rising edge, falling edge, both edges
**Priority**: Configurable interrupt priority
**Callback**: User-defined ISR function

```c
// Example: Button interrupt
void button_pressed(void) {
    sys_log(SYS_LOG_INFO, "Button pressed");
}

gpio_set_interrupt(GPIO_PIN_10, GPIO_EDGE_FALLING, button_pressed);
```

## ADC Interface

### Hardware

- **Peripheral**: MSP430 ADC12_A
- **Resolution**: 12-bit (0-4095)
- **Reference Voltage**: 2.5V or 3.3V (configurable)
- **Channels**: Multiple analog input channels

### ADC Channels

**Location**: firmware/devices/ (sensor device layers)

| Channel | Sensor | Measurement |
|---------|--------|-------------|
| ADC_CH_TEMP | Temperature Sensor | MCU die temperature |
| ADC_CH_VOLTAGE | Voltage Sensor | Input voltage |
| ADC_CH_CURRENT | Current Sensor | Input current |

### Usage

ADC access is abstracted through sensor device layer:

**Temperature Sensor**: firmware/devices/temp_sensor/temp_sensor.c:1
```c
int temp_sensor_get_data(uint16_t *temp_k);  // Returns Kelvin
```

**Voltage Sensor**: firmware/devices/voltage_sensor/voltage_sensor.c:1
```c
int voltage_sensor_get_data(uint16_t *voltage_mv);  // Returns mV
```

**Current Sensor**: firmware/devices/current_sensor/current_sensor.c:1
```c
int current_sensor_get_data(uint16_t *current_ma);  // Returns mA
```

## Timing and Clock Configuration

**Location**: firmware/system/clocks.c:1

### Clock Sources

| Clock | Frequency | Source | Usage |
|-------|-----------|--------|-------|
| MCLK | 32 MHz | DCO + FLL | CPU execution |
| SMCLK | 32 MHz | DCO + FLL | Peripherals (USCI) |
| ACLK | 32.768 kHz | External crystal | FreeRTOS tick, RTC |

### Peripheral Clock Assignments

- **UART**: SMCLK (32 MHz)
- **SPI**: SMCLK (32 MHz)
- **I2C**: SMCLK (32 MHz)
- **FreeRTOS Tick**: ACLK (32.768 kHz)
- **Watchdog**: ACLK (32.768 kHz)

### Clock Configuration Example

```c
// Initialize clocks (in system startup)
void clocks_setup(void) {
    // Configure DCO to 32 MHz
    // Configure FLL for frequency locking
    // Set MCLK = 32 MHz
    // Set SMCLK = 32 MHz
    // Set ACLK = 32.768 kHz (external crystal)
}
```

## Interface Performance

### Throughput

| Interface | Max Theoretical | Typical Achieved | Limiting Factor |
|-----------|----------------|------------------|-----------------|
| UART | 115200 bps | ~11 KB/s | Baud rate |
| SPI | 1 MHz | ~100 KB/s | Clock speed, CS overhead |
| I2C | 400 kHz | ~40 KB/s | Protocol overhead |

### Latency

| Operation | Typical Latency | Notes |
|-----------|----------------|-------|
| UART TX (1 byte) | ~87 µs | At 115200 bps |
| SPI Transfer (1 byte) | ~8 µs | At 1 MHz |
| I2C Transaction | ~25 µs | At 400 kHz, single byte |
| GPIO Write | <1 µs | Direct register access |
| ADC Conversion | ~30 µs | 12-bit conversion |

## Error Handling

### UART Errors

- **Overrun**: RX buffer full, data lost
- **Framing Error**: Invalid stop bit
- **Parity Error**: Parity mismatch

**Detection**: Status registers checked after each operation
**Recovery**: Flush buffer, reinitialize if needed

### SPI Errors

- **No Acknowledgment**: Slave not responding
- **Data Mismatch**: Checksum failure

**Detection**: Validate response data
**Recovery**: Retry transaction, check CS connection

### I2C Errors

- **NACK**: Slave not acknowledging
- **Bus Busy**: Another master using bus
- **Timeout**: Clock stretching exceeded limit

**Detection**: Status registers, timeout counters
**Recovery**: Bus reset, retry transaction

### GPIO Errors

- **Minimal**: Direct hardware access, few failure modes
- **Pin Conflict**: Multiple functions assigned to same pin

**Detection**: Compile-time checks, initialization validation
**Recovery**: Proper pin mapping configuration

## Level Shifting and Protection

### TCA4311A I2C Level Shifter

**Location**: firmware/drivers/tca4311a/

**Purpose**: Bridge different voltage domains on I2C bus
**Features**:
- Bidirectional voltage translation
- Rise-time accelerator
- Stuck-bus recovery

**Connection**: Transparent to software, hardware-level interface

## Interface Testing

### Loopback Tests

**UART**: TX connected to RX for data echo
**SPI**: MOSI connected to MISO for transfer verification
**I2C**: Software loopback through slave device

### Protocol Analyzers

- **UART**: Logic analyzer on TX/RX lines
- **SPI**: Monitor CLK, MISO, MOSI, CS signals
- **I2C**: Monitor SCL, SDA for transactions

### Validation

**Location**: firmware/tests/drivers/

Unit tests for each interface:
- Initialization
- Data transfer
- Error conditions
- Performance benchmarks

## Summary

The OBDH2 communication interfaces provide:

1. **UART**: Debug logging and expansion (115200 bps default)
2. **SPI**: High-speed module communication (TTC, Flash, 1 MHz)
3. **I2C**: Multi-device bus (EPS, Antenna, 400 kHz)
4. **GPIO**: Control signals and hardware detection (70 pins)
5. **ADC**: Sensor measurements (12-bit, multiple channels)

All interfaces are abstracted through driver layers, providing consistent APIs and error handling for reliable satellite operations.
