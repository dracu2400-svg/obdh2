# OBDH 2.0 Data Path and Data Structures

## Overview

This document describes the data flow architecture, data structures, and information pathways within the OBDH 2.0 firmware. Understanding the data path is critical for comprehending how telemetry is collected, processed, stored, and transmitted.

---

## Central Data Buffer Architecture

### Global Data Repository

**Primary Structure:** `sat_data_t sat_data_buf`

The entire satellite state is maintained in a single global structure defined in `app/structs/satellite.h`. This central repository serves as the single source of truth for all telemetry data.

```c
typedef struct {
    obdh_telemetry_t    obdh;          // OBDH subsystem telemetry
    eps_telemetry_t     eps;           // Electrical Power System
    ttc_telemetry_t     ttc[2];        // TTC 0 and TTC 1 (redundant)
    antenna_telemetry_t antenna;       // ISIS Antenna status
    payload_telemetry_t payload[4];    // Up to 4 payloads
} sat_data_t;

// Global instance
sat_data_t sat_data_buf;
```

---

## Data Structure Definitions

### 1. OBDH Telemetry

**Structure:** `obdh_telemetry_t` (defined in `app/structs/obdh_data.h`)

```c
typedef struct {
    sys_time_t timestamp;      // System time when data was collected
    obdh_data_t data;          // OBDH-specific data
} obdh_telemetry_t;

typedef struct {
    uint32_t time_counter;              // System uptime in milliseconds
    uint16_t temperature_uc;            // Microcontroller temp (Kelvin * 10)
    uint16_t input_current;             // OBDH input current (mA)
    uint16_t input_voltage;             // OBDH input voltage (mV)
    uint16_t reset_counter;             // Number of system resets
    uint8_t  last_reset_cause;          // Cause of last reset
    uint8_t  last_valid_tc;             // Last valid telecommand ID
    int8_t   rssi_last_tc;              // RSSI of last telecommand (dBm)
    uint8_t  mode;                      // 0=Normal, 1=Hibernation
    uint8_t  antenna_deployment_status; // Antenna deployment flags
} obdh_data_t;
```

**Data Sources:**
- `time_counter`: System timer (RTC + millisecond counter)
- `temperature_uc`: Internal ADC reading from MSP430
- `input_current`: External current sensor via ADC
- `input_voltage`: Voltage divider via ADC
- `reset_counter`: Stored in FRAM, incremented on each boot
- `last_reset_cause`: MSP430 reset status register
- `last_valid_tc`: Updated by Process TC task
- `rssi_last_tc`: From TTC module when command received
- `mode`: Set by hibernation control logic
- `antenna_deployment_status`: From Antenna Deploy task

**Update Frequency:** 10 Hz (Read Sensors Task)

---

### 2. EPS Telemetry

**Structure:** `eps_telemetry_t`

```c
typedef struct {
    sys_time_t timestamp;      // Collection timestamp
    eps_data_t data;           // EPS data from sl_eps2 module
} eps_telemetry_t;

// eps_data_t is defined by sl_eps2 driver
typedef struct {
    // Battery
    uint16_t battery_voltage;           // mV
    int16_t  battery_current;           // mA (charge positive)
    uint16_t battery_charge;            // mAh
    int16_t  battery_temperature;       // Celsius * 10

    // Solar Panels (multiple panels)
    uint16_t solar_panel_voltage[6];    // mV per panel
    uint16_t solar_panel_current[6];    // mA per panel

    // Power Buses
    uint16_t bus_3v3_voltage;           // mV
    uint16_t bus_5v0_voltage;           // mV
    uint16_t bus_12v_voltage;           // mV

    // Loads
    uint16_t payload_current[4];        // mA per payload

    // Heater
    uint8_t  heater_status;             // On/Off

    // Misc
    uint8_t  eps_mode;                  // EPS operating mode
    // ... (additional fields from sl_eps2)
} eps_data_t;
```

**Data Source:** SpaceLab EPS v2 module via I2C

**Update Frequency:** 60 seconds (Read EPS Task)

---

### 3. TTC Telemetry

**Structure:** `ttc_telemetry_t` (array of 2 for redundancy)

```c
typedef struct {
    sys_time_t timestamp;      // Collection timestamp
    ttc_data_t data;           // TTC data from sl_ttc2 module
} ttc_telemetry_t;

// ttc_data_t is defined by sl_ttc2 driver
typedef struct {
    // Radio Status
    uint8_t  radio_status;              // Radio state (idle, TX, RX, sleep)
    int8_t   last_rssi;                 // RSSI of last received packet (dBm)

    // Temperature
    int16_t  radio_temperature;         // Celsius * 10

    // Transmission
    uint8_t  tx_power;                  // Transmission power level (dBm)
    uint32_t tx_packet_count;           // Total packets transmitted

    // Reception
    uint32_t rx_packet_count;           // Total packets received
    uint32_t rx_packet_error_count;     // Corrupted packets

    // Configuration
    uint32_t frequency;                 // Operating frequency (Hz)
    uint16_t bitrate;                   // Data rate (bps)

    // Misc
    uint8_t  antenna_status;            // Antenna switch position
    // ... (additional fields from sl_ttc2)
} ttc_data_t;
```

**Data Source:** SpaceLab TTC v2 modules (2x) via I2C

**Update Frequency:** 60 seconds (Read TTC Task)

---

### 4. Antenna Telemetry

**Structure:** `antenna_telemetry_t`

```c
typedef struct {
    sys_time_t timestamp;           // Collection timestamp
    antenna_data_t data;            // Antenna data
} antenna_telemetry_t;

typedef struct {
    uint8_t  deployment_status[4];  // Status for each of 4 antennas
    uint16_t temperature;           // Antenna module temp (Kelvin * 10)
    uint16_t deployment_time[4];    // Burn time for each antenna (seconds)
    uint8_t  deployment_attempts;   // Total deployment attempts
} antenna_data_t;
```

**Data Source:** ISIS Antenna Deployer via I2C/SPI

**Update Frequency:** On-demand (Read Antenna Task, triggered by telecommand)

---

### 5. Payload Telemetry

**Structure:** `payload_telemetry_t` (array of 4)

```c
typedef struct {
    sys_time_t timestamp;       // Collection timestamp
    uint8_t    enabled;         // 0=Disabled, 1=Enabled
    uint8_t    data[220];       // Payload-specific data (max packet size)
} payload_telemetry_t;

// Payload IDs
#define PAYLOAD_EDC_1           0
#define PAYLOAD_EDC_2           1
#define PAYLOAD_X               2
#define PAYLOAD_HARSH           3  // Radiation monitor
```

**Data Sources:**
- **EDC (Earth Data Collection):** UART interface to EDC module
- **Payload_X:** Reserved for future payloads
- **HARSH:** Radiation monitoring instrument

**Update Frequency:** Variable (payload-dependent, typically on-demand)

**Data Format:** Payload-specific binary data up to 220 bytes

---

## Data Flow Diagrams

### Uplink Data Flow (Ground to Satellite)

```
Ground Station
    ↓ RF Transmission
TTC Module (Hardware)
    ↓ I2C
Read TTC Task (60s periodic)
    ↓ Check for new packets
┌──────────────────────────────┐
│  Packet Available?           │
│  Yes ↓          No → Return  │
└──────────────────────────────┘
    ↓
Retrieve packet via I2C
    ↓
┌──────────────────────────────┐
│  Process TC Task             │
│  (Triggered by queue/event)  │
└──────────────────────────────┘
    ↓
Parse FloripaSat Packet (fsat_pkt library)
    ↓
Verify HMAC-SHA (hmac library)
    ↓ Valid
Decode Telecommand Type
    ↓
┌──────────────────────────────────────────────────┐
│  Command Router                                  │
│  ┌──────────────────────────────────────────┐   │
│  │ - Ping → Respond via Beacon Task         │   │
│  │ - Data Request → Queue data retrieval    │   │
│  │ - Hibernation → Set mode flag            │   │
│  │ - Payload Control → Enable/Disable       │   │
│  │ - Parameter Write → Update FRAM/Flash    │   │
│  │ - Reset → Trigger System Reset Task      │   │
│  └──────────────────────────────────────────┘   │
└──────────────────────────────────────────────────┘
    ↓
Execute Command (modify sat_data_buf or trigger action)
    ↓
Generate Feedback Packet
    ↓
Queue for Downlink (Beacon Task)
```

---

### Downlink Data Flow (Satellite to Ground)

```
Periodic Data Collection Tasks
    ↓
┌────────────────────────────────────────────────┐
│  Read Sensors Task (10 Hz)                     │
│  - Temperature, Current, Voltage               │
│  - Update sat_data_buf.obdh                    │
└────────────────────────────────────────────────┘
    ↓
┌────────────────────────────────────────────────┐
│  Read EPS Task (60s)                           │
│  - Request telemetry via I2C                   │
│  - Update sat_data_buf.eps                     │
└────────────────────────────────────────────────┘
    ↓
┌────────────────────────────────────────────────┐
│  Read TTC Task (60s)                           │
│  - Request telemetry via I2C (both modules)    │
│  - Update sat_data_buf.ttc[0], ttc[1]          │
└────────────────────────────────────────────────┘
    ↓
┌────────────────────────────────────────────────┐
│  Read Antenna Task (On-demand)                 │
│  - Request status via I2C/SPI                  │
│  - Update sat_data_buf.antenna                 │
└────────────────────────────────────────────────┘
    ↓
┌────────────────────────────────────────────────┐
│  Read EDC Task (On-demand)                     │
│  - Request data via UART                       │
│  - Update sat_data_buf.payload[0], [1]         │
└────────────────────────────────────────────────┘
    ↓
┌────────────────────────────────────────────────┐
│         sat_data_buf (Central Repository)      │
│  - Complete satellite state                    │
│  - Protected by mutex                          │
└────────────────────────────────────────────────┘
    ↓ Read
┌────────────────────────────────────────────────┐
│  Beacon Task (60s)                             │
│  - Acquire mutex                               │
│  - Read sat_data_buf                           │
│  - Format beacon packet (fsat_pkt)             │
│  - Calculate HMAC (hmac)                       │
└────────────────────────────────────────────────┘
    ↓
TTC Device Interface
    ↓ I2C
TTC Module (Hardware)
    ↓ RF Transmission
Ground Station
```

---

### Data Logging Flow (Persistent Storage)

```
┌────────────────────────────────────────────────┐
│  Data Log Task (5 min period)                  │
│  - Priority 3                                  │
│  - Stack 225 bytes                             │
└────────────────────────────────────────────────┘
    ↓
Acquire mutex on sat_data_buf
    ↓
Create snapshot of satellite state
    ↓
Serialize data (packet format)
    ↓
┌────────────────────────────────────────────────┐
│  Storage Selection                             │
│  ┌──────────────────────────────────────────┐ │
│  │ - FRAM: High-priority, recent data       │ │
│  │ - NOR Flash: Long-term archival          │ │
│  │ - Internal Flash: Critical parameters    │ │
│  └──────────────────────────────────────────┘ │
└────────────────────────────────────────────────┘
    ↓
Media Device Interface (devices/media/)
    ↓
┌────────────────────────────────────────────────┐
│  Storage Drivers                               │
│  ┌──────────────────────────────────────────┐ │
│  │ - FRAM Driver (cy15x102qn)               │ │
│  │   - SPI interface                        │ │
│  │   - Fast write (no erase)                │ │
│  │   - 256 KB capacity                      │ │
│  │                                          │ │
│  │ - NOR Flash Driver (mt25q)               │ │
│  │   - SPI interface                        │ │
│  │   - Page program + sector erase          │ │
│  │   - 256 MB capacity                      │ │
│  │                                          │ │
│  │ - Internal Flash Driver                  │ │
│  │   - MSP430 flash controller              │ │
│  │   - Segment erase + write                │ │
│  │   - 128 KB (code) + 512 KB (extended)    │ │
│  └──────────────────────────────────────────┘ │
└────────────────────────────────────────────────┘
    ↓
Physical Storage (Non-Volatile)
    ↓
┌────────────────────────────────────────────────┐
│  Data Retrieval (on telecommand)               │
│  - Process TC Task receives data request       │
│  - Read from storage media                     │
│  - Package into downlink packets               │
│  - Transmit via Beacon Task                    │
└────────────────────────────────────────────────┘
```

---

## Packet Format Specifications

### FloripaSat Packet Structure (fsat_pkt)

**Maximum Packet Size:** 255 bytes total (220 bytes payload)

```
┌──────────┬──────────┬──────────┬──────────────┬─────────┐
│  Header  │ Packet ID│  Length  │   Payload    │   CRC   │
│  (SOD)   │ (1 byte) │ (1 byte) │ (0-220 bytes)│(2 bytes)│
├──────────┼──────────┼──────────┼──────────────┼─────────┤
│  0x7E    │   PID    │    N     │    Data      │  CRC16  │
└──────────┴──────────┴──────────┴──────────────┴─────────┘
  1 byte     1 byte     1 byte      N bytes       2 bytes

Total: 5 + N bytes (N ≤ 220)
```

**Fields:**
- **Header (SOD - Start of Data):** 0x7E (fixed delimiter)
- **Packet ID:** Identifies packet type (uplink/downlink category)
- **Length:** Payload length in bytes
- **Payload:** Variable-length data (depends on packet type)
- **CRC:** CRC-16-CCITT checksum for error detection

---

### Downlink Packet IDs

| Packet ID | Name | Description | Payload Contents |
|-----------|------|-------------|------------------|
| 0x20 | Telemetry | Housekeeping telemetry | OBDH, EPS, TTC status |
| 0x21 | Ping Reply | Response to ping command | Echo data |
| 0x22 | Data Reply | Response to data request | Requested telemetry |
| 0x23 | Message | Broadcast message | Text or binary message |
| 0x24 | Payload Data | Scientific payload data | Payload-specific data |
| 0x25 | TC Feedback | Telecommand execution result | Command ID, status |
| 0x26 | Parameters | System parameters | Configuration values |

---

### Uplink Packet IDs

| Packet ID | Name | Description | Payload Contents |
|-----------|------|-------------|------------------|
| 0x40 | Ping | Test communication | Echo data |
| 0x41 | Data Request | Request telemetry download | Data type specifier |
| 0x42 | Broadcast | Send message to satellite | Text or binary message |
| 0x43 | Enter Hibernation | Force hibernation mode | Duration (optional) |
| 0x44 | Leave Hibernation | Exit hibernation mode | - |
| 0x45 | Payload Enable | Enable payload | Payload ID |
| 0x46 | Payload Disable | Disable payload | Payload ID |
| 0x47 | Payload Request | Request payload data | Payload ID |
| 0x48 | Read Parameter | Read system parameter | Parameter ID |
| 0x49 | Write Parameter | Write system parameter | Parameter ID + value |
| 0x4A | Reset | Force system reset | Reset type |
| 0x4B | Set Time | Synchronize system time | Unix timestamp |
| 0x4C | Erase Memory | Erase storage | Memory type + address |
| 0x4D | Request Stored Data | Download logged data | Time range |

---

### Telemetry Packet Payload (0x20)

**Format:** Binary packed structure

```
Offset  Size  Field
──────  ────  ─────────────────────────────────────────
0       4     Time counter (ms)
4       2     µC temperature (K * 10)
6       2     Input current (mA)
8       2     Input voltage (mV)
10      2     Reset counter
12      1     Last reset cause
13      1     Last valid TC
14      1     RSSI last TC (signed)
15      1     Mode (0=Normal, 1=Hibernation)
16      1     Antenna deployment status
17      2     EPS: Battery voltage (mV)
19      2     EPS: Battery current (mA, signed)
21      2     EPS: Battery charge (mAh)
23      2     EPS: Battery temperature (°C * 10)
...     ...   (Additional EPS, TTC fields)
```

**Total Size:** ~100-150 bytes (varies based on included data)

---

### Payload Data Packet (0x24)

**Format:**

```
Offset  Size  Field
──────  ────  ─────────────────────────────────────────
0       1     Payload ID (0-3)
1       1     Data length (N)
2       N     Payload-specific data (max 218 bytes)
```

**Example - EDC Payload:**
- Payload ID: 0 (EDC_1) or 1 (EDC_2)
- Data: Raw EDC sensor readings, images, or processed data

---

## Data Synchronization and Concurrency

### Mutual Exclusion (Mutex)

**Problem:** Multiple tasks read/write `sat_data_buf` concurrently

**Solution:** FreeRTOS mutex protection

```c
// Global mutex (created in main or startup task)
SemaphoreHandle_t sat_data_mutex;

// Task writing to buffer
void read_sensors_task(void *pvParameters) {
    while (1) {
        // Acquire mutex (wait indefinitely)
        xSemaphoreTake(sat_data_mutex, portMAX_DELAY);

        // Critical section - update data
        sat_data_buf.obdh.data.temperature_uc = read_temperature();
        sat_data_buf.obdh.data.input_current = read_current();
        // ...

        // Release mutex
        xSemaphoreGive(sat_data_mutex);

        // Sleep until next period
        vTaskDelay(pdMS_TO_TICKS(100));  // 10 Hz
    }
}

// Task reading from buffer
void beacon_task(void *pvParameters) {
    while (1) {
        // Acquire mutex
        xSemaphoreTake(sat_data_mutex, portMAX_DELAY);

        // Critical section - read data
        memcpy(&local_copy, &sat_data_buf, sizeof(sat_data_t));

        // Release mutex (minimize hold time)
        xSemaphoreGive(sat_data_mutex);

        // Process and transmit beacon (outside critical section)
        format_beacon_packet(&local_copy);
        ttc_transmit(beacon_packet);

        // Sleep until next period
        vTaskDelay(pdMS_TO_TICKS(60000));  // 60 seconds
    }
}
```

**Best Practices:**
- Hold mutex for minimal time
- Copy data out of buffer quickly
- Perform processing outside critical section
- Use `portMAX_DELAY` for indefinite wait (guaranteed acquisition)

---

### Event-Driven Processing

**Use Case:** Telecommand processing triggered by packet arrival

```c
// Event group for task synchronization
EventGroupHandle_t tc_event_group;

#define TC_AVAILABLE_BIT  (1 << 0)

// Read TTC Task (producer)
void read_ttc_task(void *pvParameters) {
    while (1) {
        if (ttc_packet_available()) {
            // Signal that TC is available
            xEventGroupSetBits(tc_event_group, TC_AVAILABLE_BIT);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Process TC Task (consumer)
void process_tc_task(void *pvParameters) {
    while (1) {
        // Wait for TC available event
        xEventGroupWaitBits(tc_event_group,
                            TC_AVAILABLE_BIT,
                            pdTRUE,   // Clear bit on exit
                            pdFALSE,  // Wait for any bit
                            portMAX_DELAY);

        // Process telecommand
        process_uplink_command();
    }
}
```

---

## Data Integrity and Error Handling

### CRC Validation

**FloripaSat Packet CRC:**
- Algorithm: CRC-16-CCITT (polynomial 0x1021)
- Computed over: Packet ID + Length + Payload
- Stored in: Last 2 bytes of packet

**Validation Process:**
1. Receive packet from TTC
2. Extract CRC from last 2 bytes
3. Compute CRC over ID + Length + Payload
4. Compare computed vs. received CRC
5. If match: Process packet; If mismatch: Discard packet

### HMAC Authentication

**Security for Telecommands:**
- Algorithm: HMAC-SHA256
- Shared secret key stored in `config/keys.h`
- Prevents unauthorized commands

**Authentication Process:**
1. Receive telecommand packet
2. Extract HMAC from payload
3. Compute HMAC using shared key
4. Compare computed vs. received HMAC
5. If match: Execute command; If mismatch: Reject command

### Storage Error Handling

**FRAM (cy15x102qn):**
- No wear-out (unlimited writes)
- Error detection via SPI read-back verification
- Write → Read-back → Compare

**NOR Flash (mt25q):**
- Sector erase before write
- Program verification via read-back
- Bad block management (if needed)
- Wear leveling (circular buffer for logs)

**Internal Flash:**
- Segment erase protection (code segments locked)
- Write verification
- Info memory for critical parameters

---

## Data Persistence Strategy

### Bootstrap Configuration

**Stored in:** Internal Flash Info Memory (INFOA-INFOD)

**Contents:**
- Reset counter
- Last reset cause
- Cryptographic keys
- Radio configuration
- Antenna deployment history

**Access:** Read on boot, write on configuration change

---

### Recent Telemetry Cache

**Stored in:** FRAM (256 KB)

**Contents:**
- Last N telemetry snapshots (circular buffer)
- Recent telecommand history
- System event log

**Write Frequency:** 5 minutes (Data Log Task)

**Retention:** Persists across resets, power cycles

---

### Long-Term Archive

**Stored in:** NOR Flash (256 MB)

**Contents:**
- Historical telemetry (days to weeks)
- Payload data archives
- Mission logs

**Write Frequency:** 5-15 minutes (configurable)

**Retrieval:** On-demand via data request telecommand

---

## Data Downlink Prioritization

### Beacon Priority

**High Priority (every 60 seconds):**
- OBDH status (uptime, temperature, voltage)
- EPS battery status
- TTC radio status
- Operational mode

**Low Priority (on request):**
- Detailed EPS telemetry
- Antenna status
- Payload data
- Historical logs

### Downlink Scheduling

**Real-Time Beacon:** Immediate transmission (60s period)

**Requested Data:** Queued for next available TTC slot

**Payload Data:** Transmitted when ground station requests

**Stored Data:** Background downlink during communication windows

---

## Related Documentation

- `00_functionalities_of_system.md` - System overview
- `01_firmware_architecture.md` - Software architecture
- `03_interfaces.md` - Communication interfaces
- `04_tasks.md` - Task descriptions
- `05_pin_mapping.md` - Hardware pinout
- `06_hardware_components_and_architecture.md` - Hardware details
