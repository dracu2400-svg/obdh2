# OBDH 2.0 Hardware Components and Architecture

## Overview

This document describes the hardware design, components, and electrical architecture of the OBDH 2.0 (On-Board Data Handling 2.0) module for the FloripaSat-2 CubeSat mission.

**Developer:** SpaceLab - Federal University of Santa Catarina (UFSC)
**License:** CERN Open Hardware License v2.0
**Form Factor:** CubeSat standard PC-104 compatible
**Dimensions:** 90 x 93 mm
**Target Environment:** Low Earth Orbit (LEO)

---

## System Block Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        OBDH 2.0 MODULE                          │
│                                                                 │
│  ┌────────────────────────────────────────────────────────┐    │
│  │         MSP430F6659 Microcontroller                    │    │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐             │    │
│  │  │ CPU Core │  │  Flash   │  │   RAM    │             │    │
│  │  │ 16-bit   │  │  640 KB  │  │  64 KB   │             │    │
│  │  │ 32 MHz   │  │          │  │          │             │    │
│  │  └──────────┘  └──────────┘  └──────────┘             │    │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐             │    │
│  │  │ UART x3  │  │ SPI x6   │  │ I2C x3   │             │    │
│  │  └──────────┘  └──────────┘  └──────────┘             │    │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐             │    │
│  │  │ ADC 12b  │  │   RTC    │  │   WDT    │             │    │
│  │  └──────────┘  └──────────┘  └──────────┘             │    │
│  └────────────────────────────────────────────────────────┘    │
│                            ↕                                    │
│  ┌─────────────────────────────────────────────┐               │
│  │           Memory Subsystem                  │               │
│  │  ┌──────────────┐  ┌──────────────┐         │               │
│  │  │ NOR Flash    │  │    FRAM      │         │               │
│  │  │ MT25Q        │  │ CY15x102QN   │         │               │
│  │  │ 256 MB       │  │  256 KB      │         │               │
│  │  │ (SPI)        │  │  (SPI)       │         │               │
│  │  └──────────────┘  └──────────────┘         │               │
│  └─────────────────────────────────────────────┘               │
│                            ↕                                    │
│  ┌─────────────────────────────────────────────┐               │
│  │          I2C Bus (Primary)                  │               │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────┐    │               │
│  │  │   EPS    │ │  TTC 0   │ │  TTC 1   │    │               │
│  │  │ (0x36)   │ │  (0x40)  │ │  (0x50)  │    │               │
│  │  └──────────┘ └──────────┘ └──────────┘    │               │
│  │  ┌──────────┐ ┌──────────┐                 │               │
│  │  │ Antenna  │ │  Temp    │                 │               │
│  │  │ (0x31)   │ │ Sensors  │                 │               │
│  │  └──────────┘ └──────────┘                 │               │
│  └─────────────────────────────────────────────┘               │
│                            ↕                                    │
│  ┌─────────────────────────────────────────────┐               │
│  │          UART Interface                     │               │
│  │  ┌──────────────┐  ┌──────────────┐         │               │
│  │  │    Debug     │  │     EDC      │         │               │
│  │  │  (115200)    │  │  Payload     │         │               │
│  │  └──────────────┘  └──────────────┘         │               │
│  └─────────────────────────────────────────────┘               │
│                            ↕                                    │
│  ┌─────────────────────────────────────────────┐               │
│  │       Analog Sensing Subsystem              │               │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────┐    │               │
│  │  │ Voltage  │ │ Current  │ │   Temp   │    │               │
│  │  │ Divider  │ │  Sense   │ │  Sensor  │    │               │
│  │  └──────────┘ └──────────┘ └──────────┘    │               │
│  └─────────────────────────────────────────────┘               │
│                            ↕                                    │
│  ┌─────────────────────────────────────────────┐               │
│  │          Power Subsystem                    │               │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────┐    │               │
│  │  │   LDO    │ │ Voltage  │ │   WDT    │    │               │
│  │  │  3.3V    │ │ Monitor  │ │ TPS382x  │    │               │
│  │  └──────────┘ └──────────┘ └──────────┘    │               │
│  └─────────────────────────────────────────────┘               │
│                            ↕                                    │
│  ┌─────────────────────────────────────────────┐               │
│  │        External Interfaces                  │               │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────┐    │               │
│  │  │  PC-104  │ │   JTAG   │ │   LEDs   │    │               │
│  │  │  Power/  │ │  Debug   │ │  Status  │    │               │
│  │  │   Data   │ │  Port    │ │          │    │               │
│  │  └──────────┘ └──────────┘ └──────────┘    │               │
│  └─────────────────────────────────────────────┘               │
└─────────────────────────────────────────────────────────────────┘
```

---

## Core Microcontroller

### MSP430F6659

**Manufacturer:** Texas Instruments
**Family:** MSP430 16-bit RISC
**Package:** 100-pin LQFP (Low-profile Quad Flat Package)

#### Technical Specifications

| Parameter | Value | Notes |
|-----------|-------|-------|
| **CPU** | 16-bit RISC | Von Neumann architecture |
| **Clock Speed** | 32 MHz | Main clock (MCLK) |
| **Flash Memory** | 640 KB | 128 KB main + 512 KB extended |
| **RAM** | 64 KB + 4 KB | Main RAM + USB RAM |
| **Operating Voltage** | 1.8V - 3.6V | Typically 3.3V |
| **Active Mode Current** | ~5 mA @ 32 MHz | Typical at 3.3V |
| **LPM3 Current** | ~2 µA | Low power mode 3 (RTC active) |
| **Operating Temp** | -40°C to +85°C | Industrial grade |
| **I/O Pins** | 74 | Multiplexed GPIO |

#### Integrated Peripherals

**Communication:**
- **UART (eUSCI_A):** 3 ports, up to 4 Mbps
- **SPI (eUSCI_A/B):** 6 ports, master/slave, up to 16 MHz
- **I2C (eUSCI_B):** 3 ports, master/slave, up to 400 kHz

**Analog:**
- **ADC12_A:** 12-bit SAR ADC, 16 channels, up to 200 ksps
- **Internal Temp Sensor:** ±2°C accuracy
- **Internal Voltage Reference:** 1.5V, 2.0V, 2.5V

**Timers:**
- **Timer_A:** 3 instances (TA0, TA1, TA2), 16-bit, up to 7 capture/compare
- **Timer_B:** 1 instance (TB0), 16-bit, up to 7 capture/compare
- **RTC_B:** Real-time clock with calendar

**Security:**
- **JTAG Fuse:** Can disable JTAG for code protection
- **Flash Write Protection:** Segment-level protection

**Other:**
- **Watchdog Timer (WDT_A):** Configurable timeout
- **DMA:** 3-channel direct memory access
- **USB:** USB 2.0 full-speed device (not used in OBDH)

#### Clock System

```
External Crystals
┌────────────────┐
│  XT1: 32.768kHz│─────┐
│  (RTC)         │     │
└────────────────┘     │
                       ↓
┌────────────────┐   ┌─────────────┐
│  XT2: 32 MHz   │──→│ FLL / DCO   │
│  (Main Clock)  │   │ Frequency   │
└────────────────┘   │ Locked Loop │
                     └─────────────┘
                           ↓
        ┌──────────────────┼──────────────────┐
        ↓                  ↓                  ↓
    ┌──────┐          ┌──────┐          ┌──────┐
    │ MCLK │          │SMCLK │          │ ACLK │
    │32 MHz│          │32 MHz│          │32kHz │
    └──────┘          └──────┘          └──────┘
       ↓                 ↓                 ↓
    CPU Core      Peripherals           RTC/Timers
```

**Clock Sources:**
- **MCLK (Main Clock):** 32 MHz - CPU execution
- **SMCLK (Sub-Main Clock):** 32 MHz - Fast peripherals (SPI, UART)
- **ACLK (Auxiliary Clock):** 32.768 kHz - Low-power timers, RTC

**Crystals:**
- **XT1:** 32.768 kHz crystal for RTC (pins P7.0-P7.1)
- **XT2:** 32 MHz crystal for high-speed operation (pins P5.2-P5.3)

---

## Memory Subsystem

### Internal Flash Memory

**Type:** Non-volatile Flash (NOR-type)
**Capacity:** 640 KB total
- Main Flash: 128 KB (0x8000-0xFFFF)
- Extended Flash: 512 KB (0x10000-0x8FFFF)

**Segment Size:** 512 bytes (typical)
**Erase Granularity:** Segment or bank
**Write Granularity:** Byte or word
**Endurance:** 10,000 write/erase cycles (typical)
**Retention:** 100 years @ 25°C

**Usage:**
- Firmware code
- Constant data
- Configuration parameters
- Bootloader (if implemented)

**Info Memory (INFOA-INFOD):**
- 4 segments of 128 bytes each
- Protected from accidental erase
- Used for calibration data, keys, boot counter

---

### External NOR Flash (MT25Q)

**Part Number:** Micron MT25QL256ABA (or compatible)
**Manufacturer:** Micron (now Kioxia)
**Technology:** Serial NOR Flash
**Capacity:** 256 MB (2 Gb)
**Interface:** SPI (Quad SPI capable)
**Voltage:** 2.7V - 3.6V (3.3V nominal)
**Package:** SOIC-16 or WSON-8

#### Specifications

| Parameter | Value |
|-----------|-------|
| **Organization** | 256 Mb / 32 MB |
| **Page Size** | 256 bytes |
| **Sector Size** | 4 KB (small) / 64 KB (large) |
| **Clock Speed** | Up to 133 MHz (Quad SPI) |
| **Typical Speed** | 1 MHz (conservative) |
| **Erase Time (4KB)** | ~50 ms |
| **Erase Time (64KB)** | ~200 ms |
| **Program Time (Page)** | ~0.7 ms |
| **Endurance** | 100,000 cycles |
| **Retention** | 20 years |
| **Current (Active)** | ~15 mA @ 1 MHz |
| **Current (Standby)** | ~10 µA |
| **Current (Deep PD)** | ~1 µA |

#### Memory Organization

```
Address Range: 0x00000000 - 0x01FFFFFF (32 MB)

┌──────────────────────────────────┐ 0x01FFFFFF
│                                  │
│     Long-term Telemetry Log      │
│       (Circular Buffer)          │
│          ~30 MB                  │
│                                  │
├──────────────────────────────────┤ 0x00200000
│                                  │
│     Payload Data Archive         │
│          ~2 MB                   │
│                                  │
├──────────────────────────────────┤ 0x00000000
│     Reserved / Configuration     │
│          64 KB                   │
└──────────────────────────────────┘
```

**Usage:**
- Long-term telemetry storage (days to weeks)
- Payload data archiving
- Mission event logs
- Downloadable data for ground station

**Driver:** `drivers/mt25q/mt25q.c/h`

---

### External FRAM (CY15x102QN)

**Part Number:** Cypress CY15B102QN or CY15V102QN
**Manufacturer:** Infineon (formerly Cypress)
**Technology:** Ferroelectric RAM (F-RAM)
**Capacity:** 256 KB (2 Mb)
**Interface:** SPI
**Voltage:** 2.7V - 3.6V (3.3V nominal)
**Package:** SOIC-8

#### Specifications

| Parameter | Value |
|-----------|-------|
| **Organization** | 256 Kb / 32 KB |
| **Access Time** | 70 ns (typical) |
| **Clock Speed** | Up to 40 MHz |
| **Write Speed** | Fast (no erase needed) |
| **Endurance** | Unlimited (>10^14 cycles) |
| **Retention** | 10 years @ 85°C, 151 years @ 65°C |
| **Current (Active)** | ~1.5 mA @ 1 MHz |
| **Current (Standby)** | ~3 µA |

#### Memory Organization

```
Address Range: 0x0000 - 0x7FFF (32 KB)

┌──────────────────────────────────┐ 0x7FFF
│                                  │
│   Telemetry Circular Buffer      │
│       (Last ~100 entries)        │
│          24 KB                   │
│                                  │
├──────────────────────────────────┤ 0x1FFF
│   Telecommand History Log        │
│          4 KB                    │
├──────────────────────────────────┤ 0x0FFF
│   System Configuration           │
│          2 KB                    │
├──────────────────────────────────┤ 0x07FF
│   Boot Counter, Reset Cause      │
│          1 KB                    │
├──────────────────────────────────┤ 0x03FF
│   Reserved / Future Use          │
│          1 KB                    │
└──────────────────────────────────┘ 0x0000
```

**Advantages of FRAM:**
- **No Erase Needed:** Direct byte-level write (faster than flash)
- **Unlimited Endurance:** Suitable for high-frequency data logging
- **Low Power:** Lower write current than flash
- **Fast Write:** No page/sector erase delays

**Usage:**
- Recent telemetry cache
- Configuration parameters (frequently updated)
- System state preservation across resets
- Telecommand history

**Driver:** `drivers/cy15x102qn/cy15x102qn.c/h`

---

## Power Subsystem

### Power Input

**Primary Power Source:** PC-104 connector from EPS (Electrical Power System)

**Input Voltage:** 3.3V ± 5% (regulated by EPS)
**Backup Voltage:** 5V or 12V rails (if 3.3V fails)
**Maximum Current:** 500 mA (typical), 1 A (peak)

---

### Voltage Regulation

#### 3.3V LDO Regulator

**Part Number:** TBD (e.g., TPS7A4700, ADP150)
**Input:** 5V from PC-104
**Output:** 3.3V ± 2%
**Current:** Up to 1 A
**Dropout Voltage:** <200 mV
**Quiescent Current:** <50 µA

**Features:**
- Low noise (for ADC reference)
- Thermal shutdown
- Short-circuit protection

**Load Capacitors:**
- Input: 10 µF tantalum + 100 nF ceramic
- Output: 10 µF tantalum + 100 nF ceramic (close to IC)

---

### Power Distribution

```
PC-104 Connector (from EPS)
    │
    ├─ 3.3V Rail ────────┬─→ MSP430F6659 (DVCC, AVCC)
    │                    ├─→ NOR Flash (VCC)
    │                    ├─→ FRAM (VCC)
    │                    ├─→ Voltage Monitor (TPS382x)
    │                    └─→ Other ICs
    │
    ├─ 5V Rail ──→ LDO ──→ 3.3V (backup)
    │
    └─ GND (common ground)
```

**Decoupling:**
- 100 nF ceramic capacitor per IC VCC pin (close placement)
- 10 µF tantalum capacitor per power rail
- Separate analog and digital ground planes (if feasible)

---

### Voltage Monitor and Watchdog (TPS382x)

**Part Number:** TPS3823 or TPS3825 series
**Manufacturer:** Texas Instruments
**Function:** Voltage supervisor and watchdog timer

#### Features

- **Brownout Detection:** Resets MCU if voltage drops below threshold
- **Watchdog Timer:** Independent external watchdog
- **Manual Reset:** Push-button reset input
- **Reset Delay:** Configurable (e.g., 200 ms after power-up)

#### Specifications

| Parameter | Value |
|-----------|-------|
| **Voltage Threshold** | 2.93V (for 3.3V rail, ±1.5%) |
| **Reset Timeout** | 200 ms (typical) |
| **Watchdog Timeout** | 1.6 s (typical, configurable) |
| **Supply Current** | <1 µA |

**Connection:**
- VDD: 3.3V rail
- SENSE: 3.3V rail (or divided voltage)
- WDI: Watchdog input (from MSP430 GPIO)
- RESET: Reset output to MSP430 RST pin

**Operation:**
1. MSP430 toggles WDI pin every ~100 ms (Watchdog Reset Task)
2. If WDI not toggled within timeout, TPS382x asserts RESET
3. MSP430 resets and restarts

---

### Power Consumption Estimate

| Component | Current (mA) | Notes |
|-----------|--------------|-------|
| MSP430F6659 | 5 | Active @ 32 MHz |
| NOR Flash | 0.01 | Standby |
| FRAM | 0.003 | Standby |
| TPS382x | 0.001 | Voltage monitor |
| LEDs (3x) | 6 | ~2 mA each (if on) |
| I2C Pull-ups | 1 | Quiescent current |
| **Total (Idle)** | **~12 mA** | |
| **Total (Active)** | **~30 mA** | With NOR flash write, I2C comms |
| **Peak (Transmit)** | **50-100 mA** | If OBDH controls high-power device |

**Average Power:** ~40-100 mW (typical)

---

## Communication Interfaces (External Devices)

### I2C Bus Devices

#### EPS (Electrical Power System) - SpaceLab EPS v2

**Interface:** I2C (slave)
**I2C Address:** 0x36 (configurable)
**Driver:** `drivers/sl_eps2/`

**Function:**
- Battery management
- Solar panel monitoring
- Power distribution control
- Heater control

**Communication:**
- Request telemetry: 50-100 bytes
- Control commands: Enable/disable modules

---

#### TTC (Telemetry/Telecommand) - SpaceLab TTC v2

**Interface:** I2C (slave)
**I2C Addresses:**
- TTC 0: 0x40
- TTC 1: 0x50

**Driver:** `drivers/sl_ttc2/`

**Function:**
- UHF/VHF radio transceiver
- Packet transmission/reception
- Beacon generation
- Telemetry reporting

**Communication:**
- Send packet: Write packet to I2C
- Receive packet: Read from I2C buffer
- Request telemetry: Read status

---

#### ISIS Antenna Deployer

**Interface:** I2C or SPI (configurable)
**I2C Address:** 0x31-0x33 (configurable)
**Driver:** `drivers/isis_antenna/`

**Function:**
- Antenna deployment control
- Deployment status monitoring
- Temperature monitoring

**Antenna System:**
- 4 independent antennas
- Burn wire deployment mechanism
- Burn time: 10-20 seconds per antenna

---

#### Temperature Sensors

**Part Number:** TMP100, TMP102, or similar
**Interface:** I2C (slave)
**I2C Address:** 0x48-0x4F (configurable via address pins)

**Function:**
- Measure ambient temperature
- Monitor critical component temperatures

**Locations:**
- Radio module
- Antenna module
- OBDH PCB

**Resolution:** 12-bit (0.0625°C per LSB)

---

### SPI Bus Devices

**Devices:**
- NOR Flash (MT25Q) - See Memory Subsystem
- FRAM (CY15x102QN) - See Memory Subsystem
- ISIS Antenna (if SPI mode)

**Chip Select Pins:**
- NOR Flash CS: P6.0 (GPIO_PIN_40)
- FRAM CS: P6.1 (GPIO_PIN_41)
- Antenna CS: P6.2 (GPIO_PIN_42)

---

### UART Devices

#### Debug Port (UART 0)

**Interface:** UART
**Baud Rate:** 115200 bps
**Pins:** P3.0 (TX), P3.1 (RX)

**Connection:**
- USB-to-UART adapter (FTDI FT232R or CP2102)
- 3.3V logic levels

**Usage:**
- Debug logging (sys_log)
- Command-line interface (if implemented)
- Firmware upload (if bootloader implemented)

---

#### EDC Payload (UART 1)

**Interface:** UART
**Baud Rate:** 115200 bps (configurable)
**Pins:** P4.4 (TX), P4.5 (RX)

**Connection:**
- EDC (Earth Data Collection) module
- Payload-specific protocol

**Usage:**
- Request payload data
- Configure payload parameters

---

## Analog Sensing Subsystem

### Voltage Measurement

**Circuit:**
```
Input Voltage (0-6.6V)
    │
    ├─ R1 (10kΩ) ─┬─ ADC Input (P4.0)
    │             │
    └─ R2 (10kΩ) ─┴─ GND

Divider Ratio: 2:1
ADC Range: 0-3.3V → Input Range: 0-6.6V
```

**ADC Channel:** A0 (P4.0)
**Resolution:** 12-bit (0.8 mV per LSB)
**Accuracy:** ±2% (with resistor tolerance)

**Conversion:**
```c
uint16_t adc_raw = adc_read_channel(ADC_CHANNEL_0);
float adc_voltage = (adc_raw / 4096.0) * 2.5;  // 2.5V reference
float input_voltage = adc_voltage * 2.0;       // Divider ratio
```

---

### Current Measurement

**Circuit:**
```
Input Current
    │
    └─ Shunt Resistor (0.1Ω) ─┬─ Op-Amp (Gain 50) ─┬─ ADC Input (P4.1)
                              │                     │
                             GND                   GND

Gain: 50x
Shunt: 0.1Ω
ADC Range: 0-2.5V → Current Range: 0-500 mA
```

**ADC Channel:** A1 (P4.1)
**Shunt Resistor:** 0.1Ω, 1%, 0.5W
**Op-Amp:** Low-offset, rail-to-rail (e.g., OPA2333)
**Gain:** 50 (configurable via resistors)

**Conversion:**
```c
uint16_t adc_raw = adc_read_channel(ADC_CHANNEL_1);
float adc_voltage = (adc_raw / 4096.0) * 2.5;
float current = (adc_voltage / 50.0) / 0.1;  // Amps
uint16_t current_ma = current * 1000;
```

---

### Temperature Measurement

**Internal Temperature (MSP430):**
- ADC channel: A10 (internal)
- Formula: `T(°C) = (V_sensor - 0.986) / 0.00355`

**External Temperature (Analog Sensor):**
- ADC channel: A2 (P4.2)
- Sensor: LM35 or TMP36 (linear analog output)

**Digital Temperature Sensors:**
- I2C temperature sensors (TMP100, TMP102)
- More accurate, no ADC noise

---

## External Interfaces

### PC-104 Connector

**Type:** PC-104 standard (104-pin stacking connector)
**Purpose:** Power and data interface to satellite backplane

**Pin Assignments:**

| Pin Group | Signals | Usage |
|-----------|---------|-------|
| Power | 3.3V, 5V, 12V, GND | Power input from EPS |
| I2C | SDA, SCL | Shared I2C bus to other modules |
| UART | TXD, RXD | Inter-module UART (optional) |
| SPI | MOSI, MISO, SCK, CS | Inter-module SPI (optional) |
| GPIO | Control, Status | Module enable, fault signals |

**Mechanical:**
- Stacking height: 15 mm (typical)
- PCB dimensions: 90 x 96 mm (PC-104 standard)
- Mounting holes: 4x M3 (compatible with CubeSat structure)

---

### JTAG Programming Interface

**Connector:** 14-pin or 20-pin JTAG header (2.54 mm pitch)
**Programmer:** Texas Instruments MSP-FET
**Signals:** TDI, TDO, TMS, TCK, RST, TEST, VCC, GND

**JTAG Header Pinout (14-pin TI standard):**
```
1: VCC     2: VCC
3: TDI     4: GND
5: TMS     6: GND
7: TCK     8: GND
9: TDO    10: GND
11: RST   12: GND
13: TEST  14: GND
```

**Usage:**
- Firmware programming
- In-circuit debugging
- Flash memory programming

**Security:**
- JTAG fuse can be blown to disable external access (for flight units)

---

### LED Indicators

**Quantity:** 3 status LEDs

| LED | Color | GPIO Pin | Usage |
|-----|-------|----------|-------|
| LED 0 | Green | P1.0 (GPIO_PIN_0) | Heartbeat (1 Hz blink) |
| LED 1 | Yellow | P1.1 (GPIO_PIN_1) | Communication Activity |
| LED 2 | Red | P1.2 (GPIO_PIN_2) | Error / Fault Indicator |

**Circuit:**
```
GPIO Pin ─┬─ Resistor (330Ω) ─┬─ LED Anode
          │                   │
         GND               LED Cathode ─ GND

Active High: LED on when GPIO high
```

**Current:** ~5-10 mA per LED
**Voltage Drop:** 2.0V (red), 2.2V (yellow), 2.8V (green)

---

## PCB Design

### Board Specifications

**Dimensions:** 90 x 93 mm (CubeSat PC-104 compatible)
**Layers:** 2-layer or 4-layer FR-4
**Thickness:** 1.6 mm (standard)
**Copper Weight:** 1 oz (35 µm) or 2 oz (70 µm) for power traces

**Design Considerations:**
- **Ground Plane:** Continuous ground plane (bottom layer or internal)
- **Power Plane:** Dedicated 3.3V power plane (if 4-layer)
- **Signal Routing:** Minimize trace lengths for high-speed signals (SPI, I2C)
- **Decoupling:** Capacitors close to IC power pins (<5 mm)
- **Thermal:** Thermal vias under power ICs

---

### Component Placement

**Critical Placement:**
1. **MSP430F6659:** Center of board for shortest traces to peripherals
2. **Crystals (XT1, XT2):** Close to MSP430 clock pins (<10 mm)
3. **Decoupling Capacitors:** Immediately adjacent to IC power pins
4. **NOR Flash, FRAM:** Near SPI pins of MSP430
5. **Power Regulators:** Near power input connector, with thermal relief

---

### Shielding and EMI

**EMI Considerations:**
- **Clock Signals:** Route away from sensitive analog traces
- **SPI/I2C:** Differential routing if high speed (>10 MHz)
- **ADC Inputs:** Guard traces or ground isolation
- **RF:** Keep TTC modules isolated from digital circuits

**Shielding:**
- Metal enclosure (if required for EMI compliance)
- Ground stitching vias around sensitive areas

---

## Environmental Specifications

### Operating Conditions

| Parameter | Min | Typ | Max | Unit |
|-----------|-----|-----|-----|------|
| **Temperature (Operating)** | -20 | +25 | +60 | °C |
| **Temperature (Storage)** | -40 | - | +85 | °C |
| **Humidity** | 0 | - | 95 | % RH (non-condensing) |
| **Pressure** | Vacuum | - | 1 atm | |

**Space Environment Considerations:**
- **Radiation:** Total Ionizing Dose (TID) ~10 krad (LEO, ~2 years)
- **Single Event Upsets (SEU):** Watchdog and reset mechanisms for recovery
- **Thermal Cycling:** -40°C to +85°C (eclipse to sun exposure)
- **Vacuum:** Outgassing-compliant materials (low TML, CVCM)

---

### Radiation Tolerance

**MSP430F6659:**
- Not radiation-hardened
- Susceptible to SEU (Single Event Upset)
- Mitigation: Software-based (watchdog, CRC checks, redundancy)

**Memory:**
- Flash/FRAM: Generally radiation-tolerant
- Error detection: CRC checks on critical data

**SEU Mitigation Strategies:**
1. Watchdog timer (automatic recovery from lockup)
2. Periodic system reset
3. Redundant data storage (FRAM + NOR)
4. CRC validation on read
5. Telemetry monitoring by ground station

---

## Mechanical Design

### Mounting

**Form Factor:** CubeSat 1U module (90 x 93 mm)
**Mounting Holes:** 4x M3 threaded inserts (compatible with CubeSat rails)
**Stacking:** PC-104 connectors allow vertical stacking with other modules

**Assembly:**
1. Mount OBDH PCB to CubeSat structure using M3 screws
2. Connect PC-104 power/data connector to satellite backplane
3. Route external cables (JTAG, debug UART) through CubeSat panels

---

### Thermal Management

**Heat Dissipation:**
- MSP430F6659: ~165 mW @ 32 MHz
- NOR Flash: ~15 mW (active)
- Total: <500 mW

**Thermal Design:**
- Passive cooling (no fans in space)
- Thermal conduction through PCB to satellite structure
- Thermal vias under high-power ICs

**Thermal Interface:**
- Thermal pads or paste between OBDH and satellite structure
- Ensure good thermal contact for heat dissipation

---

## Bill of Materials (BOM) Summary

| Category | Key Components |
|----------|----------------|
| **Microcontroller** | MSP430F6659 (100-LQFP) |
| **Memory** | MT25Q256 (NOR Flash, 256 MB), CY15B102QN (FRAM, 256 KB) |
| **Power** | LDO 3.3V regulator, TPS382x voltage monitor |
| **Crystals** | 32 MHz (XT2), 32.768 kHz (XT1) |
| **Communication** | PC-104 connector (104-pin) |
| **Connectors** | JTAG header (14-pin), Debug UART header |
| **Passives** | Resistors (0603), Capacitors (0603/0805), Inductors |
| **LEDs** | 3x status LEDs (0603 or through-hole) |
| **Sensors** | Temperature sensors (optional) |

**Estimated Component Count:** ~150-200 components (including passives)

---

## Testing and Qualification

### Functional Testing

1. **Power-On Test:** Verify 3.3V rail, check current draw
2. **JTAG Programming:** Flash firmware, verify upload
3. **UART Communication:** Test debug port at 115200 bps
4. **I2C Bus:** Scan for devices (EPS, TTC, sensors)
5. **SPI Devices:** Read/write NOR Flash and FRAM
6. **ADC:** Verify voltage, current, temperature readings
7. **Watchdog:** Test watchdog reset functionality
8. **LED Indicators:** Verify heartbeat, activity, error LEDs

---

### Environmental Testing

1. **Thermal Vacuum:** -40°C to +85°C cycling in vacuum
2. **Vibration:** Random vibration per NASA-STD-7001A
3. **Shock:** Pyroshock testing (if applicable)
4. **EMI/EMC:** Electromagnetic compatibility testing
5. **Radiation:** TID testing (optional, for critical missions)

---

## Related Documentation

- `00_functionalities_of_system.md` - System overview and functionalities
- `01_firmware_architecture.md` - Software architecture
- `02_datapath.md` - Data structures and flow
- `03_interfaces.md` - Communication protocols
- `04_tasks.md` - FreeRTOS task descriptions
- `05_pin_mapping.md` - Detailed pin assignments

---

## Design Files

**Hardware Design:**
- Schematic: `hardware/obdh2_schematic.pdf`
- PCB Layout: `hardware/obdh2_pcb.kicad_pcb` (KiCad format)
- BOM: `hardware/obdh2_bom.csv`
- Gerber Files: `hardware/gerbers/`

**Mechanical Design:**
- 3D Model: `hardware/obdh2_3d.step`
- Assembly Drawing: `hardware/obdh2_assembly.pdf`

---

## Revision History

| Version | Date | Changes |
|---------|------|---------|
| 0.10.0 | 2024 | Initial development version |

---

**Document Status:** Development
**Last Updated:** 2024
**Prepared by:** SpaceLab - UFSC
