# OBDH2 System Functionalities

## Overview

The OBDH2 (On-Board Data Handling 2.0) is the central on-board computer for the FloripaSat-2 CubeSat mission. It serves as the primary data processing and coordination subsystem, managing all satellite operations, telemetry collection, command processing, and inter-module communication.

**Firmware Version**: 0.10.0 (Development)
**Hardware Platform**: MSP430F6659 Microcontroller
**Operating System**: FreeRTOS v10.2.1
**License**: GPLv3 (Firmware), CERN OHL v2 (Hardware), CC BY-SA 4.0 (Documentation)

## Primary System Functionalities

### 1. Module Synchronization and Coordination

The OBDH2 acts as the central coordinator for all satellite subsystems:

- **EPS (Electrical Power System)**: Monitors and manages power distribution
- **TTC (Telemetry, Tracking & Command)**: Coordinates radio communication
- **Antenna Subsystem**: Controls deployment sequences
- **Payloads**: Manages scientific payload operations (EDC-1, EDC-2, Payload-X, HARSH)

The system ensures synchronized operation between all modules, preventing conflicts and optimizing resource usage.

### 2. Data Acquisition and Processing

The OBDH2 continuously collects telemetry data from all satellite subsystems:

**Housekeeping Data**:
- Temperature measurements from multiple sensors
- Voltage levels across power systems
- Current consumption monitoring
- System health indicators

**Telemetry Sources**:
- OBDH internal sensors (temperature, voltage, current)
- EPS telemetry (battery status, solar panel outputs, power distribution)
- TTC telemetry (radio status, RSSI, temperature)
- Antenna telemetry (deployment status, temperature)
- Payload data (up to 220 bytes per payload)

**Data Structure**: firmware/app/structs/satellite.h:1

All collected data is stored in a centralized buffer (`sat_data_buf`) for processing and transmission.

### 3. Communication Management

The OBDH2 manages all uplink and downlink communications:

**Uplink (Ground to Satellite)**:
- Receives telecommands via TTC modules
- Validates and parses command packets
- Executes authorized commands
- Sends acknowledgment/feedback packets

**Downlink (Satellite to Ground)**:
- Transmits telemetry data (beacon packets)
- Responds to data requests
- Broadcasts status messages
- Sends payload data

**Communication Protocols**:
- **NGHam**: Network protocol for packet formatting
- **libcsp v1.5.16**: Communication System Protocol
- **fsat_pkt**: FloripaSat packet library

### 4. Data Logging and Storage

The OBDH2 implements a robust data logging system for mission-critical information:

**Storage Media**:
- **Internal Flash**: Configuration and program storage
- **FRAM (CY15x102QN)**: 64 KB for state persistence
- **NOR Flash (MT25Q)**: 16/64/128/256 MB for data logging

**Logging Features**:
- Periodic housekeeping data archival (default: 10-minute intervals)
- Configurable log intervals
- Non-volatile storage ensures data survival during power cycles
- Data retrieval on command from ground station

**Implementation**: firmware/app/tasks/data_log.c:1

### 5. Telecommand Processing

The OBDH2 processes various telecommands received from the ground station:

**Supported Commands** (firmware/config/config.h:1):

| Packet ID | Command | Description |
|-----------|---------|-------------|
| 0x40 | Ping Request | System availability check |
| 0x41 | Data Request | Request specific telemetry data |
| 0x42 | Broadcast Message | Send message to satellite |
| 0x43 | Enter Hibernation | Activate low-power mode |
| 0x44 | Leave Hibernation | Exit low-power mode |
| 0x45 | Activate Module | Power on specific module |
| 0x46 | Deactivate Module | Power off specific module |
| 0x47 | Activate Payload | Enable payload operation |
| 0x48 | Deactivate Payload | Disable payload operation |
| 0x49 | Erase Memory | Clear memory sections |
| 0x4A | Force Reset | Trigger system reset |
| 0x4B | Get Payload Data | Retrieve payload information |
| 0x4C | Set Parameter | Configure system parameters |
| 0x4D | Get Parameter | Read system parameters |

**Command Processing**: firmware/app/tasks/process_tc.c:1

The Process TC (Telecommand) task runs at high priority (4) with a 5 ms period to ensure responsive command handling.

### 6. Antenna Deployment Control

The OBDH2 manages the critical antenna deployment sequence:

**Deployment Configuration**:
- **Independent Deployment**: 10 seconds burn time per antenna
- **Sequential Deployment**: 20 seconds total burn time
- **Maximum Attempts**: 10 deployment cycles
- **Hibernation Period**: 45 minutes after failed deployment

**Safety Features**:
- Status monitoring throughout deployment
- Temperature monitoring to prevent overheating
- Automatic retry mechanism
- Deployment confirmation verification

**Implementation**:
- firmware/app/tasks/antenna_deployment.c:1
- firmware/devices/antenna/antenna.c:1

### 7. Operational Mode Management

The OBDH2 supports multiple operational modes to optimize power consumption and mission objectives:

**Normal Mode**:
- Full operational functionality
- All subsystems active
- Regular telemetry collection and transmission
- Continuous command monitoring

**Hibernation Mode**:
- Low-power state
- Configurable hibernation duration
- Critical systems maintained
- Automatic wake-up after timeout

**Mode Transitions**:
- Command-triggered mode changes
- Automatic mode switching based on system conditions
- State preservation during transitions

**Implementation**: firmware/app/tasks/housekeeping.c:1

### 8. Time Management

Accurate timekeeping is essential for satellite operations:

**Features**:
- System time counter (incrementing every second)
- Timestamp generation for all telemetry packets
- Event scheduling and timing
- Log entry time-stamping

**Clock Sources**:
- Main Clock (MCLK): 32 MHz
- Auxiliary Clock (ACLK): 32.768 kHz (RTC source)

**Implementation**: firmware/app/tasks/time_control.c:1

### 9. Watchdog Protection and Fault Recovery

The OBDH2 implements comprehensive watchdog mechanisms:

**Hardware Watchdog** (TPS382x):
- External watchdog IC for independent monitoring
- ~500 ms timeout period
- Automatic system reset on timeout
- Periodic refresh by dedicated task

**Software Features**:
- Reset cause detection and logging
- Reset counter tracking
- System state recovery after reset
- Diagnostic information preservation

**Implementation**:
- firmware/app/tasks/watchdog_reset.c:1
- firmware/drivers/tps382x/tps382x.c:1
- firmware/system/system.c:1 (reset cause detection)

### 10. Health Monitoring and Status Reporting

Continuous system health monitoring ensures mission reliability:

**Monitored Parameters**:
- MCU temperature
- Input voltage levels
- Current consumption
- Module availability
- Communication link quality (RSSI)
- Memory usage and availability

**Status Indicators**:
- Heartbeat LED (1-second toggle)
- Status reporting in telemetry packets
- Error logging via UART
- Event tracking

**Implementation**: firmware/app/tasks/heartbeat.c:1

## Data Types and Formats

### Downlink Packet Types

| Packet ID | Type | Description |
|-----------|------|-------------|
| 0x20 | General Telemetry | Comprehensive system status |
| 0x21 | Ping Answer | Response to ping request |
| 0x22 | Data Request Answer | Response to data request |
| 0x23 | Message Broadcast | General message downlink |
| 0x24 | Payload Data | Scientific payload information |
| 0x25 | TC Feedback | Telecommand execution result |
| 0x26 | Parameter Value | Parameter query response |

### Telemetry Data Structure

**OBDH Telemetry** (firmware/app/structs/obdh_data.h:1):
- Temperature (Kelvin)
- Current (milliamperes)
- Voltage (millivolts)
- Reset cause and counter
- Last valid telecommand ID
- Radio temperature and RSSI
- Antenna temperature and status
- Hardware/Firmware version information
- Current operational mode

**Satellite Data Buffer** (firmware/app/structs/satellite.h:1):
```c
typedef struct {
    obdh_telemetry_t obdh;        // OBDH housekeeping
    eps_telemetry_t eps;           // Power subsystem data
    ttc_telemetry_t ttc_0;         // Radio 0 telemetry
    ttc_telemetry_t ttc_1;         // Radio 1 telemetry
    antenna_telemetry_t antenna;   // Antenna status
    payload_telemetry_t edc_0;     // Payload EDC-1 (220 bytes)
    payload_telemetry_t edc_1;     // Payload EDC-2 (220 bytes)
    payload_telemetry_t payload_x; // Payload-X (220 bytes)
    payload_telemetry_t harsh;     // HARSH payload (220 bytes)
} sat_data_t;
```

## System Capabilities Summary

| Capability | Specification |
|------------|---------------|
| **Processing** | MSP430F6659 @ 32 MHz |
| **Memory (RAM)** | 16 KB |
| **Flash Memory** | 256 KB |
| **Non-Volatile Storage** | 64 KB FRAM + up to 256 MB NOR Flash |
| **Communication Interfaces** | UART (3), SPI (6), I2C (3) |
| **Radio Modules** | Dual TTC (redundancy) |
| **Payload Support** | Up to 4 payloads (220 bytes each) |
| **Task Management** | FreeRTOS (15+ concurrent tasks) |
| **Total Heap** | 40 KB |
| **Max Task Priority Levels** | 5 |
| **Watchdog Period** | 500 ms |
| **Data Log Interval** | Configurable (default: 10 minutes) |
| **Beacon Interval** | Configurable (default: 60 seconds, disabled) |

## Configuration and Feature Control

System features can be enabled/disabled via compile-time flags:

**Configuration File**: firmware/config/config.h:1

**Configurable Features**:
- Task enable/disable flags
- Communication protocol selection
- Memory subsystem selection
- Debug logging levels
- Operational parameters (timeouts, intervals)
- Hardware version compatibility

## System Initialization Sequence

1. **Hardware Initialization**
   - Clock configuration (32 MHz MCLK, 32.768 kHz ACLK)
   - GPIO setup and hardware version detection
   - Peripheral initialization (UART, SPI, I2C)

2. **Device Initialization**
   - Media subsystem (Flash, FRAM)
   - Sensors (temperature, voltage, current)
   - Communication modules (TTC, EPS, Antenna)
   - Watchdog timer setup

3. **FreeRTOS Initialization**
   - Task creation and registration
   - Event group setup
   - Queue initialization

4. **Startup Task Execution**
   - System verification
   - Reset cause analysis
   - Device availability checks
   - Startup completion signaling

5. **Normal Operations**
   - Task scheduler activation
   - Periodic telemetry collection
   - Command monitoring
   - Continuous health monitoring

**Implementation**: firmware/main.c:1, firmware/app/tasks/startup.c:1

## Mission Support

The OBDH2 is designed to support various mission phases:

- **Launch and Early Orbit**: Antenna deployment, initial system checkout
- **Commissioning**: System testing, parameter tuning, ground link establishment
- **Normal Operations**: Routine telemetry collection, payload operations
- **Hibernation**: Extended low-power periods for power conservation
- **Contingency**: Fault recovery, safe mode operations

## References

- Main firmware entry point: firmware/main.c:1
- System configuration: firmware/config/config.h:1
- Task implementations: firmware/app/tasks/
- Data structures: firmware/app/structs/
- Device abstractions: firmware/devices/
- Hardware drivers: firmware/drivers/
- Version information: firmware/version.h:1
