# OBDH 2.0 Communication Interfaces

## Overview

The OBDH 2.0 firmware implements multiple communication interfaces for internal subsystem communication and external data exchange. This document details the hardware interfaces, protocols, and communication patterns used throughout the system.

---

## Interface Summary

| Interface | Ports | Speed | Usage | Devices Connected |
|-----------|-------|-------|-------|-------------------|
| **UART** | 3 | 115200 bps | Serial communication | Debug, EDC, Reserved |
| **SPI** | 6 | 1 MHz (configurable) | High-speed serial | NOR Flash, FRAM, Antenna |
| **I2C** | 3 | 100/400 kHz | Multi-device bus | EPS, TTC, Sensors, Antenna |
| **GPIO** | 47 | - | Digital I/O | LEDs, Control signals, Interrupts |
| **ADC** | 12-bit | - | Analog input | Temperature, Voltage, Current |
| **RF** | - | UHF/VHF | Radio uplink/downlink | TTC modules (via I2C control) |

---

## UART (Universal Asynchronous Receiver/Transmitter)

### Hardware Configuration

**MSP430 Module:** eUSCI_A (3 instances: A0, A1, A2)

**Electrical Characteristics:**
- Voltage levels: 3.3V CMOS
- Full-duplex operation
- Hardware flow control: Optional (RTS/CTS)

### UART Ports

#### UART Port 0

**Primary Use:** Debug and system logging

**Configuration:**
- **Baud Rate:** 115200 bps
- **Data Bits:** 8
- **Stop Bits:** 1
- **Parity:** None
- **Flow Control:** None

**Pins:**
- TX: P3.4 (or configurable)
- RX: P3.5 (or configurable)

**Driver:** `drivers/uart/`

**Device Interface:** System Log (`system/sys_log/`)

**Usage Example:**
```c
#include "sys_log/sys_log.h"

// Initialize debug UART
sys_log_init();

// Print debug message
sys_log_print_event("OBDH startup complete");
sys_log_print_hex(data_buffer, 16);  // Print 16 bytes in hex
```

---

#### UART Port 1

**Primary Use:** EDC (Earth Data Collection) module communication

**Configuration:**
- **Baud Rate:** 115200 bps (configurable per payload)
- **Data Bits:** 8
- **Stop Bits:** 1
- **Parity:** None

**Driver:** `drivers/uart/`, `drivers/edc/`

**Device Interface:** `devices/payload/`

**Protocol:** Payload-specific binary protocol

**Usage Example:**
```c
#include "edc/edc.h"

// Initialize EDC UART
edc_init(EDC_PORT_1, 115200);

// Request data from EDC
edc_request_data(buffer, sizeof(buffer));

// Read response
int len = edc_read(buffer, MAX_SIZE);
```

---

#### UART Port 2

**Primary Use:** Reserved for future expansion

**Potential Uses:**
- Additional payload communication
- Inter-satellite links
- Backup debug port

---

### UART Driver API

**File:** `drivers/uart/uart.c/h`

```c
// Initialize UART port
int uart_init(uart_port_t port, uart_config_t config);

// Write data (blocking)
int uart_write(uart_port_t port, uint8_t *data, uint16_t len);

// Read data (blocking with timeout)
int uart_read(uart_port_t port, uint8_t *buffer, uint16_t max_len, uint32_t timeout_ms);

// Check bytes available
int uart_bytes_available(uart_port_t port);

// Flush buffers
void uart_flush(uart_port_t port);
```

**Configuration Structure:**
```c
typedef struct {
    uint32_t baud_rate;      // 9600, 19200, 38400, 57600, 115200, etc.
    uint8_t  data_bits;      // 7 or 8
    uint8_t  stop_bits;      // 1 or 2
    uint8_t  parity;         // NONE, EVEN, ODD
    uint8_t  flow_control;   // NONE, RTS_CTS
} uart_config_t;
```

---

## SPI (Serial Peripheral Interface)

### Hardware Configuration

**MSP430 Modules:**
- eUSCI_A: A0, A1, A2 (3 SPI ports)
- eUSCI_B: B0, B1, B2 (3 SPI ports)

**Total:** 6 SPI ports

**Electrical Characteristics:**
- Voltage levels: 3.3V CMOS
- Full-duplex operation
- Hardware chip select management

### SPI Configuration

**Mode:** Master (OBDH controls clock)

**Clock Speed:** Configurable per device
- NOR Flash: 1 MHz (safe speed, up to 133 MHz capable)
- FRAM: 40 MHz maximum, typically 1-10 MHz
- Antenna: Device-dependent

**SPI Mode:** Mode 0 (CPOL=0, CPHA=0) - typical
- Clock idle low
- Data sampled on rising edge
- Data changed on falling edge

**Bit Order:** MSB first

---

### SPI Devices

#### NOR Flash (MT25Q - 256 MB)

**Interface:** SPI Port 0 (eUSCI_A0 or configurable)

**Chip Select:** GPIO pin (configurable)

**Speed:** 1 MHz (conservative), up to 133 MHz capable

**Driver:** `drivers/mt25q/`

**Commands:**
- Read: 0x03 (standard), 0x0B (fast read)
- Page Program: 0x02 (256 bytes)
- Sector Erase: 0x20 (4 KB), 0xD8 (64 KB)
- Chip Erase: 0xC7
- Read Status: 0x05
- Write Enable: 0x06

**Usage Example:**
```c
#include "mt25q/mt25q.h"

// Initialize NOR flash
mt25q_init(SPI_PORT_0, CS_PIN_NOR);

// Erase sector (4 KB at address 0x1000)
mt25q_erase_sector(0x1000);

// Write page (256 bytes)
mt25q_write_page(0x1000, data, 256);

// Read data
mt25q_read(0x1000, buffer, 1024);
```

---

#### FRAM (CY15x102QN - 256 KB)

**Interface:** SPI Port (configurable)

**Chip Select:** GPIO pin (configurable)

**Speed:** Up to 40 MHz

**Driver:** `drivers/cy15x102qn/`

**Features:**
- No erase needed (byte-level write)
- Unlimited write endurance
- Fast write speed
- Low power

**Commands:**
- Read: 0x03
- Write: 0x02
- Read Status: 0x05
- Write Enable: 0x06

**Usage Example:**
```c
#include "cy15x102qn/cy15x102qn.h"

// Initialize FRAM
fram_init(SPI_PORT_1, CS_PIN_FRAM);

// Write data (no erase needed)
fram_write(0x0000, data, 100);

// Read data
fram_read(0x0000, buffer, 100);
```

---

#### ISIS Antenna Deployer

**Interface:** SPI or I2C (configurable)

**Driver:** `drivers/isis_antenna/`

**Commands:**
- Deploy Antenna 1-4: 0xA1-0xA4
- Auto Deploy: 0xAD
- Cancel Deploy: 0xA5
- Read Status: 0xC3
- Read Temperature: 0xC0

**Usage Example:**
```c
#include "isis_antenna/isis_antenna.h"

// Initialize antenna (SPI mode)
isis_antenna_init(INTERFACE_SPI, SPI_PORT_2, CS_PIN_ANTENNA);

// Deploy antenna 1 (10 second burn)
isis_antenna_deploy(ANTENNA_1, 10);

// Read deployment status
uint8_t status = isis_antenna_get_status(ANTENNA_1);
```

---

### SPI Driver API

**File:** `drivers/spi/spi.c/h`

```c
// Initialize SPI port
int spi_init(spi_port_t port, spi_config_t config);

// Transfer data (full-duplex)
int spi_transfer(spi_port_t port, uint8_t *tx_data, uint8_t *rx_data, uint16_t len);

// Write only (ignore received data)
int spi_write(spi_port_t port, uint8_t *data, uint16_t len);

// Read only (send dummy bytes)
int spi_read(spi_port_t port, uint8_t *buffer, uint16_t len);

// Chip select control
void spi_select(spi_port_t port, gpio_pin_t cs_pin);
void spi_deselect(spi_port_t port, gpio_pin_t cs_pin);
```

**Configuration Structure:**
```c
typedef struct {
    uint32_t clock_speed;    // Hz (e.g., 1000000 for 1 MHz)
    uint8_t  mode;           // 0-3 (CPOL/CPHA combinations)
    uint8_t  bit_order;      // MSB_FIRST or LSB_FIRST
} spi_config_t;
```

---

## I2C (Inter-Integrated Circuit)

### Hardware Configuration

**MSP430 Module:** eUSCI_B (3 instances: B0, B1, B2)

**Electrical Characteristics:**
- Voltage levels: 3.3V (with pull-up resistors)
- Open-drain outputs
- Multi-master capable (typically single master)

### I2C Ports

#### I2C Port 0 (Primary Bus)

**Connected Devices:**
- EPS (Electrical Power System) - Address: 0x30-0x3F (configurable)
- TTC 0 (Transceiver 0) - Address: 0x40-0x4F (configurable)
- TTC 1 (Transceiver 1) - Address: 0x50-0x5F (configurable)
- Temperature sensors - Addresses: 0x48-0x4F
- ISIS Antenna (if I2C mode) - Address: 0x31-0x33

**Configuration:**
- **Speed:** 100 kHz (standard mode) or 400 kHz (fast mode)
- **Pins:**
  - SDA: P3.1 (or configurable)
  - SCL: P3.2 (or configurable)
- **Pull-ups:** 4.7 kΩ to 3.3V (external)

**Driver:** `drivers/i2c/`

---

#### I2C Port 1 (Secondary Bus)

**Use:** Reserved for additional sensors or payload interfaces

---

#### I2C Port 2 (Tertiary Bus)

**Use:** Reserved for expansion

---

### I2C Device Communication

#### EPS (SpaceLab EPS v2)

**Driver:** `drivers/sl_eps2/`

**I2C Address:** 0x36 (example, configurable)

**Protocol:**
- Write command byte to request telemetry
- Read response (variable length, typically 50-100 bytes)

**Commands:**
- Request Telemetry: 0x01
- Enable Module: 0x10 + module_id
- Disable Module: 0x20 + module_id
- Reset: 0xFF

**Usage Example:**
```c
#include "sl_eps2/sl_eps2.h"

// Initialize EPS interface
sl_eps2_init(I2C_PORT_0, EPS_I2C_ADDR);

// Request telemetry
eps_data_t eps_data;
sl_eps2_request_telemetry(&eps_data);

// Enable heater (module 1)
sl_eps2_enable_module(MODULE_HEATER);
```

**Telemetry Structure:**
```c
typedef struct {
    uint16_t battery_voltage;       // mV
    int16_t  battery_current;       // mA
    uint16_t battery_charge;        // mAh
    int16_t  battery_temperature;   // °C * 10
    uint16_t solar_panel_voltage[6]; // mV
    uint16_t solar_panel_current[6]; // mA
    // ... (50+ fields total)
} eps_data_t;
```

---

#### TTC (SpaceLab TTC v2)

**Driver:** `drivers/sl_ttc2/`

**I2C Addresses:**
- TTC 0: 0x40
- TTC 1: 0x50

**Protocol:**
- Write packet to transmit
- Read to receive packets
- Read telemetry on request

**Commands:**
- Send Packet: 0x01 + length + data
- Receive Packet: 0x02 → read response
- Request Telemetry: 0x03
- Set Power Level: 0x10 + power_dbm
- Set Frequency: 0x11 + frequency_bytes

**Usage Example:**
```c
#include "sl_ttc2/sl_ttc2.h"

// Initialize TTC 0
sl_ttc2_init(TTC_PORT_0, I2C_PORT_0, TTC0_I2C_ADDR);

// Transmit beacon packet
sl_ttc2_send_packet(TTC_PORT_0, beacon_data, beacon_len);

// Check for received packets
if (sl_ttc2_packet_available(TTC_PORT_0)) {
    uint16_t len = sl_ttc2_receive_packet(TTC_PORT_0, rx_buffer, MAX_SIZE);
    // Process packet
}

// Read telemetry
ttc_data_t ttc_data;
sl_ttc2_request_telemetry(TTC_PORT_0, &ttc_data);
```

---

#### Temperature Sensors

**Typical IC:** TMP100, TMP102, or similar I2C temperature sensors

**I2C Address:** 0x48-0x4F (configurable via address pins)

**Driver:** Generic I2C temperature sensor driver

**Resolution:** 12-bit (0.0625°C per LSB)

**Usage Example:**
```c
// Read temperature from sensor at address 0x48
uint8_t reg = 0x00;  // Temperature register
uint8_t data[2];

i2c_write_read(I2C_PORT_0, 0x48, &reg, 1, data, 2);

int16_t raw_temp = (data[0] << 4) | (data[1] >> 4);
float temperature = raw_temp * 0.0625;  // Celsius
```

---

### I2C Driver API

**File:** `drivers/i2c/i2c.c/h`

```c
// Initialize I2C port
int i2c_init(i2c_port_t port, i2c_config_t config);

// Write data to device
int i2c_write(i2c_port_t port, uint8_t addr, uint8_t *data, uint16_t len);

// Read data from device
int i2c_read(i2c_port_t port, uint8_t addr, uint8_t *buffer, uint16_t len);

// Write then read (common pattern)
int i2c_write_read(i2c_port_t port, uint8_t addr,
                   uint8_t *write_data, uint16_t write_len,
                   uint8_t *read_buffer, uint16_t read_len);

// Check device presence (returns 0 if ACK received)
int i2c_probe(i2c_port_t port, uint8_t addr);
```

**Configuration Structure:**
```c
typedef struct {
    uint32_t clock_speed;    // 100000 (100 kHz) or 400000 (400 kHz)
    uint8_t  timeout_ms;     // Communication timeout
} i2c_config_t;
```

---

### I2C Bus Arbitration and Error Handling

**Multi-Device Access:**
- Only one device can communicate at a time
- OBDH (master) initiates all transactions
- Slaves respond when addressed

**Error Conditions:**
- **NACK (No Acknowledge):** Slave not responding
  - Retry mechanism (up to 3 attempts)
  - Log error and continue
- **Bus Busy:** Wait with timeout
- **Arbitration Lost:** Retry transaction (multi-master only)

**Bus Recovery:**
```c
// Reset I2C bus (clock stretching recovery)
void i2c_reset_bus(i2c_port_t port) {
    // Generate 9 clock pulses
    // Re-initialize I2C module
}
```

---

## GPIO (General Purpose Input/Output)

### GPIO Abstraction

**Total Pins:** 47 mapped GPIO pins across ports P1-P6

**Driver:** `drivers/gpio/gpio.c/h`

**Pin Naming:** `GPIO_PIN_0` to `GPIO_PIN_46`

**Port Mapping:**
```
GPIO_PIN_0-7   → Port P1 (GPIO_PORT_P1)
GPIO_PIN_8-15  → Port P2 (GPIO_PORT_P2)
GPIO_PIN_16-23 → Port P3 (GPIO_PORT_P3)
GPIO_PIN_24-31 → Port P4 (GPIO_PORT_P4)
GPIO_PIN_32-39 → Port P5 (GPIO_PORT_P5)
GPIO_PIN_40-46 → Port P6 (GPIO_PORT_P6)
```

---

### GPIO Functions

#### Output Pins

**Usage:**
- LED indicators
- Chip select for SPI devices
- Module enable/disable signals
- Antenna burn triggers

**Example:**
```c
#include "gpio/gpio.h"

// Configure pin as output
gpio_set_mode(GPIO_PIN_10, GPIO_MODE_OUTPUT);

// Set high
gpio_set(GPIO_PIN_10);

// Set low
gpio_clear(GPIO_PIN_10);

// Toggle
gpio_toggle(GPIO_PIN_10);
```

---

#### Input Pins

**Usage:**
- Interrupt signals from peripherals
- Status indicators
- Fault detection

**Features:**
- Pull-up/pull-down resistors
- Interrupt on edge (rising/falling/both)

**Example:**
```c
// Configure pin as input with pull-up
gpio_set_mode(GPIO_PIN_15, GPIO_MODE_INPUT);
gpio_set_pull(GPIO_PIN_15, GPIO_PULL_UP);

// Read state
uint8_t state = gpio_read(GPIO_PIN_15);

// Configure interrupt (falling edge)
gpio_enable_interrupt(GPIO_PIN_15, GPIO_INT_FALLING_EDGE, interrupt_handler);

// Interrupt handler
void interrupt_handler(void) {
    // Handle interrupt
    gpio_clear_interrupt(GPIO_PIN_15);
}
```

---

### LED Indicators

**Driver:** `devices/leds/`

**Typical Configuration:**
- **LED 0:** System heartbeat (blinks at 1 Hz)
- **LED 1:** Communication activity
- **LED 2:** Error indicator

**Usage:**
```c
#include "leds/leds.h"

// Initialize LEDs
leds_init();

// Turn on LED
led_on(LED_0);

// Turn off LED
led_off(LED_0);

// Toggle LED
led_toggle(LED_0);

// Blink pattern
led_blink(LED_0, 500);  // 500 ms period
```

---

## ADC (Analog-to-Digital Converter)

### Hardware Configuration

**MSP430 Module:** ADC12_A

**Resolution:** 12-bit (0-4095)

**Reference Voltage:** 2.5V internal reference

**Channels:** Up to 16 analog input channels

---

### ADC Usage

#### Temperature Measurement

**Internal Temperature Sensor:**
- Channel: ADC12INCH_10 (internal)
- Formula: `T(°C) = (V_sensor - 0.986) / 0.00355`

**Example:**
```c
#include "adc/adc.h"

// Initialize ADC
adc_init();

// Read internal temperature
uint16_t raw = adc_read_channel(ADC_CHANNEL_TEMP);
float voltage = (raw / 4096.0) * 2.5;
float temp_c = (voltage - 0.986) / 0.00355;

// Convert to Kelvin * 10 for storage
uint16_t temp_k10 = (temp_c + 273.15) * 10;
```

---

#### Voltage Measurement

**Input Voltage Divider:**
- R1 = 10 kΩ, R2 = 10 kΩ (example, 2:1 divider)
- Max input: 6.6V (for 3.3V ADC input)

**Example:**
```c
// Read voltage on ADC channel 0
uint16_t raw = adc_read_channel(ADC_CHANNEL_0);
float adc_voltage = (raw / 4096.0) * 2.5;
float input_voltage = adc_voltage * 2.0;  // Divider ratio

// Store in mV
uint16_t voltage_mv = input_voltage * 1000;
```

---

#### Current Measurement

**Current Sense Amplifier:**
- Shunt resistor (e.g., 0.1 Ω)
- Op-amp gain (e.g., 50x)
- Output to ADC

**Example:**
```c
// Read current on ADC channel 1
uint16_t raw = adc_read_channel(ADC_CHANNEL_1);
float adc_voltage = (raw / 4096.0) * 2.5;
float current = (adc_voltage / 50.0) / 0.1;  // Amps

// Store in mA
uint16_t current_ma = current * 1000;
```

---

### ADC Driver API

**File:** `drivers/adc/adc.c/h`

```c
// Initialize ADC module
int adc_init(void);

// Read single channel (blocking)
uint16_t adc_read_channel(adc_channel_t channel);

// Start conversion (non-blocking)
void adc_start_conversion(adc_channel_t channel);

// Check if conversion complete
bool adc_conversion_ready(void);

// Get result
uint16_t adc_get_result(void);
```

---

## RF Communication (Via TTC Modules)

### Overview

The OBDH does not directly control the radio hardware. Instead, it communicates with TTC modules via I2C, which handle the RF transmission and reception.

---

### Uplink (Ground to Satellite)

**Frequency:** UHF/VHF (configurable, e.g., 437 MHz)

**Modulation:** FSK, GFSK, or configurable

**Data Rate:** 1200 bps, 9600 bps, or higher (configurable)

**Flow:**
1. Ground station transmits RF signal
2. TTC module demodulates and stores packet
3. OBDH polls TTC via I2C (Read TTC Task)
4. OBDH retrieves packet
5. Process TC Task validates and executes command

---

### Downlink (Satellite to Ground)

**Frequency:** UHF/VHF (configurable, e.g., 437 MHz)

**Modulation:** FSK, GFSK, or configurable

**Data Rate:** 1200 bps, 9600 bps, or higher (configurable)

**Flow:**
1. Beacon Task formats packet
2. OBDH sends packet to TTC via I2C
3. TTC module modulates and transmits RF signal
4. Ground station receives and decodes

---

### Radio Configuration

**File:** `config/radio_config_Si4463.h` (Si4463 transceiver configuration)

**Parameters:**
- Frequency: Center frequency in Hz
- Deviation: FSK frequency deviation
- Data rate: Bits per second
- TX power: Transmission power in dBm
- Modulation: FSK, GFSK, OOK, etc.

---

## External Interfaces

### PC-104 Connector

**Purpose:** Power and data interface to satellite backplane

**Signals:**
- Power input (3.3V, 5V, 12V)
- I2C bus to other satellite modules
- GPIO signals for inter-module communication
- UART or SPI links (optional)

---

### JTAG Interface

**Purpose:** Programming and debugging

**Connector:** Standard MSP430 JTAG header (14-pin or 20-pin)

**Signals:**
- TDI, TDO, TMS, TCK (JTAG signals)
- VCC, GND (power for programmer)
- RST (reset)

**Programmer:** Texas Instruments MSP-FET or compatible

---

## Interface Summary Table

| Interface | Type | Speed | Driver Location | Device Interface | Primary Use |
|-----------|------|-------|-----------------|------------------|-------------|
| UART 0 | Serial | 115200 bps | `drivers/uart/` | `system/sys_log/` | Debug logging |
| UART 1 | Serial | 115200 bps | `drivers/uart/` | `drivers/edc/` | EDC payload |
| UART 2 | Serial | Configurable | `drivers/uart/` | Reserved | Future use |
| SPI 0 | Serial | 1 MHz | `drivers/spi/` | `drivers/mt25q/` | NOR Flash |
| SPI 1 | Serial | 1-10 MHz | `drivers/spi/` | `drivers/cy15x102qn/` | FRAM |
| SPI 2 | Serial | Configurable | `drivers/spi/` | `drivers/isis_antenna/` | Antenna (SPI mode) |
| SPI 3-5 | Serial | Configurable | `drivers/spi/` | Reserved | Future use |
| I2C 0 | Bus | 100/400 kHz | `drivers/i2c/` | `drivers/sl_eps2/`, `drivers/sl_ttc2/` | EPS, TTC, Sensors |
| I2C 1 | Bus | 100/400 kHz | `drivers/i2c/` | Reserved | Future use |
| I2C 2 | Bus | 100/400 kHz | `drivers/i2c/` | Reserved | Future use |
| GPIO | Digital I/O | - | `drivers/gpio/` | `devices/leds/` | LEDs, control signals |
| ADC | Analog | 12-bit | `drivers/adc/` | `devices/temp_sensor/`, `devices/current_sensor/` | Sensors |
| RF | Radio | 1200-9600 bps | - | `drivers/sl_ttc2/` | Uplink/Downlink |

---

## Related Documentation

- `00_functionalities_of_system.md` - System overview
- `01_firmware_architecture.md` - Firmware architecture
- `02_datapath.md` - Data structures and flow
- `04_tasks.md` - Task descriptions
- `05_pin_mapping.md` - Pin assignments
- `06_hardware_components_and_architecture.md` - Hardware details
