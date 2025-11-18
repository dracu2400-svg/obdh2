# OBDH2 Data Path and Flow

## Overview

This document describes the data acquisition, processing, storage, and transmission paths within the OBDH2 system. It details how data flows from sensors and subsystems through the firmware to ground station communication and non-volatile storage.

## Data Flow Architecture

The OBDH2 implements three primary data paths:

1. **Acquisition Path**: Sensors/Modules → Read Tasks → Global Buffer
2. **Transmission Path**: Global Buffer → Packet Formatting → TTC → Ground Station
3. **Storage Path**: Global Buffer → Data Log Task → Non-Volatile Memory

## Global Data Buffer

### Centralized Telemetry Storage

**Location**: firmware/app/structs/satellite.h:1

All telemetry data converges into a single global buffer structure:

```c
sat_data_t sat_data_buf;  // Global satellite data buffer
```

### Data Structure Definition

```c
typedef struct {
    obdh_telemetry_t obdh;        // OBDH housekeeping data
    eps_telemetry_t eps;           // EPS power subsystem data
    ttc_telemetry_t ttc_0;         // TTC radio 0 telemetry
    ttc_telemetry_t ttc_1;         // TTC radio 1 telemetry
    antenna_telemetry_t antenna;   // Antenna deployment status
    payload_telemetry_t edc_0;     // Payload EDC-1 data (220 bytes)
    payload_telemetry_t edc_1;     // Payload EDC-2 data (220 bytes)
    payload_telemetry_t payload_x; // Payload-X data (220 bytes)
    payload_telemetry_t harsh;     // HARSH payload data (220 bytes)
} sat_data_t;
```

### OBDH Telemetry Structure

**Location**: firmware/app/structs/obdh_data.h:1

```c
typedef struct {
    // Environmental sensors
    uint16_t temperature;           // MCU temperature (Kelvin)
    uint16_t current;               // Input current (milliamperes)
    uint16_t voltage;               // Input voltage (millivolts)

    // System status
    uint8_t reset_cause;            // Last reset cause code
    uint16_t reset_counter;         // Total reset count
    uint8_t last_valid_tc;          // Last valid telecommand ID

    // Radio subsystem
    int16_t radio_temperature;      // TTC temperature (Celsius)
    int8_t radio_rssi;              // Received Signal Strength Indicator

    // Antenna subsystem
    int16_t antenna_temperature;    // Antenna temperature (Celsius)
    uint8_t antenna_status;         // Deployment status flags

    // Version information
    uint8_t hardware_version;       // Hardware revision
    uint16_t firmware_version;      // Firmware version code

    // Operational mode
    uint8_t operation_mode;         // 0=Normal, 1=Hibernation

    // Timestamps
    uint32_t system_time;           // System time counter (seconds)
} obdh_telemetry_t;
```

## Data Acquisition Path

### Phase 1: Hardware Data Generation

Physical sensors and modules continuously generate data:

```
┌─────────────────────────────────────────────────────────┐
│ HARDWARE DATA SOURCES                                   │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │ EPS Module   │  │ TTC Module 0 │  │ TTC Module 1 │ │
│  │ (I2C Slave)  │  │ (SPI Slave)  │  │ (SPI Slave)  │ │
│  │ 0x36         │  │              │  │              │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
│        ↓                   ↓                  ↓         │
│  • Battery V/I        • RSSI            • RSSI         │
│  • Solar panels       • Temp            • Temp         │
│  • Temperature        • TX/RX pkts      • TX/RX pkts   │
│  • Power status       • Config          • Config       │
│                                                         │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │ Antenna      │  │ OBDH Sensors │  │ Payloads     │ │
│  │ (I2C Slave)  │  │ (ADC inputs) │  │ (SPI/I2C)    │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
│        ↓                   ↓                  ↓         │
│  • Deploy status      • Temperature     • EDC-1        │
│  • Temperature        • Voltage         • EDC-2        │
│  • Burn status        • Current         • Payload-X    │
│                                          • HARSH        │
└─────────────────────────────────────────────────────────┘
```

### Phase 2: Periodic Data Collection

**FreeRTOS read tasks** execute periodically to poll hardware:

#### Read Sensors Task

**Location**: firmware/app/tasks/read_sensors.c:1
**Period**: 1000 ms (1 second)
**Priority**: 3

```
┌──────────────────────────────────────────────┐
│ Read Sensors Task (1 Hz)                     │
├──────────────────────────────────────────────┤
│                                              │
│  1. temp_sensor_get_data()                   │
│     └─> ADC read → MCU die temperature      │
│         └─> sat_data_buf.obdh.temperature   │
│                                              │
│  2. voltage_sensor_get_data()                │
│     └─> ADC read → Input voltage            │
│         └─> sat_data_buf.obdh.voltage       │
│                                              │
│  3. current_sensor_get_data()                │
│     └─> ADC read → Input current            │
│         └─> sat_data_buf.obdh.current       │
│                                              │
└──────────────────────────────────────────────┘
```

#### Read EPS Task

**Location**: firmware/app/tasks/read_eps.c:1
**Period**: 1000 ms (1 second)
**Priority**: 3

```
┌──────────────────────────────────────────────┐
│ Read EPS Task (1 Hz)                         │
├──────────────────────────────────────────────┤
│                                              │
│  1. eps_get_data(&eps_data)                  │
│     └─> I2C transaction to EPS (0x36)       │
│         └─> Read power telemetry registers  │
│                                              │
│  2. Copy to global buffer                    │
│     └─> sat_data_buf.eps = eps_data         │
│                                              │
│  Data includes:                              │
│  • Battery voltage/current                   │
│  • Solar panel voltages/currents (3 axes)    │
│  • EPS temperature                           │
│  • Module status flags                       │
│  • Heater status                             │
│                                              │
└──────────────────────────────────────────────┘
```

#### Read TTC Task

**Location**: firmware/app/tasks/read_ttc.c:1
**Period**: 100 ms (10 Hz, high frequency)
**Priority**: 4 (High)

```
┌──────────────────────────────────────────────┐
│ Read TTC Task (10 Hz)                        │
├──────────────────────────────────────────────┤
│                                              │
│  For each TTC module (TTC_0, TTC_1):        │
│                                              │
│  1. ttc_get_data(TTC_x, &ttc_data)           │
│     └─> SPI transaction                     │
│         └─> Read housekeeping registers     │
│             • Temperature                    │
│             • RSSI                           │
│             • TX/RX packet counts            │
│                                              │
│  2. ttc_recv(TTC_x, rx_buffer, &len)         │
│     └─> Check for received packets          │
│         └─> Extract uplink telecommands     │
│             └─> Queue for processing        │
│                                              │
│  3. Update global buffer                     │
│     └─> sat_data_buf.ttc_0/1 = ttc_data     │
│                                              │
└──────────────────────────────────────────────┘
```

#### Read Antenna Task

**Location**: firmware/app/tasks/read_antenna.c:1
**Period**: 1000 ms (1 second)
**Priority**: 2

```
┌──────────────────────────────────────────────┐
│ Read Antenna Task (1 Hz)                     │
├──────────────────────────────────────────────┤
│                                              │
│  1. antenna_get_data(&antenna_data)          │
│     └─> I2C transaction to ISIS Antenna     │
│         └─> Read deployment status          │
│         └─> Read temperature                │
│         └─> Read antenna counter            │
│                                              │
│  2. Update global buffer                     │
│     └─> sat_data_buf.antenna = antenna_data │
│                                              │
└──────────────────────────────────────────────┘
```

#### Read EDC (Payload) Task

**Location**: firmware/app/tasks/read_edc.c:1
**Period**: 1000 ms (1 second)
**Priority**: 3

```
┌──────────────────────────────────────────────┐
│ Read EDC Task (1 Hz)                         │
├──────────────────────────────────────────────┤
│                                              │
│  For each payload (EDC_0, EDC_1, etc.):     │
│                                              │
│  1. payload_get_data(PAYLOAD_x, &data)       │
│     └─> SPI/I2C transaction                 │
│         └─> Read up to 220 bytes            │
│                                              │
│  2. Update global buffer                     │
│     └─> sat_data_buf.edc_0 = payload_data   │
│     └─> sat_data_buf.edc_1 = payload_data   │
│     └─> sat_data_buf.payload_x = ...        │
│     └─> sat_data_buf.harsh = ...            │
│                                              │
└──────────────────────────────────────────────┘
```

### Phase 3: Data Aggregation

All read tasks update their respective sections of `sat_data_buf`:

```
sat_data_buf {
    .obdh     ← Read Sensors Task (1 Hz)
    .eps      ← Read EPS Task (1 Hz)
    .ttc_0    ← Read TTC Task (10 Hz)
    .ttc_1    ← Read TTC Task (10 Hz)
    .antenna  ← Read Antenna Task (1 Hz)
    .edc_0    ← Read EDC Task (1 Hz)
    .edc_1    ← Read EDC Task (1 Hz)
    .payload_x ← Read EDC Task (1 Hz)
    .harsh    ← Read EDC Task (1 Hz)
}
```

**Synchronization**: Each task updates its own section; no mutexes needed due to non-overlapping writes.

## Command Processing Path (Uplink)

### Uplink Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│ UPLINK PATH: Ground Station → OBDH                         │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Ground Station                                             │
│      │                                                      │
│      ▼                                                      │
│  Radio Transmission (VHF uplink)                            │
│      │                                                      │
│      ▼                                                      │
│  ┌────────────────────────────────────┐                    │
│  │ TTC Module (SPI Slave)             │                    │
│  │ • Receives radio packet            │                    │
│  │ • Stores in RX buffer              │                    │
│  └────────────────────────────────────┘                    │
│      │                                                      │
│      ▼                                                      │
│  ┌────────────────────────────────────┐                    │
│  │ Read TTC Task (10 Hz)              │                    │
│  │ • Polls TTC module                 │                    │
│  │ • ttc_recv() → extract packet      │                    │
│  │ • Validate packet format           │                    │
│  └────────────────────────────────────┘                    │
│      │                                                      │
│      ▼                                                      │
│  ┌────────────────────────────────────┐                    │
│  │ Command Queue (FreeRTOS Queue)     │                    │
│  │ • FIFO buffer for telecommands     │                    │
│  └────────────────────────────────────┘                    │
│      │                                                      │
│      ▼                                                      │
│  ┌────────────────────────────────────┐                    │
│  │ Process TC Task (5 ms, Priority 4) │                    │
│  │ • Dequeue command                  │                    │
│  │ • Parse packet ID and parameters   │                    │
│  │ • Validate command authority       │                    │
│  │ • Execute command action           │                    │
│  │ • Generate feedback packet         │                    │
│  └────────────────────────────────────┘                    │
│      │                                                      │
│      ▼                                                      │
│  Command Execution                                          │
│  • Mode change (hibernation)                               │
│  • Module activation/deactivation                          │
│  • Parameter modification                                  │
│  • System reset                                            │
│  • Memory operations                                       │
│  • Data request                                            │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Telecommand Packet Format

**Uplink Packet IDs** (firmware/config/config.h:1):

| Packet ID | Command Name | Parameters |
|-----------|--------------|------------|
| 0x40 | Ping Request | None |
| 0x41 | Data Request | Data type ID |
| 0x42 | Broadcast Message | Message string |
| 0x43 | Enter Hibernation | Duration (seconds) |
| 0x44 | Leave Hibernation | None |
| 0x45 | Activate Module | Module ID |
| 0x46 | Deactivate Module | Module ID |
| 0x47 | Activate Payload | Payload ID |
| 0x48 | Deactivate Payload | Payload ID |
| 0x49 | Erase Memory | Memory type, address range |
| 0x4A | Force Reset | Reset type |
| 0x4B | Get Payload Data | Payload ID |
| 0x4C | Set Parameter | Parameter ID, value |
| 0x4D | Get Parameter | Parameter ID |

### Command Processing Flow

**Location**: firmware/app/tasks/process_tc.c:1

```
┌─────────────────────────────────────────────┐
│ Process TC Task (200 Hz, 5 ms period)       │
├─────────────────────────────────────────────┤
│                                             │
│  1. Check command queue                     │
│     └─> xQueueReceive(tc_queue, &cmd, 0)   │
│                                             │
│  2. Parse packet                            │
│     └─> Extract packet_id                  │
│     └─> Extract parameters                 │
│                                             │
│  3. Validate command                        │
│     └─> Check packet_id validity           │
│     └─> Verify parameter ranges            │
│                                             │
│  4. Execute command                         │
│     └─> switch(packet_id) {                │
│         case 0x40: handle_ping()           │
│         case 0x41: handle_data_request()   │
│         case 0x43: enter_hibernation()     │
│         case 0x45: activate_module()       │
│         ...                                │
│         }                                  │
│                                             │
│  5. Generate feedback                       │
│     └─> Create feedback packet (0x25)      │
│     └─> Include execution result           │
│     └─> ttc_send(TTC_0, feedback_pkt)      │
│                                             │
│  6. Log command execution                   │
│     └─> Update last_valid_tc               │
│     └─> sys_log(INFO, "TC executed")       │
│                                             │
└─────────────────────────────────────────────┘
```

## Transmission Path (Downlink)

### Downlink Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│ DOWNLINK PATH: OBDH → Ground Station                       │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  sat_data_buf (Global telemetry)                            │
│      │                                                      │
│      ▼                                                      │
│  ┌────────────────────────────────────┐                    │
│  │ Beacon Task (60s period, DISABLED) │                    │
│  │ OR                                 │                    │
│  │ Process TC Task (on data request)  │                    │
│  └────────────────────────────────────┘                    │
│      │                                                      │
│      ▼                                                      │
│  ┌────────────────────────────────────┐                    │
│  │ Packet Formatting (fsat_pkt)       │                    │
│  │ • Create packet header             │                    │
│  │ • Set packet ID (0x20-0x26)        │                    │
│  │ • Populate payload fields          │                    │
│  │ • Calculate checksum               │                    │
│  │ • Apply NGHam encoding             │                    │
│  └────────────────────────────────────┘                    │
│      │                                                      │
│      ▼                                                      │
│  ┌────────────────────────────────────┐                    │
│  │ TTC Device Layer                   │                    │
│  │ • ttc_send(TTC_0, packet, len)     │                    │
│  │   OR                               │                    │
│  │ • ttc_send(TTC_1, packet, len)     │                    │
│  └────────────────────────────────────┘                    │
│      │                                                      │
│      ▼                                                      │
│  ┌────────────────────────────────────┐                    │
│  │ SL_TTC2 Driver (SPI)               │                    │
│  │ • sl_ttc2_send_cmd()               │                    │
│  │ • Write packet to TX buffer        │                    │
│  │ • Trigger transmission             │                    │
│  └────────────────────────────────────┘                    │
│      │                                                      │
│      ▼                                                      │
│  TTC Module (Radio transmitter)                             │
│      │                                                      │
│      ▼                                                      │
│  Radio Transmission (UHF downlink)                          │
│      │                                                      │
│      ▼                                                      │
│  Ground Station                                             │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Downlink Packet Types

**Packet IDs** (firmware/config/config.h:1):

| Packet ID | Type | Trigger | Content |
|-----------|------|---------|---------|
| 0x20 | General Telemetry | Beacon task / Data request | Full sat_data_buf |
| 0x21 | Ping Answer | Ping command (0x40) | Acknowledgment |
| 0x22 | Data Request Answer | Data request (0x41) | Requested data subset |
| 0x23 | Message Broadcast | Broadcast command (0x42) | Message echo |
| 0x24 | Payload Data | Payload data request (0x4B) | Specific payload telemetry |
| 0x25 | TC Feedback | After command execution | Execution result |
| 0x26 | Parameter Value | Get parameter (0x4D) | Parameter value |

### Beacon Task (Periodic Telemetry Broadcast)

**Location**: firmware/app/tasks/beacon.c:1
**Status**: DISABLED by default (CONFIG_TASK_BEACON_EN = 0)
**Period**: 60 seconds (configurable)
**Priority**: 3

```
┌─────────────────────────────────────────────┐
│ Beacon Task (when enabled)                  │
├─────────────────────────────────────────────┤
│                                             │
│  Every 60 seconds:                          │
│                                             │
│  1. Create packet structure                 │
│     └─> packet_id = 0x20 (General Telem)   │
│                                             │
│  2. Populate packet payload                 │
│     └─> Copy sat_data_buf.obdh             │
│     └─> Copy sat_data_buf.eps              │
│     └─> Copy sat_data_buf.ttc_0            │
│     └─> Copy sat_data_buf.antenna          │
│     └─> Add timestamp                      │
│                                             │
│  3. Format with fsat_pkt library            │
│     └─> Add header, checksum               │
│     └─> NGHam encoding                     │
│                                             │
│  4. Transmit                                │
│     └─> ttc_send(TTC_0, beacon_pkt, len)   │
│                                             │
│  5. Log transmission                        │
│     └─> sys_log(INFO, "Beacon sent")       │
│                                             │
└─────────────────────────────────────────────┘
```

### Packet Formatting (fsat_pkt Library)

**Location**: firmware/app/libs/fsat_pkt/

**Packet Structure**:
```
┌──────────────────────────────────────────┐
│ Packet Header                            │
│ • Sync bytes (0xEB90)                    │
│ • Packet ID (1 byte)                     │
│ • Payload length (1-2 bytes)             │
│ • Sequence number (optional)             │
├──────────────────────────────────────────┤
│ Packet Payload                           │
│ • Telemetry data                         │
│ • Command parameters                     │
│ • Variable length (up to 256 bytes)      │
├──────────────────────────────────────────┤
│ Packet Footer                            │
│ • CRC16/CRC32 checksum                   │
└──────────────────────────────────────────┘
```

**NGHam Encoding** (firmware/libs/ngham/):
- Forward Error Correction (FEC)
- Reed-Solomon encoding
- Increases reliability over noisy radio channel

## Storage Path (Data Logging)

### Data Logging Flow

```
┌─────────────────────────────────────────────────────────────┐
│ STORAGE PATH: Telemetry → Non-Volatile Memory              │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  sat_data_buf (Global telemetry)                            │
│      │                                                      │
│      ▼                                                      │
│  ┌────────────────────────────────────┐                    │
│  │ Data Log Task (10 min intervals)   │                    │
│  │ • Priority: 3                      │                    │
│  │ • Period: 600 seconds              │                    │
│  └────────────────────────────────────┘                    │
│      │                                                      │
│      ▼                                                      │
│  ┌────────────────────────────────────┐                    │
│  │ Data Collection                    │                    │
│  │ • Read sat_data_buf                │                    │
│  │ • Add timestamp                    │                    │
│  │ • Format log entry                 │                    │
│  └────────────────────────────────────┘                    │
│      │                                                      │
│      ▼                                                      │
│  ┌────────────────────────────────────┐                    │
│  │ Media Device Layer                 │                    │
│  │ • media_write(MEDIA_NOR, data)     │                    │
│  │   OR                               │                    │
│  │ • media_write(MEDIA_FRAM, data)    │                    │
│  │   OR                               │                    │
│  │ • media_write(MEDIA_INT_FLASH,...)  │                    │
│  └────────────────────────────────────┘                    │
│      │                                                      │
│      ▼                                                      │
│  ┌────────────────────────────────────┐                    │
│  │ Storage Driver                     │                    │
│  │ • MT25Q driver (NOR Flash, SPI)    │                    │
│  │ • CY15x102QN driver (FRAM, I2C)    │                    │
│  │ • Internal Flash driver            │                    │
│  └────────────────────────────────────┘                    │
│      │                                                      │
│      ▼                                                      │
│  Non-Volatile Memory                                        │
│  • NOR Flash: 16/64/128/256 MB                             │
│  • FRAM: 64 KB                                             │
│  • Internal Flash: 256 KB                                  │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Data Log Task

**Location**: firmware/app/tasks/data_log.c:1
**Period**: 600 seconds (10 minutes)
**Priority**: 3
**Stack Size**: 225 bytes

```
┌─────────────────────────────────────────────┐
│ Data Log Task (0.00167 Hz, 10 min)          │
├─────────────────────────────────────────────┤
│                                             │
│  Every 10 minutes:                          │
│                                             │
│  1. Collect housekeeping data               │
│     └─> Read sat_data_buf.obdh             │
│     └─> Read sat_data_buf.eps              │
│     └─> Read sat_data_buf.ttc_0            │
│     └─> Read system_time                   │
│                                             │
│  2. Format log entry                        │
│     └─> Create timestamped record          │
│     └─> Add entry header                   │
│                                             │
│  3. Write to non-volatile memory            │
│     └─> media_write(MEDIA_NOR, entry)      │
│         └─> MT25Q NOR Flash (primary)      │
│                                             │
│  4. Verify write operation                  │
│     └─> Check return status                │
│     └─> Log errors if write failed         │
│                                             │
│  5. Update write pointer                    │
│     └─> Increment log address              │
│     └─> Handle memory wrap-around          │
│                                             │
└─────────────────────────────────────────────┘
```

### Media Device Abstraction

**Location**: firmware/devices/media/media.c:1

Provides unified interface for different storage types:

```c
// Initialize storage media
int media_init(media_type_t type);

// Write data to storage
int media_write(media_type_t type, uint32_t address,
                uint8_t *data, uint16_t len);

// Read data from storage
int media_read(media_type_t type, uint32_t address,
               uint8_t *data, uint16_t len);

// Erase storage sector/block
int media_erase(media_type_t type, uint32_t address);
```

**Media Types**:
- `MEDIA_INT_FLASH`: Internal MSP430 flash
- `MEDIA_FRAM`: External FRAM (CY15x102QN, 64 KB)
- `MEDIA_NOR`: External NOR flash (MT25Q, up to 256 MB)

### Storage Driver Layer

#### MT25Q NOR Flash Driver

**Location**: firmware/drivers/mt25q/mt25q.c:1
**Interface**: SPI
**Capacity**: 16/64/128/256 MB variants

**Operations**:
- Page Program: Write up to 256 bytes
- Sector Erase: Erase 4 KB sectors
- Block Erase: Erase 64 KB blocks
- Chip Erase: Erase entire device
- Read: Random access read

#### CY15x102QN FRAM Driver

**Location**: firmware/drivers/cy15x102qn/
**Interface**: I2C or SPI
**Capacity**: 64 KB

**Characteristics**:
- Non-volatile RAM (instant write, no erase needed)
- Unlimited write endurance
- Ideal for configuration and state data

## Data Retrieval Path

### On-Demand Data Transmission

When ground station requests data:

```
┌─────────────────────────────────────────────┐
│ Data Request Flow                           │
├─────────────────────────────────────────────┤
│                                             │
│  1. Ground sends Data Request (0x41)        │
│     └─> Specifies data type/range          │
│                                             │
│  2. Process TC Task receives command        │
│     └─> Parses data request parameters     │
│                                             │
│  3. Data retrieval                          │
│     └─> If live data: read sat_data_buf    │
│     └─> If logged data:                    │
│         └─> media_read(MEDIA_NOR, addr)    │
│         └─> Retrieve historical records    │
│                                             │
│  4. Format response packet (0x22)           │
│     └─> Populate with requested data       │
│     └─> Add timestamp and sequence         │
│                                             │
│  5. Transmit response                       │
│     └─> ttc_send(TTC_0, response_pkt)      │
│                                             │
└─────────────────────────────────────────────┘
```

## Data Flow Timing

### Update Frequencies

| Data Source | Read Task | Frequency | Priority |
|-------------|-----------|-----------|----------|
| OBDH Sensors | Read Sensors | 1 Hz | 3 |
| EPS Module | Read EPS | 1 Hz | 3 |
| TTC Module 0 | Read TTC | 10 Hz | 4 |
| TTC Module 1 | Read TTC | 10 Hz | 4 |
| Antenna | Read Antenna | 1 Hz | 2 |
| Payloads | Read EDC | 1 Hz | 3 |

### Processing Frequencies

| Process | Task | Frequency | Priority |
|---------|------|-----------|----------|
| Command Processing | Process TC | 200 Hz (5 ms) | 4 |
| Beacon Transmission | Beacon | 0.0167 Hz (60 s) | 3 |
| Data Logging | Data Log | 0.00167 Hz (10 min) | 3 |
| Time Update | Time Control | 1 Hz | 3 |
| Watchdog Refresh | Watchdog Reset | 2 Hz (500 ms) | 4 |

## Data Integrity and Reliability

### Error Detection

1. **Packet Checksums**: CRC16/CRC32 on all transmitted packets
2. **NGHam FEC**: Forward error correction on radio link
3. **Hardware CRC**: Flash memory integrity checks
4. **Parameter Validation**: Range checking on received commands

### Error Recovery

1. **Retransmission**: Ground station can request data retransmission
2. **Redundant Radios**: TTC_0 and TTC_1 provide backup communication
3. **Logged Data**: Non-volatile storage preserves data through resets
4. **Reset Recovery**: System state restored from FRAM after reset

### Data Synchronization

**Global Buffer Access**:
- Each read task updates independent sections
- No mutex required (non-overlapping writes)
- Transmission tasks perform read-only access
- Atomic updates for critical parameters

## Communication Protocols

### NGHam Protocol

**Location**: firmware/libs/ngham/

**Features**:
- Variable-length packets (up to 220 bytes payload)
- Reed-Solomon error correction
- Automatic packet framing
- Checksum validation

### libcsp (CubeSat Space Protocol)

**Version**: v1.5.16

**Features**:
- Network layer for inter-module communication
- Routing between subsystems
- Connection-oriented and connectionless modes
- Port-based addressing

## Summary

The OBDH2 data path architecture provides:

1. **Comprehensive Data Acquisition**: All subsystems polled at appropriate frequencies
2. **Centralized Data Management**: Single global buffer for telemetry
3. **Flexible Transmission**: Beacon, on-demand, and event-driven downlink
4. **Persistent Storage**: Non-volatile logging for mission data
5. **Reliable Communication**: Error correction and redundancy
6. **Responsive Command Processing**: High-priority telecommand handling

Data flows efficiently from sensors through processing to ground station and storage, ensuring mission-critical information is captured, transmitted, and preserved.
