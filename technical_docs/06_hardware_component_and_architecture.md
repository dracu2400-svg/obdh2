# OBDH2 Hardware Components and Architecture

## Overview

This document describes the hardware components, peripherals, and physical architecture of the OBDH2 (On-Board Data Handling 2.0) satellite subsystem. It details the microcontroller, external components, communication interfaces, and integration with other satellite modules.

**Hardware Version**: Multiple revisions (v0, v1, v2, v3)
**Form Factor**: CubeSat compatible
**Mission**: FloripaSat-2 satellite on-board computer

## System Block Diagram

```
┌───────────────────────────────────────────────────────────────────┐
│                        OBDH2 SYSTEM                               │
├───────────────────────────────────────────────────────────────────┤
│                                                                   │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │         MSP430F6659 Microcontroller                     │    │
│  │  ┌──────────────────────────────────────────────────┐   │    │
│  │  │ CPU Core (16-bit RISC, 32 MHz)                   │   │    │
│  │  └──────────────────────────────────────────────────┘   │    │
│  │  ┌──────────────────────────────────────────────────┐   │    │
│  │  │ Memory                                           │   │    │
│  │  │ • 256 KB Flash                                   │   │    │
│  │  │ • 16 KB RAM                                      │   │    │
│  │  └──────────────────────────────────────────────────┘   │    │
│  │  ┌──────────────────────────────────────────────────┐   │    │
│  │  │ Peripherals                                      │   │    │
│  │  │ • UART × 3, SPI × 6, I2C × 3                     │   │    │
│  │  │ • ADC12_A (12-bit)                               │   │    │
│  │  │ • Timers, WDT, RTC                               │   │    │
│  │  └──────────────────────────────────────────────────┘   │    │
│  └─────────────────────────────────────────────────────────┘    │
│           │           │           │            │                 │
│           ▼           ▼           ▼            ▼                 │
│  ┌─────────────┐ ┌─────────┐ ┌─────────┐ ┌─────────────┐       │
│  │   SPI Bus   │ │ I2C Bus │ │ADC Inputs│ │   GPIO     │       │
│  └─────────────┘ └─────────┘ └─────────┘ └─────────────┘       │
│        │              │           │              │               │
│  ┌─────┴──────┐  ┌───┴────┐ ┌────┴────┐   ┌────┴──────┐       │
│  │ TTC Radios │  │  EPS   │ │ Sensors │   │    LEDs   │       │
│  │  (0 & 1)   │  │ Module │ │ (T,V,I) │   │  Watchdog │       │
│  │            │  │        │ │         │   └───────────┘       │
│  │ MT25Q      │  │Antenna │ │         │                       │
│  │ NOR Flash  │  │ Module │ │         │                       │
│  └────────────┘  └────────┘ └─────────┘                       │
│                                                                   │
│  External Components:                                            │
│  • TPS382x Watchdog Timer                                        │
│  • TCA4311A I2C Level Shifter                                    │
│  • CY15x102QN FRAM (64 KB)                                       │
│  • 32.768 kHz Crystal Oscillator                                 │
│                                                                   │
└───────────────────────────────────────────────────────────────────┘
         │                  │                 │
         ▼                  ▼                 ▼
  ┌─────────────┐   ┌─────────────┐   ┌─────────────┐
  │   Ground    │   │  Satellite  │   │  Payloads   │
  │   Station   │   │  Subsystems │   │ (EDC, etc)  │
  │  (UHF/VHF)  │   │ (Power, etc)│   │             │
  └─────────────┘   └─────────────┘   └─────────────┘
```

## Core Microcontroller

### MSP430F6659

**Manufacturer**: Texas Instruments
**Family**: MSP430F6xx (16-bit ultra-low-power MCU)
**Datasheet**: SLAS645

#### Specifications

| Parameter | Specification |
|-----------|---------------|
| **Architecture** | 16-bit RISC, Von Neumann |
| **Clock Speed** | Up to 25 MHz (firmware uses 32 MHz overdrive) |
| **Flash Memory** | 256 KB |
| **RAM** | 16 KB |
| **Operating Voltage** | 1.8V - 3.6V (typical 3.3V) |
| **Operating Temperature** | -40°C to +85°C (industrial) |
| **Package** | LQFP-100 (100-pin) |
| **Power Consumption** | Ultra-low-power (µA range in sleep modes) |

#### Memory Map

**Linker Script**: firmware/lnk_msp430f6659.cmd:1

```
┌─────────────────────────────────────────┐
│ Memory Map                              │
├─────────────────────────────────────────┤
│ 0x0000 - 0x00FF: Special Function Regs  │
│ 0x0100 - 0x01FF: 8-bit Peripherals      │
│ 0x0200 - 0x09FF: 16-bit Peripherals     │
├─────────────────────────────────────────┤
│ RAM (16 KB)                             │
│ 0x1C00 - 0x5BFF                         │
│ • .data (initialized variables)         │
│ • .bss (uninitialized variables)        │
│ • Stack                                 │
│ • Heap (FreeRTOS: 40 KB allocated)      │
├─────────────────────────────────────────┤
│ Flash (256 KB)                          │
│ 0x4400 - 0x43FFF                        │
│ • .text (code)                          │
│ • .const (read-only data)               │
│ • Interrupt vectors (at top of Flash)  │
├─────────────────────────────────────────┤
│ 0xFF80 - 0xFFFF: Interrupt Vectors      │
└─────────────────────────────────────────┘
```

#### Peripheral Integration

**UART (USCI_A)**:
- USCI_A0: UART_PORT_0 (debug logging, 115200 bps)
- USCI_A1: UART_PORT_1 (expansion)
- USCI_A2: UART_PORT_2 (expansion)

**SPI (USCI_A/B)**:
- USCI_A/B modules configured for SPI mode
- 6 SPI ports available (SPI_PORT_0 through SPI_PORT_5)
- Clock speeds up to SMCLK/2 (16 MHz max)

**I2C (USCI_B)**:
- USCI_B modules configured for I2C mode
- 3 I2C ports (I2C_PORT_0, I2C_PORT_1, I2C_PORT_2)
- Multi-master capable (currently master-only)

**ADC12_A**:
- 12-bit successive approximation ADC
- Multiple input channels
- Configurable reference voltage (1.5V, 2.0V, 2.5V, 3.3V)
- Used for temperature, voltage, current sensors

**Timers**:
- Timer_A: General-purpose timer, PWM generation
- Timer_B: Capture/compare
- Timer_D: High-resolution PWM
- Usage: FreeRTOS tick generation (Timer A0 with ACLK)

**Watchdog Timer (WDT_A)**:
- Internal watchdog (typically disabled, using external watchdog)
- Can be configured as interval timer

**Real-Time Clock (RTC)**:
- Calendar mode or counter mode
- Low-power operation from ACLK (32.768 kHz)

#### Clock System

**Location**: firmware/system/clocks.c:1

```
┌────────────────────────────────────────┐
│ Clock Configuration                    │
├────────────────────────────────────────┤
│                                        │
│  External Crystal (32.768 kHz)         │
│         │                              │
│         ▼                              │
│  ┌──────────────┐                      │
│  │     ACLK     │ 32.768 kHz           │
│  │ (Auxiliary)  │ → FreeRTOS tick      │
│  └──────────────┘   → RTC              │
│                      → Watchdog        │
│                                        │
│  DCO (Digitally Controlled Oscillator) │
│         │                              │
│         ▼                              │
│  ┌──────────────┐                      │
│  │     FLL      │ (Frequency Locked)   │
│  │ (Multiplier) │                      │
│  └──────────────┘                      │
│         │                              │
│         ▼                              │
│  ┌──────────────┐                      │
│  │     MCLK     │ 32 MHz               │
│  │   (Main)     │ → CPU execution      │
│  └──────────────┘                      │
│         │                              │
│         ▼                              │
│  ┌──────────────┐                      │
│  │    SMCLK     │ 32 MHz               │
│  │ (Subsystem)  │ → UART, SPI, I2C     │
│  └──────────────┘   → ADC              │
│                                        │
└────────────────────────────────────────┘
```

**Configuration**:
- **MCLK**: 32 MHz (DCO with FLL, overdrive mode)
- **SMCLK**: 32 MHz (same as MCLK)
- **ACLK**: 32.768 kHz (external crystal)

**FreeRTOS Tick**: Generated by Timer A0 clocked from ACLK (1 ms period)

## External Memory Components

### MT25Q NOR Flash

**Location**: firmware/drivers/mt25q/mt25q.c:1

**Manufacturer**: Micron Technology
**Type**: Serial NOR Flash Memory
**Interface**: SPI (Quad-SPI capable)

#### Specifications

| Parameter | Specification |
|-----------|---------------|
| **Capacity** | 16/64/128/256 MB variants |
| **Interface** | SPI (Standard, Dual, Quad) |
| **Clock Speed** | Up to 133 MHz (firmware uses 1 MHz) |
| **Operating Voltage** | 2.7V - 3.6V |
| **Page Size** | 256 bytes |
| **Sector Size** | 4 KB, 64 KB (sub-sector, sector) |
| **Endurance** | 100,000 erase cycles |

#### Memory Organization

- **Page Program**: Write up to 256 bytes at a time
- **Sector Erase**: Erase 4 KB sectors (minimum erasable unit)
- **Block Erase**: Erase 64 KB blocks
- **Chip Erase**: Erase entire device

#### Usage

**Primary Function**: Data logging storage
- Housekeeping data archives (10-minute intervals)
- Long-term telemetry storage
- Retrievable by ground station command

**Device Layer**: firmware/devices/media/media.c:1
**Media Type**: `MEDIA_NOR`

### CY15x102QN FRAM

**Location**: firmware/drivers/cy15x102qn/

**Manufacturer**: Cypress Semiconductor (Infineon)
**Type**: Ferroelectric RAM (F-RAM)
**Interface**: I2C or SPI

#### Specifications

| Parameter | Specification |
|-----------|---------------|
| **Capacity** | 64 KB (512 Kbit) |
| **Interface** | I2C (400 kHz) or SPI |
| **Operating Voltage** | 2.0V - 3.6V |
| **Write Endurance** | 10^14 cycles (virtually unlimited) |
| **Write Time** | Instant (no erase needed) |
| **Data Retention** | 10 years (minimum) |
| **Operating Temperature** | -40°C to +85°C |

#### Advantages

- **Non-Volatile RAM**: Retains data without power
- **Instant Write**: No erase cycle, write at bus speed
- **High Endurance**: Unlimited write cycles (practical use)
- **Low Power**: Ultra-low write current

#### Usage

**Primary Function**: Configuration and state storage
- System configuration parameters
- Reset cause and counter
- Last valid telecommand
- Persistent state across power cycles

**Device Layer**: firmware/devices/media/media.c:1
**Media Type**: `MEDIA_FRAM`

**Hardware Availability**: HW_VERSION_1 and later

## Communication Modules

### SL_TTC2 (SpaceLab TTC 2.0)

**Location**: firmware/drivers/sl_ttc2/sl_ttc2.c:1

**Manufacturer**: SpaceLab UFSC
**Type**: Telemetry, Tracking & Command Module
**Quantity**: 2 modules (TTC_0, TTC_1) for redundancy

#### Specifications

| Parameter | Specification |
|-----------|---------------|
| **Interface** | SPI |
| **Radio IC** | Si4463 (underlying transceiver) |
| **Frequency Band** | UHF (downlink), VHF (uplink) |
| **Data Rate** | Configurable (up to 100 kbps) |
| **Transmit Power** | Up to 1W (30 dBm) |
| **Modulation** | FSK, GFSK, OOK |
| **Operating Voltage** | 3.3V - 5V |

**Radio Configuration**: firmware/config/radio_config_Si4463.h:1

#### Features

- **Dual Radio Redundancy**: TTC_0 (primary), TTC_1 (backup)
- **Housekeeping Telemetry**: Temperature, RSSI, packet counters
- **Packet TX/RX**: Up to 256-byte packets
- **Hibernation Mode**: Low-power sleep mode
- **Device ID**: 0xCC2A (Radio 0), 0xCC2B (Radio 1)

#### Functions

**Location**: firmware/devices/ttc/ttc.c:1

```c
int ttc_init(ttc_id_t id);                          // Initialize
int ttc_send(ttc_id_t id, uint8_t *data, uint16_t len);  // Transmit
int ttc_recv(ttc_id_t id, uint8_t *data, uint16_t *len); // Receive
int ttc_get_data(ttc_id_t id, ttc_telemetry_t *data);    // Housekeeping
int ttc_enter_hibernation(ttc_id_t id);             // Low power
int ttc_leave_hibernation(ttc_id_t id);             // Wake up
```

#### Usage

- **Uplink**: Receive telecommands from ground station (VHF)
- **Downlink**: Transmit telemetry, beacon, responses (UHF)
- **Protocol**: NGHam encoding, libcsp network layer

### SL_EPS2 (SpaceLab EPS 2.0)

**Location**: firmware/drivers/sl_eps2/sl_eps2.c:1

**Manufacturer**: SpaceLab UFSC
**Type**: Electrical Power System Module
**Interface**: I2C (Slave address 0x36)

#### Features

- **Battery Management**: Charge control, voltage/current monitoring
- **Solar Panel Interface**: 3-axis solar panel monitoring
- **Power Distribution**: Module and payload power control
- **Heater Control**: Thermal management
- **Telemetry**: Voltages, currents, temperature

#### Telemetry Data

```c
typedef struct {
    uint16_t battery_voltage;       // mV
    uint16_t battery_current;       // mA (charge/discharge)
    uint16_t solar_panel_v[3];      // mV per axis (X, Y, Z)
    uint16_t solar_panel_i[3];      // mA per axis
    int16_t temperature;            // °C
    uint8_t module_status;          // Flags
    uint8_t heater_status;          // On/off
} eps_telemetry_t;
```

#### Functions

**Location**: firmware/devices/eps/eps.c:1

```c
int eps_init(void);                          // Initialize
int eps_get_data(eps_telemetry_t *data);     // Read telemetry
int eps_set_heater(uint8_t state);           // Control heater
int eps_set_module_power(uint8_t module, uint8_t state); // Control power
```

#### Usage

- **Power Monitoring**: Battery and solar panel telemetry
- **Module Control**: Enable/disable satellite subsystems
- **Thermal Management**: Heater control based on temperature

### ISIS Antenna Deployment System

**Location**: firmware/drivers/isis_antenna/isis_antenna.c:1

**Manufacturer**: ISIS (Innovative Solutions In Space)
**Type**: Deployable Antenna System
**Interface**: I2C

#### Features

- **4-Channel Deployment**: Independent burn wire circuits
- **Deployment Monitoring**: Status feedback for each antenna
- **Temperature Sensor**: Monitor antenna temperature
- **Redundant Deployment**: Multiple attempts, configurable delays
- **Safety Timers**: Prevent excessive burn current

#### Configuration

**Firmware Settings** (firmware/config/config.h:1):
- **Independent Deployment**: 10 seconds burn time
- **Sequential Deployment**: 20 seconds total
- **Max Attempts**: 10
- **Hibernation After Failure**: 45 minutes

#### Functions

**Location**: firmware/devices/antenna/antenna.c:1

```c
int antenna_init(void);                          // Initialize
int antenna_deploy(antenna_deploy_mode_t mode);  // Deploy antennas
int antenna_get_status(antenna_status_t *status); // Read status
int antenna_get_data(antenna_telemetry_t *data); // Telemetry
```

#### Deployment Sequence

**Location**: firmware/app/tasks/antenna_deployment.c:1

1. Send deployment command via I2C
2. Monitor deployment status (polling)
3. Check for successful deployment confirmation
4. If failed, retry after hibernation period
5. Log deployment result

## Sensors

### Temperature Sensor

**Location**: firmware/devices/temp_sensor/temp_sensor.c:1

**Type**: MCU die temperature sensor (internal to MSP430)
**Interface**: ADC channel

**Range**: -40°C to +85°C (typical)
**Resolution**: 12-bit ADC (0.1°C precision)
**Output Format**: Kelvin (uint16_t)

**Calibration**: Factory calibration data stored in MSP430 info memory

**Usage**: Monitor OBDH temperature, thermal management

### Voltage Sensor

**Location**: firmware/devices/voltage_sensor/voltage_sensor.c:1

**Type**: Analog voltage divider → ADC
**Measurement**: Input power voltage

**Range**: 0 - 5V (typical satellite bus voltage)
**Resolution**: 12-bit ADC (~1.2 mV per bit at 3.3V ref)
**Output Format**: Millivolts (uint16_t)

**Usage**: Monitor input power quality, detect undervoltage conditions

### Current Sensor

**Location**: firmware/devices/current_sensor/current_sensor.c:1

**Type**: Current shunt amplifier → ADC
**Measurement**: Input power current

**Range**: 0 - 1A (typical)
**Resolution**: 12-bit ADC (~0.25 mA per bit)
**Output Format**: Milliamperes (uint16_t)

**Usage**: Monitor power consumption, detect overcurrent

## Protection and Management ICs

### TPS382x Watchdog Timer

**Location**: firmware/drivers/tps382x/tps382x.c:1

**Manufacturer**: Texas Instruments
**Type**: External Watchdog Timer IC
**Interface**: GPIO (watchdog input pin)

#### Specifications

| Parameter | Specification |
|-----------|---------------|
| **Timeout Period** | ~500-600 ms (configurable) |
| **Reset Pulse Width** | 200 ms (typical) |
| **Operating Voltage** | 1.0V - 5.5V |
| **Supply Current** | <10 µA (typical) |

#### Operation

**Watchdog Refresh**: firmware/app/tasks/watchdog_reset.c:1

- Task refreshes watchdog every 500 ms (high priority)
- If firmware hangs, watchdog times out
- Watchdog pulls reset line low → system reset

**Control Method**: GPIO toggle on WDI (Watchdog Input) pin

**Purpose**: Ultimate fault recovery mechanism

### TCA4311A I2C Level Shifter/Buffer

**Location**: firmware/drivers/tca4311a/

**Manufacturer**: Texas Instruments
**Type**: I2C Bus Buffer with Level Shifting
**Interface**: Transparent (hardware-level)

#### Features

- **Bidirectional Voltage Translation**: 3.3V ↔ 5V
- **Rise-Time Accelerator**: Improves signal integrity
- **Stuck-Bus Recovery**: Automatic recovery from bus lockup
- **Low-Power**: <3 µA quiescent current

#### Usage

Bridges I2C voltage domains between:
- MSP430 (3.3V logic)
- External modules (may use 5V logic)

**Software**: Transparent, no driver interaction needed

## Payload Interfaces

### Generic Payload Device

**Location**: firmware/devices/payload/payload.c:1

**Supported Payloads**:
- **EDC-1** (Educational CubeSat payload)
- **EDC-2** (Second EDC unit)
- **Payload-X** (External experiment)
- **HARSH** (Radiation monitoring)

#### Interface

**Protocol**: SPI or I2C (configurable per payload)
**Data Size**: Up to 220 bytes per payload

#### Functions

```c
int payload_init(payload_id_t id);                   // Initialize
int payload_get_data(payload_id_t id, uint8_t *data, uint16_t *len);
int payload_send_cmd(payload_id_t id, uint8_t cmd);
int payload_enable(payload_id_t id);
int payload_disable(payload_id_t id);
```

#### Data Structure

```c
typedef struct {
    uint8_t data[220];      // Payload-specific data
    uint16_t length;        // Valid data length
    uint32_t timestamp;     // Acquisition timestamp
} payload_telemetry_t;
```

**Storage**: firmware/app/structs/satellite.h:1 (`sat_data_buf`)

## Hardware Revisions

**Location**: firmware/system/system.c:1

### Version Detection

Hardware version detected via 2 GPIO pins:
- **GPIO_PIN_14**: Hardware version bit 0
- **GPIO_PIN_15**: Hardware version bit 1

### Supported Versions

| Version Code | Name | Key Features |
|--------------|------|--------------|
| 0x0 | HW_VERSION_0 | Initial prototype |
| 0x1 | HW_VERSION_1 | Added FRAM support |
| 0x2 | HW_VERSION_2 | Revised power circuitry |
| 0x3 | HW_VERSION_3 | Production version |

**Detection Code**: firmware/system/system.c:1

```c
uint8_t hw_version = system_get_hw_version();
// Returns 0, 1, 2, or 3
```

### Version-Specific Features

**HW_VERSION_0**:
- Basic functionality
- No FRAM support

**HW_VERSION_1+**:
- CY15x102QN FRAM enabled
- Enhanced power management
- Improved signal integrity

## Power Architecture

### Input Power

- **Source**: EPS module (battery + solar panels)
- **Voltage Range**: 3.3V - 5V (typical 3.3V)
- **Current Draw**: ~100-300 mA (operational), <10 mA (hibernation)

### Internal Regulation

- **MCU Voltage**: 3.3V (regulated)
- **Peripheral Voltage**: 3.3V or 5V (depending on module)
- **Decoupling**: Capacitors on power rails

### Power Consumption

| Component | Current Draw | Notes |
|-----------|--------------|-------|
| MSP430F6659 (active) | ~5-10 mA | At 32 MHz |
| MSP430F6659 (sleep) | <1 µA | Deep sleep mode |
| TTC Radio (RX) | ~20-30 mA | Per radio |
| TTC Radio (TX) | ~200-500 mA | Peak during transmission |
| MT25Q Flash | ~15 mA | During write |
| EPS I2C | <1 mA | Communication |
| Sensors | <1 mA | ADC active |
| Watchdog | <10 µA | Always on |

**Total (typical operation)**: 100-300 mA
**Total (beacon TX)**: 300-600 mA (peak)
**Total (hibernation)**: <10 mA

## Physical Integration

### Form Factor

**Size**: CubeSat PC/104 compatible
**Mounting**: Stackable with other satellite modules
**Connectors**: PC/104 headers (SPI, I2C, power, GPIO)

### Environmental Specifications

| Parameter | Specification |
|-----------|---------------|
| **Operating Temperature** | -40°C to +85°C |
| **Storage Temperature** | -55°C to +125°C |
| **Humidity** | Non-condensing |
| **Vibration** | Launch-rated (per CubeSat standard) |
| **Radiation** | TID tolerance (mission-dependent) |

### Interfaces to Satellite

**PC/104 Bus**:
- **Power**: +3.3V, +5V rails
- **I2C**: Shared bus with other modules
- **SPI**: Dedicated lines to payloads
- **UART**: Debug/logging interface
- **GPIO**: Discrete control signals

## Hardware Documentation

### Schematics and PCB

**Location**: hardware/ directory (if available)

**Files**:
- Schematic diagrams (PDF)
- PCB layout files (Gerber, drill files)
- Bill of Materials (BOM)
- Assembly drawings

**Licensing**: CERN OHL v2 (Open Hardware License)

### Component Datasheets

**Key Components**:
- MSP430F6659: Texas Instruments SLAS645
- MT25Q: Micron Flash memory datasheet
- CY15x102QN: Cypress FRAM datasheet
- Si4463: Silicon Labs radio IC datasheet
- TPS382x: TI watchdog datasheet
- TCA4311A: TI I2C level shifter datasheet

### Design Tools

**Schematic Capture**: KiCad, Altium Designer, or Eagle
**PCB Layout**: KiCad, Altium Designer
**Firmware Development**: Code Composer Studio (CCS)

## Testing and Validation

### Hardware Testing

**Location**: firmware/tests/ (firmware-level tests)

**Test Types**:
- **Unit Tests**: Individual component verification
- **Integration Tests**: Inter-component communication
- **Environmental Tests**: Temperature cycling, vibration
- **Functional Tests**: End-to-end system operation

### Test Equipment

- **Logic Analyzer**: SPI, I2C, UART protocol analysis
- **Oscilloscope**: Signal integrity verification
- **Multimeter**: Voltage, current measurements
- **JTAG Debugger**: Firmware debugging (MSP-FET)
- **Thermal Chamber**: Temperature testing
- **Vibration Table**: Launch qualification

### Debug Interfaces

**JTAG** (MSP430 Spy-Bi-Wire):
- 2-wire debug interface (SBWTDIO, SBWTCK)
- Programming and real-time debugging
- Breakpoints, watchpoints, memory inspection

**UART Logging** (firmware/system/sys_log/):
- Real-time system log output
- 115200 bps (UART_PORT_0)
- Debug levels: ERROR, WARNING, INFO, DEBUG

## Manufacturing

### PCB Specifications

- **Layers**: 4-layer (typical for signal integrity)
- **Material**: FR-4
- **Copper Weight**: 1 oz (35 µm)
- **Finish**: ENIG (Electroless Nickel Immersion Gold)
- **Solder Mask**: Green (standard)

### Assembly

- **SMT Components**: Reflow soldering
- **Through-Hole**: Wave soldering or hand assembly
- **Conformal Coating**: Optional for environmental protection

### Quality Control

- **Visual Inspection**: Component placement verification
- **Automated Optical Inspection (AOI)**: Solder joint quality
- **Functional Test**: Power-up and basic operation
- **Burn-In**: Extended operation test

## Hardware Architecture Summary

The OBDH2 hardware provides:

1. **Central Processing**: MSP430F6659 (256 KB Flash, 16 KB RAM, 32 MHz)
2. **External Storage**: MT25Q NOR Flash (up to 256 MB), CY15x102QN FRAM (64 KB)
3. **Radio Communication**: Dual SL_TTC2 modules (UHF/VHF, redundancy)
4. **Power Interface**: SL_EPS2 (I2C, battery/solar monitoring)
5. **Antenna Deployment**: ISIS Antenna (I2C, 4-channel)
6. **Sensors**: Temperature, voltage, current (ADC inputs)
7. **Protection**: TPS382x external watchdog, TCA4311A I2C buffer
8. **Expandability**: Multiple SPI/I2C/UART ports for payloads

**Design Philosophy**:
- **Modularity**: Stackable PC/104 form factor
- **Redundancy**: Dual radios, external watchdog
- **Low Power**: Ultra-low-power MCU, sleep modes
- **Robustness**: Environmental qualification, fault recovery
- **Flexibility**: Configurable interfaces, payload support

This hardware platform provides a reliable, feature-rich on-board computer for small satellite missions.
