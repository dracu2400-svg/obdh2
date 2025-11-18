# OBDH 2.0 System Functionalities

## Overview

OBDH 2.0 (On-Board Data Handling 2.0) is the central on-board computer for the FloripaSat-2 CubeSat mission, developed by SpaceLab at the Federal University of Santa Catarina (UFSC). It serves as the primary data management and control system for the satellite.

**Version:** 0.10.0 (Development)
**License:** GPLv3 (Firmware), CERN OHL v2 (Hardware)
**Target Platform:** Texas Instruments MSP430F6659 microcontroller
**Operating System:** FreeRTOS v10.2.1

---

## Core Functionalities

### 1. Subsystem Coordination and Synchronization

The OBDH acts as the central hub for coordinating all satellite subsystems:

- **Electrical Power System (EPS):** Monitors power generation, battery status, and power distribution
- **Telemetry and Telecommand (TTC):** Manages dual redundant radio transceivers (TTC_0 and TTC_1)
- **Payloads:** Controls and collects data from scientific payloads
  - EDC (Earth Data Collection) modules
  - Radiation monitor (HARSH)
  - Additional payload interfaces (Payload_X)
- **Antenna Deployment System:** Manages ISIS antenna deployer

**Key Features:**
- Centralized data collection from all subsystems
- Task-based periodic polling of subsystem status
- Event-driven command execution
- Synchronized timing across all modules

---

### 2. Communication Management

#### Uplink (Ground to Satellite)

**Telecommand Processing:**
- Receives and validates uplink packets from ground station
- Authenticates commands using HMAC-SHA cryptography
- Routes commands to appropriate subsystems
- Provides command execution feedback

**Supported Telecommands:**
- Ping/Echo response
- Data request (telemetry download)
- Broadcast messages
- Hibernation mode control
- Payload enable/disable
- Parameter read/write operations
- Time synchronization
- Force system reset
- Erase non-volatile memory
- Enter/exit hibernation mode
- Request stored telemetry packets

#### Downlink (Satellite to Ground)

**Data Transmission:**
- Packages telemetry data into FloripaSat Protocol packets
- Transmits periodic status beacons (60-second interval)
- Sends requested telemetry on demand
- Provides telecommand execution feedback
- Forwards payload scientific data

**Downlink Packet Types:**
- System telemetry (OBDH, EPS, TTC status)
- Ping responses
- Stored data packets
- Broadcast messages
- Payload data frames
- Telecommand feedback
- System parameters

**Beacon Transmission:**
- Periodic status beacon every 60 seconds
- Contains critical satellite health information
- Callsign: "PY0EFS"
- Transmitted via both TTC modules (redundancy)

---

### 3. Telemetry Data Collection

The OBDH continuously collects and organizes telemetry from multiple sources:

#### OBDH Internal Telemetry
- **Timing:** System uptime counter (milliseconds)
- **Thermal:** Microcontroller temperature
- **Power:** Input current and voltage measurements
- **Status:** Reset counter, last reset cause
- **Command History:** Last valid telecommand ID and RSSI
- **Mode:** Current operational mode (Normal/Hibernation)
- **Antenna:** Deployment status

**Collection Period:** 100 ms (sensor task at 10 Hz)

#### Subsystem Telemetry

**EPS (Electrical Power System):**
- Battery voltage, current, charge
- Solar panel voltages and currents
- Power bus voltages
- Temperature sensors
- **Collection Period:** 60 seconds

**TTC (Telemetry/Telecommand - Dual Redundant):**
- Radio transceiver status
- Transmission power levels
- Receiver signal strength (RSSI)
- Temperature
- **Collection Period:** 60 seconds per module

**Antenna (ISIS Deployer):**
- Deployment status for each antenna
- Temperature
- Burn activation history
- **Collection Period:** On-demand

**Payloads (Up to 4 supported):**
- EDC_1, EDC_2: Earth observation data
- Payload_X: Generic payload interface
- HARSH: Radiation monitoring data
- **Collection Period:** On-demand or payload-specific
- **Maximum Payload Data:** 220 bytes per packet

---

### 4. Data Storage and Logging

The OBDH manages three levels of data storage:

#### Internal Flash (128 KB)
- Firmware code storage
- System configuration and constants
- Boot parameters

#### FRAM (256 KB - CY15x102QN)
- **Technology:** Ferroelectric RAM (non-volatile, fast write)
- **Use Cases:**
  - Critical telemetry buffers
  - System state preservation across resets
  - High-frequency data logging
  - Configuration parameters
- **Advantages:**
  - Unlimited write endurance
  - Fast write speeds
  - Low power consumption

#### NOR Flash (256 MB - MT25Q)
- **Technology:** Serial NOR Flash via SPI
- **Use Cases:**
  - Long-term telemetry storage
  - Payload data archiving
  - Mission data logs
  - Event history
- **Logging Task Period:** 5 minutes

**Data Log Functionality:**
- Periodic storage of satellite state
- Circular buffer management
- Data retrieval via telecommand
- Support for data request packets

---

### 5. Time Management

**System Time Control:**
- Real-time clock (RTC) using 32.768 kHz crystal
- Millisecond-resolution uptime counter
- Time synchronization via telecommand
- Timestamp generation for all telemetry

**Clock Sources:**
- **MCLK (Main Clock):** 32 MHz - CPU and high-speed peripherals
- **SMCLK (Sub-Main Clock):** 32 MHz - Peripheral modules
- **ACLK (Auxiliary Clock):** 32.768 kHz - Low-power timers and RTC

**Time Synchronization:**
- Ground station can set system time via telecommand
- All telemetry packets timestamped with system time
- Time control task maintains accurate timekeeping

---

### 6. Operational Mode Management

#### Normal Mode
- All systems active
- Periodic telemetry collection
- Beacon transmission every 60 seconds
- Full telecommand processing
- Data logging active

#### Hibernation Mode
- **Purpose:** Power conservation during eclipse or low-battery conditions
- **Behavior:**
  - Reduced task execution
  - Minimal beacon transmission
  - Limited telecommand processing
  - Payload shutdown
  - Time-based automatic wake-up
- **Entry Conditions:**
  - Low battery voltage (EPS-triggered)
  - Manual telecommand
  - Antenna deployment failure
- **Exit Conditions:**
  - Battery voltage recovery
  - Time-based schedule
  - Manual telecommand
  - System reset

---

### 7. Antenna Deployment Control

**ISIS Antenna Deployer Management:**
- Automated deployment sequence on first boot
- Multiple deployment strategies:
  - Independent antenna burn (10 seconds per antenna)
  - Sequential burn (20 seconds total)
- Deployment verification
- **Safety Features:**
  - Maximum 10 deployment attempts
  - Hibernation mode entry on repeated failures (45 minutes)
  - Status monitoring and telemetry

**Deployment Task:**
- Priority: 5 (highest)
- Configurable via `TASK_ANTENNA_ENABLED` flag
- Default: Disabled (manual control preferred)

---

### 8. Watchdog and System Reliability

**Watchdog Timer Functions:**
- **Periodic Reset Task:** ~100 ms period, priority 1
- Prevents system lockup
- Monitors critical task execution
- **Reset Cause Tracking:**
  - Power-on reset (POR)
  - Watchdog timeout
  - Software reset
  - External reset
  - Brownout detection

**System Reset Capability:**
- Controlled reset via telecommand
- Task-based reset mechanism
- Reset counter preserved in non-volatile memory
- Last reset cause stored for diagnostics

**Voltage Monitoring:**
- TPS382x voltage monitor/supervisor IC
- Brownout detection and protection
- Power-on reset generation

---

### 9. Payload Management

**Payload Interface Support:**
- Up to 4 simultaneous payloads
- Generic data interface (220 bytes max per packet)
- Enable/disable control via telecommand
- Payload-specific data collection tasks

**Supported Payloads:**
1. **EDC (Earth Data Collection):**
   - UART interface
   - Configurable data rates
   - On-demand data retrieval
2. **Payload_X:** Generic interface for future expansion
3. **HARSH (Radiation Monitor):** Scientific data collection

**Payload Control:**
- Individual enable/disable commands
- Power management through EPS
- Data encapsulation in FloripaSat packets
- Downlink scheduling

---

### 10. Debug and Diagnostics

**System Logging (sys_log):**
- UART-based debug output (115200 bps)
- Configurable log levels
- Task activity monitoring
- Error and status reporting

**Diagnostic Information:**
- Task execution status
- Memory usage statistics
- Communication link status
- Subsystem health indicators
- Error logs and event history

**Test and Development Support:**
- Unit test framework
- Mockup tasks for FreeRTOS
- Hardware-in-the-loop testing capability

---

## System Capabilities Summary

| Functionality | Capability |
|---------------|------------|
| **Operating Modes** | Normal, Hibernation |
| **Subsystem Management** | EPS, TTC (x2), Antenna, Payloads (x4) |
| **Uplink Commands** | 13+ telecommand types |
| **Downlink Packets** | 7+ packet types |
| **Data Storage** | 128 KB Flash + 256 KB FRAM + 256 MB NOR |
| **Communication Interfaces** | 3 UART, 6 SPI, 3 I2C |
| **Real-Time Tasks** | 12 active tasks, 5 priority levels |
| **Beacon Period** | 60 seconds (configurable) |
| **Telemetry Collection** | 10 Hz (sensors), 60s (subsystems) |
| **Data Logging** | 5-minute intervals |
| **Time Resolution** | 1 millisecond |
| **Watchdog Period** | ~100 milliseconds |
| **Maximum Packet Size** | 220 bytes payload |
| **Security** | HMAC-SHA authentication |

---

## Mission Context

The OBDH 2.0 is designed for the FloripaSat-2 CubeSat mission, a university satellite project with the following goals:

- **Educational Mission:** Training students in satellite systems engineering
- **Technology Demonstration:** Testing Brazilian-designed space hardware
- **Scientific Payloads:** Earth observation and radiation monitoring
- **Communication:** Amateur radio beacon and data downlink

The OBDH provides the critical command and data handling infrastructure necessary for mission success, ensuring reliable operation in the harsh space environment.

---

## Related Documentation

- `01_firmware_architecture.md` - Detailed firmware architecture and design
- `02_datapath.md` - Data flow and packet structures
- `03_interfaces.md` - Communication interfaces and protocols
- `04_tasks.md` - FreeRTOS task descriptions and scheduling
- `05_pin_mapping.md` - GPIO and peripheral pin assignments
- `06_hardware_components_and_architecture.md` - Hardware design and components
