# OBDH 2.0 Firmware Architecture

## Document Overview

This document provides a comprehensive description of the OBDH 2.0 (On-Board Data Handling) firmware architecture, including software layers, data flow paths, task organization, communication protocols, and memory management.

**Target Mission:** FloripaSat-2 CubeSat
**Organization:** SpaceLab - Federal University of Santa Catarina
**Document Version:** 1.0
**Last Updated:** 2025-01-18

---

## Table of Contents

1. [System Overview](#system-overview)
2. [Architectural Layers](#architectural-layers)
3. [Task Architecture](#task-architecture)
4. [Data Flow and Data Paths](#data-flow-and-data-paths)
5. [Communication Architecture](#communication-architecture)
6. [Memory Architecture](#memory-architecture)
7. [Interrupt and Event Handling](#interrupt-and-event-handling)
8. [Power Management](#power-management)
9. [Error Handling and Fault Tolerance](#error-handling-and-fault-tolerance)
10. [Configuration Management](#configuration-management)

---

## System Overview

### Mission Context

The OBDH 2.0 is the central on-board computer for the FloripaSat-2 CubeSat mission. It serves as the primary data handling and command/control node, coordinating all satellite subsystems and managing the data flow between:

- **Earth Segment** (Ground Station)
- **TTC Module** (Telemetry, Tracking, and Command)
- **EPS Module** (Electrical Power System)
- **Antenna Module**
- **Payload Modules** (EDC, Payload-X, Harsh Environment)

### Key Responsibilities

1. **Data Acquisition**: Collect telemetry from all subsystems
2. **Data Storage**: Store telemetry in non-volatile memory
3. **Command Processing**: Execute commands from ground station
4. **Telemetry Packaging**: Format and transmit data to ground
5. **Time Management**: Maintain satellite time and schedule operations
6. **Fault Management**: Monitor system health and handle errors
7. **Mode Management**: Control satellite operational modes

### Firmware Characteristics

| Attribute | Value |
|-----------|-------|
| **RTOS** | FreeRTOS (ARM_CM4F port for STM32) |
| **Programming Language** | C (C99 standard) |
| **Architecture** | Layered, event-driven, multi-tasking |
| **Tasks** | 14+ concurrent tasks |
| **Memory Model** | Shared memory with mutex protection |
| **Communication** | I2C, SPI, UART-based protocols |
| **Timing** | Hard real-time constraints |

---

## Architectural Layers

The firmware is organized in a **layered architecture** that promotes modularity, reusability, and portability.

```
┌─────────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                        │
│  ┌──────────────┐  ┌──────────────┐  ┌─────────────────┐   │
│  │    Tasks     │  │   Libraries  │  │  Data Structs   │   │
│  │ (app/tasks)  │  │  (app/libs)  │  │ (app/structs)   │   │
│  └──────────────┘  └──────────────┘  └─────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                            ↕
┌─────────────────────────────────────────────────────────────┐
│                      DEVICE LAYER                           │
│  ┌─────┐  ┌─────┐  ┌──────┐  ┌────────┐  ┌──────────┐     │
│  │ EPS │  │ TTC │  │ Anten│  │ Sensor │  │  Media   │     │
│  └─────┘  └─────┘  └──────┘  └────────┘  └──────────┘     │
│            (devices/*)                                      │
└─────────────────────────────────────────────────────────────┘
                            ↕
┌─────────────────────────────────────────────────────────────┐
│                      DRIVER LAYER (HAL)                     │
│  ┌─────┐  ┌─────┐  ┌──────┐  ┌─────┐  ┌──────┐  ┌──────┐  │
│  │ SPI │  │ I2C │  │ UART │  │ ADC │  │ GPIO │  │ RTC  │  │
│  └─────┘  └─────┘  └──────┘  └─────┘  └──────┘  └──────┘  │
│            (drivers/*)                                      │
└─────────────────────────────────────────────────────────────┘
                            ↕
┌─────────────────────────────────────────────────────────────┐
│                      SYSTEM LAYER                           │
│  ┌───────────┐  ┌─────────┐  ┌──────────┐  ┌───────────┐  │
│  │  Clocks   │  │ Sys Log │  │  Setup   │  │  Hooks    │  │
│  └───────────┘  └─────────┘  └──────────┘  └───────────┘  │
│            (system/*)                                       │
└─────────────────────────────────────────────────────────────┘
                            ↕
┌─────────────────────────────────────────────────────────────┐
│                 RTOS LAYER (FreeRTOS)                       │
│  ┌──────────┐  ┌─────────┐  ┌──────────┐  ┌────────────┐  │
│  │Scheduler │  │  Tasks  │  │  Queues  │  │  Mutexes   │  │
│  └──────────┘  └─────────┘  └──────────┘  └────────────┘  │
└─────────────────────────────────────────────────────────────┘
                            ↕
┌─────────────────────────────────────────────────────────────┐
│              HARDWARE ABSTRACTION (STM32 HAL)               │
│                    STM32L476RG MCU                          │
└─────────────────────────────────────────────────────────────┘
```

### Layer Descriptions

#### 1. Application Layer (`app/`)

**Purpose**: Implements satellite mission logic and application-specific functionality.

**Components**:
- **Tasks** (`app/tasks/`): FreeRTOS tasks implementing satellite operations
- **Libraries** (`app/libs/`): Protocol implementations (FSP, HMAC, Packet handling)
- **Data Structures** (`app/structs/`): Satellite telemetry and data definitions

**Responsibilities**:
- Mission-specific logic
- Telemetry collection and formatting
- Command interpretation
- Data packaging
- Mode management

#### 2. Device Layer (`devices/`)

**Purpose**: Provides abstraction for satellite subsystems and external devices.

**Components**:
- `devices/eps/` - EPS (Electrical Power System) interface
- `devices/ttc/` - TTC (Telemetry, Tracking, Command) interface
- `devices/antenna/` - Antenna deployment and control
- `devices/payload/` - Payload interface
- `devices/media/` - Storage media abstraction (FRAM, NOR Flash, Internal Flash)
- `devices/watchdog/` - Watchdog device abstraction
- `devices/leds/` - LED indicators
- `devices/current_sensor/` - Current measurement
- `devices/voltage_sensor/` - Voltage measurement
- `devices/temp_sensor/` - Temperature measurement

**Responsibilities**:
- Device-specific initialization
- High-level device operations
- Data format conversion
- Device state management

#### 3. Driver Layer (`drivers/`)

**Purpose**: Hardware Abstraction Layer (HAL) for peripheral controllers.

**Components**:
- **Communication**: `spi/`, `i2c/`, `uart/`
- **Sensors**: `adc/`
- **Storage**: `flash/`, `cy15x102qn/` (FRAM), `mt25q/` (NOR Flash)
- **External Devices**: `isis_antenna/`, `edc/`, `phj/`, `px/`
- **Timing**: `rtc/`
- **Control**: `gpio/`, `wdt/`

**Responsibilities**:
- Low-level peripheral control
- Register-level operations
- Interrupt service routines
- DMA configuration
- Hardware-specific optimizations

#### 4. System Layer (`system/`)

**Purpose**: Core system services and infrastructure.

**Components**:
- `clocks.c` - System clock configuration
- `system.c` - System-level utilities (time, reset, version)
- `setup.c` - Hardware initialization
- `hooks.c` - FreeRTOS hooks (idle, stack overflow, malloc failed)
- `sys_log/` - Logging subsystem

**Responsibilities**:
- Clock tree configuration
- System initialization sequence
- Global system state
- Debug and logging infrastructure
- Error reporting

#### 5. RTOS Layer (FreeRTOS)

**Purpose**: Real-time operating system providing multitasking and synchronization.

**Features Used**:
- **Task Management**: 14+ concurrent tasks with priorities
- **Synchronization**: Mutexes, semaphores, event groups
- **Communication**: Queues for inter-task messaging
- **Memory Management**: Dynamic allocation with heap_4 scheme
- **Timing**: Software timers, delay functions

**Configuration**: `config/FreeRTOSConfig.h`
- CPU Clock: 80 MHz (STM32L476RG)
- Tick Rate: 1000 Hz (1 ms tick)
- Heap Size: 64 KB
- Priority Levels: 4 (0=lowest, 3=highest)

---

## Task Architecture

### Task Overview

The firmware implements a **multi-tasking architecture** with 14+ FreeRTOS tasks, each responsible for specific satellite functions.

```
Task Hierarchy (by priority):

Priority 3 (Highest):
  └─ Watchdog Reset Task

Priority 2:
  ├─ System Reset Task
  ├─ Process Telecommand Task
  └─ Heartbeat Task

Priority 1:
  ├─ Read Sensors Task
  ├─ Read EPS Task
  ├─ Read TTC Task
  ├─ Read Antenna Task
  ├─ Read EDC Task
  ├─ Beacon Task
  ├─ Time Control Task
  ├─ Data Log Task
  └─ Housekeeping Task

Priority 0 (Lowest):
  └─ Startup Task (self-deletes after completion)
```

### Task Catalog

#### 1. Startup Task

**File**: `app/tasks/startup.c`
**Priority**: 0 (runs once, then deletes itself)
**Period**: One-shot
**Stack Size**: 500 words

**Responsibilities**:
- Initialize all device drivers
- Initialize logging system
- Initialize storage media (Internal Flash, FRAM, NOR Flash)
- Initialize external subsystems (EPS, TTC, Antenna)
- Verify system integrity
- Set startup event flag for other tasks

**Initialization Sequence**:
```
1. sys_log_init()
2. media_init(MEDIA_INT_FLASH)
3. media_init(MEDIA_FRAM)
4. media_init(MEDIA_NOR)
5. leds_init()
6. current_sensor_init()
7. voltage_sensor_init()
8. temp_sensor_init()
9. eps_init()
10. ttc_init()
11. antenna_init()
12. payload_init()
13. Set TASK_STARTUP_DONE event flag
14. vTaskDelete(NULL)  // Delete self
```

#### 2. Watchdog Reset Task

**File**: `app/tasks/watchdog_reset.c`
**Priority**: 3 (Highest)
**Period**: 1000 ms (1 Hz)
**Stack Size**: 256 words

**Responsibilities**:
- Kick hardware watchdog timer
- Prevent system reset due to watchdog timeout
- Verify task scheduler is running correctly

**Operation**:
```c
while(1) {
    watchdog_reset();  // Kick the watchdog
    vTaskDelay(pdMS_TO_TICKS(1000));
}
```

#### 3. Heartbeat Task

**File**: `app/tasks/heartbeat.c`
**Priority**: 2
**Period**: 500 ms (2 Hz)
**Stack Size**: 256 words

**Responsibilities**:
- Toggle system LED (visual health indicator)
- Indicate system is alive and running
- Simple sanity check

#### 4. System Reset Task

**File**: `app/tasks/system_reset.c`
**Priority**: 2
**Period**: 60000 ms (1 minute)
**Stack Size**: 256 words

**Responsibilities**:
- Check for reset conditions
- Execute scheduled resets
- Log reset events
- Perform graceful shutdown before reset

#### 5. Read Sensors Task

**File**: `app/tasks/read_sensors.c`
**Priority**: 1
**Period**: 5000 ms (0.2 Hz)
**Stack Size**: 500 words

**Responsibilities**:
- Read OBDH internal sensors:
  - Current sensors
  - Voltage sensors
  - Temperature sensors
- Store readings in `sat_data_buf.obdh.data`
- Update timestamp

**Data Flow**:
```
Sensors → ADC → current_sensor_read() → sat_data_buf.obdh.data.current[]
                voltage_sensor_read() → sat_data_buf.obdh.data.voltage[]
                temp_sensor_read()    → sat_data_buf.obdh.data.temperature[]
```

#### 6. Read EPS Task

**File**: `app/tasks/read_eps.c`
**Priority**: 1
**Period**: 5000 ms (0.2 Hz)
**Stack Size**: 500 words

**Responsibilities**:
- Request telemetry from EPS module via I2C
- Parse EPS data packet
- Store in `sat_data_buf.eps.data`
- Handle communication errors

**Communication Protocol**: I2C-based FSP (FloripaSat Protocol)

#### 7. Read TTC Task

**File**: `app/tasks/read_ttc.c`
**Priority**: 1
**Period**: 5000 ms (0.2 Hz)
**Stack Size**: 500 words

**Responsibilities**:
- Request telemetry from TTC modules (TTC0, TTC1)
- Parse TTC data packets
- Store in `sat_data_buf.ttc_0.data` and `sat_data_buf.ttc_1.data`
- Monitor radio link status

**Communication Protocol**: I2C-based FSP

#### 8. Read Antenna Task

**File**: `app/tasks/read_antenna.c`
**Priority**: 1
**Period**: 10000 ms (0.1 Hz)
**Stack Size**: 500 words

**Responsibilities**:
- Query antenna deployment status
- Read antenna telemetry
- Store in `sat_data_buf.antenna.data`
- Monitor deployment sequence

**Communication Protocol**: I2C (ISIS Antenna protocol)

#### 9. Read EDC Task

**File**: `app/tasks/read_edc.c`
**Priority**: 1
**Period**: Variable (depends on payload state)
**Stack Size**: 512 words

**Responsibilities**:
- Interface with EDC (Energetic Particle Detector) payload
- Retrieve science data
- Store in `sat_data_buf.edc_0.data` and `sat_data_buf.edc_1.data`
- Manage payload power state

#### 10. Beacon Task

**File**: `app/tasks/beacon.c`
**Priority**: 1
**Period**: 60000 ms (1 minute) - configurable
**Stack Size**: 512 words

**Responsibilities**:
- Generate periodic beacon messages
- Package basic telemetry for beacon
- Transmit via TTC radio
- Comply with amateur radio regulations

**Beacon Content**:
- Satellite callsign (PY0EFS)
- Basic health status
- Battery voltage
- Uptime

#### 11. Time Control Task

**File**: `app/tasks/time_control.c`
**Priority**: 1
**Period**: 1000 ms (1 Hz)
**Stack Size**: 256 words

**Responsibilities**:
- Maintain satellite mission elapsed time
- Synchronize RTC with system time
- Handle time updates from ground
- Provide timestamps for telemetry

#### 12. Data Log Task

**File**: `app/tasks/data_log.c`
**Priority**: 1
**Period**: 60000 ms (1 minute) - configurable
**Stack Size**: 512 words

**Responsibilities**:
- Periodically save telemetry to NOR Flash
- Implement wear-leveling
- Manage flash memory addressing
- Compute CRC for data integrity

**Storage Format**:
```
[Data ID: 1 byte][Length: 1 byte][OBDH Data][EPS Data][TTC Data]...[CRC16: 2 bytes]
```

**Storage Media**: External NOR Flash (MT25Q)

#### 13. Process Telecommand Task

**File**: `app/tasks/process_tc.c`
**Priority**: 2
**Period**: Event-driven (queue-based)
**Stack Size**: 1024 words

**Responsibilities**:
- Receive telecommands from TTC
- Validate command authenticity (HMAC)
- Parse command structure
- Execute command actions
- Generate command acknowledgment
- Log command execution

**Command Types**:
- Ping request/response
- Data request
- Module activation/deactivation
- Payload control
- Parameter get/set
- Memory erase
- Force reset
- Hibernation mode

#### 14. Housekeeping Task

**File**: `app/tasks/housekeeping.c`
**Priority**: 1
**Period**: 10000 ms (0.1 Hz)
**Stack Size**: 256 words

**Responsibilities**:
- Monitor satellite operational mode
- Check hibernation timeout
- Perform mode transitions
- General health monitoring

**Operational Modes**:
- **NORMAL**: Full operation, all tasks active
- **HIBERNATION**: Low-power mode, reduced task activity
- **SAFE**: Minimal operation, fault recovery

#### 15. Antenna Deployment Task (Optional)

**File**: `app/tasks/antenna_deployment.c`
**Priority**: 1
**Period**: Event-driven
**Stack Size**: 512 words

**Responsibilities**:
- Execute antenna deployment sequence
- Monitor deployment timeout
- Retry deployment if necessary
- Update deployment status

---

## Data Flow and Data Paths

### Global Data Buffer

The firmware uses a **centralized global data buffer** for telemetry aggregation.

**Declaration**: `app/structs/satellite.h`

```c
typedef struct {
    obdh_telemetry_t obdh;          // OBDH sensors and status
    eps_telemetry_t eps;            // EPS telemetry
    ttc_telemetry_t ttc_0;          // TTC radio 0
    ttc_telemetry_t ttc_1;          // TTC radio 1
    antenna_telemetry_t antenna;    // Antenna status
    payload_telemetry_t edc_0;      // EDC payload 0
    payload_telemetry_t edc_1;      // EDC payload 1
    payload_telemetry_t payload_x;  // Payload-X
    payload_telemetry_t harsh;      // Harsh environment payload
} sat_data_t;

extern sat_data_t sat_data_buf;  // Global instance
```

Each telemetry structure contains:
- `timestamp` - System time when data was collected
- `data` - Subsystem-specific telemetry structure

### Data Flow Diagram

```
┌──────────────────────────────────────────────────────────────────┐
│                     EXTERNAL SUBSYSTEMS                          │
│  ┌──────┐  ┌──────┐  ┌─────────┐  ┌──────────┐  ┌──────────┐   │
│  │ EPS  │  │ TTC  │  │ Antenna │  │ Payloads │  │ Sensors  │   │
│  └───┬──┘  └───┬──┘  └────┬────┘  └─────┬────┘  └────┬─────┘   │
└──────┼─────────┼──────────┼─────────────┼───────────┼─────────┘
       │ I2C     │ I2C      │ I2C         │ UART      │ ADC
       ↓         ↓          ↓             ↓           ↓
┌──────────────────────────────────────────────────────────────────┐
│                      DRIVER LAYER (HAL)                          │
│      i2c_transfer()    spi_transfer()    adc_read()              │
└──────────────────────────────────────────────────────────────────┘
       ↓         ↓          ↓             ↓           ↓
┌──────────────────────────────────────────────────────────────────┐
│                       DEVICE LAYER                               │
│   eps_get_data()  ttc_recv()  antenna_get_status()  ...          │
└──────────────────────────────────────────────────────────────────┘
       ↓         ↓          ↓             ↓           ↓
┌──────────────────────────────────────────────────────────────────┐
│                    APPLICATION TASKS                             │
│  ┌──────────┐ ┌─────────┐ ┌───────────┐ ┌─────────────────┐    │
│  │ Read EPS │ │Read TTC │ │Read Antenna│ │ Read Sensors   │    │
│  └────┬─────┘ └────┬────┘ └─────┬─────┘ └────────┬────────┘    │
│       │            │            │                 │              │
│       ↓            ↓            ↓                 ↓              │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │           sat_data_buf (Global Data Buffer)              │   │
│  │  ┌──────┐ ┌──────┐ ┌─────────┐ ┌──────────┐ ┌────────┐  │   │
│  │  │ OBDH │ │ EPS  │ │   TTC   │ │ Antenna  │ │Payloads│  │   │
│  │  └──────┘ └──────┘ └─────────┘ └──────────┘ └────────┘  │   │
│  └──────────────────────────────────────────────────────────┘   │
│       │            │            │                              │
│       ↓            ↓            ↓                              │
│  ┌─────────┐  ┌──────────┐  ┌────────────┐                    │
│  │Data Log │  │ Beacon   │  │Process TC  │                    │
│  └────┬────┘  └────┬─────┘  └─────┬──────┘                    │
└───────┼────────────┼──────────────┼─────────────────────────────┘
        │            │              │
        ↓            ↓              ↓
┌──────────────────────────────────────────────────────────────────┐
│                    OUTPUT PATHS                                  │
│  ┌──────────┐  ┌──────────────┐  ┌──────────────────────────┐  │
│  │NOR Flash │  │ TTC Downlink │  │  Command Acknowledgment  │  │
│  │(Storage) │  │  (Beacon)    │  │      (Uplink)            │  │
│  └──────────┘  └──────────────┘  └──────────────────────────┘  │
└──────────────────────────────────────────────────────────────────┘
```

### Telemetry Collection Cycle

**Frequency**: Every 5 seconds

```
1. Read Sensors Task (every 5s)
   ├─ current_sensor_read() → sat_data_buf.obdh.data.current[3]
   ├─ voltage_sensor_read() → sat_data_buf.obdh.data.voltage[6]
   └─ temp_sensor_read()    → sat_data_buf.obdh.data.temperature[3]

2. Read EPS Task (every 5s)
   ├─ eps_request_data() via I2C
   ├─ Parse FSP packet
   └─ sat_data_buf.eps.data = parsed_eps_data

3. Read TTC Task (every 5s)
   ├─ ttc_request_data(TTC_0) via I2C
   ├─ ttc_request_data(TTC_1) via I2C
   └─ sat_data_buf.ttc_0.data / ttc_1.data = parsed_ttc_data

4. Read Antenna Task (every 10s)
   └─ sat_data_buf.antenna.data = antenna_status

5. Read EDC Task (variable)
   └─ sat_data_buf.edc_0.data / edc_1.data = payload_data
```

### Command Processing Datapath

```
┌─────────────────────────────────────────────────────────────┐
│                  GROUND STATION                             │
│            Sends encrypted telecommand                      │
└────────────────────┬────────────────────────────────────────┘
                     │ RF Link
                     ↓
┌─────────────────────────────────────────────────────────────┐
│                   TTC MODULE                                │
│  Receives RF, demodulates, forwards to OBDH via I2C         │
└────────────────────┬────────────────────────────────────────┘
                     │ I2C (FSP Protocol)
                     ↓
┌─────────────────────────────────────────────────────────────┐
│                  OBDH - I2C Driver                          │
│  Interrupt-based receive, buffer management                 │
└────────────────────┬────────────────────────────────────────┘
                     │
                     ↓
┌─────────────────────────────────────────────────────────────┐
│               TTC Device Layer                              │
│  ttc_recv() - Parse FSP packet, extract payload             │
└────────────────────┬────────────────────────────────────────┘
                     │ Queue
                     ↓
┌─────────────────────────────────────────────────────────────┐
│            Process Telecommand Task                         │
│  1. Dequeue command packet                                  │
│  2. Verify HMAC signature (authentication)                  │
│  3. Parse command ID and parameters                         │
│  4. Execute command action:                                 │
│     ├─ PING_REQ → Generate PING_ANS                         │
│     ├─ DATA_REQ → Retrieve stored telemetry                 │
│     ├─ ACTIVATE_MODULE → Enable subsystem                   │
│     ├─ ERASE_MEMORY → Erase NOR Flash region                │
│     ├─ SET_PARAM → Update configuration parameter           │
│     └─ FORCE_RESET → Schedule system reset                  │
│  5. Generate acknowledgment packet                          │
│  6. Send ACK via ttc_send()                                 │
└────────────────────┬────────────────────────────────────────┘
                     │
                     ↓
┌─────────────────────────────────────────────────────────────┐
│                  TTC Device Layer                           │
│  ttc_send() - Format FSP packet, transmit via I2C           │
└────────────────────┬────────────────────────────────────────┘
                     │ I2C (FSP Protocol)
                     ↓
┌─────────────────────────────────────────────────────────────┐
│                    TTC MODULE                               │
│  Modulates data, transmits via RF                           │
└────────────────────┬────────────────────────────────────────┘
                     │ RF Link
                     ↓
┌─────────────────────────────────────────────────────────────┐
│                  GROUND STATION                             │
│            Receives acknowledgment                          │
└─────────────────────────────────────────────────────────────┘
```

### Data Storage Datapath

```
Data Log Task (every 60s)
   ↓
1. Read sat_data_buf (all telemetry)
   ↓
2. Format data packet:
   ┌────────────────────────────────────────────┐
   │ [ID:1][Len:1][OBDH][EPS][TTC][Ant][CRC:2] │
   └────────────────────────────────────────────┘
   ↓
3. Compute CRC16-CCITT for integrity
   ↓
4. media_write(MEDIA_NOR, address, packet, length)
   ↓
5. NOR Flash Driver (MT25Q)
   ├─ spi_select(CS_NOR_FLASH)
   ├─ spi_transfer(WRITE_ENABLE_CMD)
   ├─ spi_transfer(PAGE_PROGRAM_CMD)
   ├─ spi_transfer(address[3])
   ├─ spi_transfer(data[256])
   └─ spi_deselect()
   ↓
6. Increment address pointer (wear leveling)
   ↓
7. Update flash metadata in FRAM
```

---

## Communication Architecture

### Inter-Module Communication (FSP)

**FloripaSat Protocol (FSP)** is a lightweight packet-based protocol for communication between satellite modules.

**Protocol Stack**:
```
┌──────────────────────────────────┐
│   Application Layer (Commands)  │
├──────────────────────────────────┤
│   FSP (Packet Framing, CRC)     │
├──────────────────────────────────┤
│   I2C Transport Layer            │
├──────────────────────────────────┤
│   Physical (I2C Bus)             │
└──────────────────────────────────┘
```

**FSP Packet Format**:
```
┌──────┬──────┬──────┬────────┬──────┬─────────────┬────────┐
│ SOD  │ SRC  │ DST  │ LENGTH │ TYPE │   PAYLOAD   │  CRC16 │
├──────┼──────┼──────┼────────┼──────┼─────────────┼────────┤
│ 0x7E │ 1B   │ 1B   │  1B    │  1B  │  0-248 B    │  2B    │
└──────┴──────┴──────┴────────┴──────┴─────────────┴────────┘
Total: 7-255 bytes
```

**Field Descriptions**:
- **SOD** (Start of Data): Fixed 0x7E marker
- **SRC** (Source Address): Module ID (1=EPS, 2=TTC, 3=OBDH)
- **DST** (Destination Address): Target module ID
- **LENGTH**: Total packet length in bytes
- **TYPE**: Packet type (DATA, CMD, ACK, NACK)
- **PAYLOAD**: Variable-length data
- **CRC16**: CRC16-CCITT checksum for error detection

**Packet Types**:
1. `FSP_PKT_TYPE_DATA` - Data transmission
2. `FSP_PKT_TYPE_DATA_WITH_ACK` - Data with acknowledgment required
3. `FSP_PKT_TYPE_CMD` - Command packet
4. `FSP_PKT_TYPE_CMD_WITH_ACK` - Command with acknowledgment
5. `FSP_PKT_TYPE_ACK` - Acknowledgment
6. `FSP_PKT_TYPE_NACK` - Negative acknowledgment

**Example Communication Sequence**:
```
OBDH → EPS: Request telemetry
┌──────┬─────┬─────┬────┬──────┬─────┬────────┐
│ 0x7E │  3  │  1  │ 8  │  4   │ 0x02│ CRC16  │
└──────┴─────┴─────┴────┴──────┴─────┴────────┘
         OBDH  EPS   Len  CMD_ACK SEND_DATA

EPS → OBDH: Send telemetry data
┌──────┬─────┬─────┬────┬──────┬───────────────┬────────┐
│ 0x7E │  1  │  3  │ 52 │  2   │  EPS Data(45B)│ CRC16  │
└──────┴─────┴─────┴────┴──────┴───────────────┴────────┘
         EPS  OBDH  Len DATA_ACK    Payload

OBDH → EPS: Acknowledge receipt
┌──────┬─────┬─────┬────┬──────┬────────┐
│ 0x7E │  3  │  1  │ 7  │  5   │ CRC16  │
└──────┴─────┴─────┴────┴──────┴────────┘
         OBDH  EPS  Len   ACK
```

### Ground Communication Protocol

**FSAT Packet Protocol** is used for ground-to-satellite communication.

**Security**:
- HMAC-SHA1 authentication for uplink commands
- Pre-shared key stored in `config/keys.h`
- Replay attack prevention via sequence numbers

**Command Packet Structure**:
```
┌──────────┬────────────┬──────────┬────────────┬────────────┐
│  Header  │ Command ID │ Sequence │ Parameters │    HMAC    │
├──────────┼────────────┼──────────┼────────────┼────────────┤
│   4B     │     1B     │    2B    │  Variable  │    20B     │
└──────────┴────────────┴──────────┴────────────┴────────────┘
```

**Uplink Commands** (Ground → Satellite):
| Command ID | Name | Description |
|-----------|------|-------------|
| 0x40 | PING_REQ | Ping request |
| 0x41 | DATA_REQ | Request stored telemetry |
| 0x42 | BROADCAST_MSG | Broadcast message |
| 0x43 | ENTER_HIBERNATION | Enter low-power mode |
| 0x44 | LEAVE_HIBERNATION | Exit low-power mode |
| 0x45 | ACTIVATE_MODULE | Enable subsystem |
| 0x46 | DEACTIVATE_MODULE | Disable subsystem |
| 0x47 | ACTIVATE_PAYLOAD | Enable payload |
| 0x48 | DEACTIVATE_PAYLOAD | Disable payload |
| 0x49 | ERASE_MEMORY | Erase storage region |
| 0x4A | FORCE_RESET | Force system reset |
| 0x4B | GET_PAYLOAD_DATA | Retrieve payload data |
| 0x4C | SET_PARAM | Set parameter |
| 0x4D | GET_PARAM | Get parameter |

**Downlink Packets** (Satellite → Ground):
| Packet ID | Name | Description |
|----------|------|-------------|
| 0x20 | GENERAL_TELEMETRY | Full telemetry dump |
| 0x21 | PING_ANS | Ping response |
| 0x22 | DATA_REQUEST_ANS | Stored data response |
| 0x23 | MESSAGE_BROADCAST | Broadcast message |
| 0x24 | PAYLOAD_DATA | Payload science data |
| 0x25 | TC_FEEDBACK | Command acknowledgment |
| 0x26 | PARAM_VALUE | Parameter value response |

### I2C Bus Architecture

**Bus Configuration**:
- **I2C0** (I2C2 peripheral): EPS, Sensors
  - Speed: 100 kHz (standard mode)
  - Pull-ups: 4.7kΩ external

- **I2C1** (I2C1 peripheral): TTC modules, Antenna
  - Speed: 100 kHz (standard mode)
  - Pull-ups: 4.7kΩ external

**Device Addresses**:
```
I2C0 Bus:
├─ EPS Module: 0x36 (7-bit)
└─ Current Sensors: 0x40-0x42

I2C1 Bus:
├─ TTC Module 0: 0x20
├─ TTC Module 1: 0x21
└─ ISIS Antenna: 0x31/0x32 (dual redundant)
```

**I2C Transaction Flow**:
```
Task calls device function:
  eps_get_data()
    ↓
  i2c_write(I2C0, EPS_ADDR, fsp_request_packet, len)
    ↓
  [I2C START] [ADDR+W] [DATA...] [STOP]
    ↓
  Wait for response (timeout 100ms)
    ↓
  i2c_read(I2C0, EPS_ADDR, fsp_response_packet, len)
    ↓
  [I2C START] [ADDR+R] [DATA...] [STOP]
    ↓
  Parse FSP packet, extract payload
    ↓
  Return to task
```

### SPI Bus Architecture

**Bus Configuration**:
- **SPI0** (SPI1 peripheral): External Flash, FRAM, Radio
  - Speed: 10 MHz
  - Mode: 0 (CPOL=0, CPHA=0)
  - Chip Selects: 4 (PA4, PC8, PC9, PA8)

- **SPI1** (SPI2 peripheral): Reserved for expansion
  - Speed: 10 MHz

- **SPI2** (SPI3 peripheral): Reserved for expansion
  - Speed: 10 MHz

**SPI Device Mapping**:
```
SPI0 Bus:
├─ CS0 (PA4): MT25Q NOR Flash (256 Mbit)
├─ CS1 (PC8): CY15x102QN FRAM (1 Mbit)
├─ CS2 (PC9): Si4463 Radio Transceiver
└─ CS3 (PA8): Reserved
```

**SPI Transaction Example** (NOR Flash Read):
```
media_read(MEDIA_NOR, 0x1000, buffer, 256)
  ↓
mt25q_read(0x1000, buffer, 256)
  ↓
spi_select(SPI_PORT_0, SPI_CS_0)  // Assert CS0 (PA4)
  ↓
spi_transfer(SPI_PORT_0, READ_CMD)  // 0x03
spi_transfer(SPI_PORT_0, addr[23:16])
spi_transfer(SPI_PORT_0, addr[15:8])
spi_transfer(SPI_PORT_0, addr[7:0])
for (i=0; i<256; i++)
  buffer[i] = spi_transfer(SPI_PORT_0, 0xFF)
  ↓
spi_deselect(SPI_PORT_0, SPI_CS_0)  // Deassert CS0
  ↓
Return to caller
```

### UART Communication

**UART Configuration**:
- **UART0** (USART2): Debug console
  - Baud: 115200 bps
  - Data: 8 bits, No parity, 1 stop bit
  - Flow control: None
  - Usage: sys_log output, debug messages

- **UART1** (USART6): Payload communication
  - Baud: 115200 bps
  - Usage: EDC, Payload-X data transfer

- **UART2** (USART1): Reserved
  - Baud: 115200 bps

**Logging System** (`system/sys_log/`):
```
Application code:
  sys_log_print_event_from_module(SYS_LOG_INFO, "TaskName", "Message");
    ↓
  Format string with timestamp and module name
    ↓
  uart_write(UART_PORT_0, formatted_string, length)
    ↓
  Interrupt-based transmission
    ↓
  Debug console (ground support equipment)
```

---

## Memory Architecture

### Memory Map

**STM32L476RG Memory Regions**:
```
┌─────────────────────────────────────────────────────────────┐
│ 0x0800 0000 - 0x080F FFFF │ Internal Flash (1024 KB)       │
│                            │ - Firmware code               │
│                            │ - Constant data (read-only)   │
├─────────────────────────────────────────────────────────────┤
│ 0x2000 0000 - 0x2001 FFFF │ SRAM (128 KB)                  │
│                            │ - .data section               │
│                            │ - .bss section                │
│                            │ - Heap (64 KB for FreeRTOS)   │
│                            │ - Stacks (task stacks)        │
├─────────────────────────────────────────────────────────────┤
│ 0x4000 0000 - 0x5FFF FFFF │ Peripherals                    │
│                            │ - GPIO, SPI, I2C, UART, etc.  │
└─────────────────────────────────────────────────────────────┘
```

**External Memory**:
```
┌─────────────────────────────────────────────────────────────┐
│ NOR Flash (MT25Q256) - 32 MB                                │
│ ├─ Telemetry Log Storage                                    │
│ ├─ Payload Data Storage                                     │
│ └─ Firmware Update Image (future)                           │
├─────────────────────────────────────────────────────────────┤
│ FRAM (CY15x102QN) - 128 KB                                  │
│ ├─ Configuration Parameters                                 │
│ ├─ Event Log (power cycles, resets)                         │
│ ├─ Persistent State (deployment flags, etc.)                │
│ └─ Flash Metadata (write pointers)                          │
├─────────────────────────────────────────────────────────────┤
│ Internal Flash (Last sector) - 2 KB                         │
│ └─ Bootloader Parameters (future)                           │
└─────────────────────────────────────────────────────────────┘
```

### Memory Hierarchy

```
┌───────────────────────────────────────────────────────────┐
│                     CPU Registers                         │
│                    (Fastest Access)                       │
└─────────────────────┬─────────────────────────────────────┘
                      │
┌─────────────────────▼─────────────────────────────────────┐
│                  SRAM (128 KB)                            │
│  - Global variables (sat_data_buf)                        │
│  - Task stacks                                            │
│  - FreeRTOS heap (dynamic allocation)                     │
│  - DMA buffers                                            │
│  Access time: ~12.5 ns @ 80 MHz                           │
└─────────────────────┬─────────────────────────────────────┘
                      │
┌─────────────────────▼─────────────────────────────────────┐
│            Internal Flash (1024 KB)                       │
│  - Firmware code (read-only)                              │
│  - Constant data (lookup tables, strings)                 │
│  - Config defaults                                        │
│  Access time: ~25 ns (4 wait states @ 80 MHz)             │
└─────────────────────┬─────────────────────────────────────┘
                      │
┌─────────────────────▼─────────────────────────────────────┐
│              FRAM (128 KB via SPI)                        │
│  - Non-volatile configuration                             │
│  - Event log                                              │
│  - Persistent state                                       │
│  Access time: ~10 μs (SPI transfer @ 10 MHz)              │
└─────────────────────┬─────────────────────────────────────┘
                      │
┌─────────────────────▼─────────────────────────────────────┐
│           NOR Flash (32 MB via SPI)                       │
│  - Telemetry archive                                      │
│  - Payload science data                                   │
│  - Firmware update images                                 │
│  Write time: ~2 ms per page (256 bytes)                   │
│  Erase time: ~40 ms per sector (64 KB)                    │
└───────────────────────────────────────────────────────────┘
```

### Flash Management

**FRAM (Ferroelectric RAM)**:
- **Technology**: Non-volatile, unlimited write endurance
- **Size**: 128 KB (CY15B102QN)
- **Interface**: SPI
- **Use Cases**:
  - Configuration parameters (frequently updated)
  - Boot counters
  - Deployment flags
  - Flash write pointers (for wear leveling)

**NOR Flash (External)**:
- **Technology**: Serial NOR Flash
- **Size**: 32 MB (MT25Q256)
- **Interface**: Quad-SPI (currently used in standard SPI mode)
- **Page Size**: 256 bytes
- **Sector Size**: 64 KB
- **Erase Granularity**: Sector (64 KB) or Chip (full erase)

**Wear Leveling Strategy**:
```
1. Divide NOR Flash into circular buffer
2. Store write pointer in FRAM (survives power cycles)
3. Each data log write:
   - Read current pointer from FRAM
   - Write data to NOR at pointer address
   - Increment pointer (wrap at end of flash)
   - Update pointer in FRAM
4. Erase sector when pointer crosses sector boundary
```

**Data Integrity**:
- CRC16-CCITT computed for each data packet
- Stored alongside data in NOR Flash
- Verified during data retrieval

### Memory Usage Estimates

**SRAM Allocation** (128 KB total):
```
.data + .bss sections:       ~20 KB
  ├─ sat_data_buf              ~5 KB
  ├─ Device buffers            ~5 KB
  └─ Other globals            ~10 KB

FreeRTOS Heap:               ~64 KB
  ├─ Task control blocks       ~2 KB
  ├─ Queues                    ~2 KB
  └─ Dynamic allocations      ~60 KB

Task Stacks:                 ~40 KB
  ├─ Startup Task (500 words)  2 KB
  ├─ Process TC (1024 words)   4 KB
  ├─ Data Log (512 words)      2 KB
  ├─ Read tasks (10x500)      20 KB
  └─ Other tasks (11x256)     11 KB

Reserved:                     ~4 KB
  └─ DMA buffers, alignment

Total:                      ~128 KB
```

**Flash Usage** (1024 KB total):
```
Firmware code:              ~400 KB
Libraries (FreeRTOS, HAL):  ~150 KB
Constant data:               ~50 KB
Reserved for bootloader:     ~32 KB (future)
Free space:                 ~392 KB
```

---

## Interrupt and Event Handling

### Interrupt Architecture

The firmware uses a **priority-based interrupt system** provided by ARM Cortex-M4 NVIC (Nested Vectored Interrupt Controller).

**Interrupt Priority Levels** (STM32L476RG):
- 4 priority bits = 16 priority levels (0-15)
- Lower number = higher priority
- Priority grouping: 4 bits for preemption, 0 for sub-priority

**FreeRTOS Integration**:
- Interrupts at priority ≤ `configMAX_SYSCALL_INTERRUPT_PRIORITY` (5) can call FreeRTOS APIs
- Interrupts at priority > 5 cannot use FreeRTOS functions

**Interrupt Priority Assignment**:
```
Priority 0 (Highest - Non-maskable):
  └─ NMI (Clock Security System failure)

Priority 1-4 (High - Cannot call FreeRTOS):
  ├─ HardFault, MemManage, BusFault
  └─ Critical hardware errors

Priority 5-10 (Medium - Can call FreeRTOS):
  ├─ UART RX (Priority 6)
  ├─ I2C Events (Priority 7)
  ├─ SPI Transfer Complete (Priority 8)
  ├─ ADC Conversion Complete (Priority 9)
  └─ Timer interrupts (Priority 10)

Priority 11-15 (Low):
  ├─ SysTick (Priority 15) - FreeRTOS tick
  ├─ PendSV (Priority 15) - FreeRTOS context switch
  └─ Other low-priority events

Priority 16 (Lowest):
  └─ Idle / Background tasks
```

### Interrupt Service Routines (ISRs)

**UART RX Interrupt** (Circular Buffer):
```c
void USART2_IRQHandler(void) {
    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE)) {
        uint8_t data = (uint8_t)(huart2.Instance->RDR);

        // Store in circular buffer
        uart_rx_buffer[uart_rx_head] = data;
        uart_rx_head = (uart_rx_head + 1) % UART_RX_BUFFER_SIZE;

        // Signal waiting task if queue full
        if (uart_rx_count >= UART_RX_THRESHOLD) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xSemaphoreGiveFromISR(uart_rx_semaphore, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}
```

**I2C Event Interrupt** (DMA-based):
```c
void I2C2_EV_IRQHandler(void) {
    HAL_I2C_EV_IRQHandler(&hi2c2);
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    // Transmission complete
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(i2c_tx_complete, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
```

**SysTick ISR** (FreeRTOS Tick):
```c
void SysTick_Handler(void) {
    HAL_IncTick();  // Increment HAL tick counter

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xPortSysTickHandler();  // FreeRTOS tick handler
    }
}
```

### Event Synchronization

**FreeRTOS Synchronization Primitives**:

1. **Semaphores** (Binary/Counting):
   - I2C transfer complete
   - SPI transfer complete
   - UART data available

2. **Mutexes**:
   - `sat_data_buf` access mutex
   - I2C bus mutex (prevents concurrent access)
   - SPI bus mutex
   - Media (Flash/FRAM) access mutex

3. **Event Groups**:
   - Startup completion event
   - Task ready flags
   - System state events

4. **Queues**:
   - Telecommand queue (Process TC task)
   - Log message queue

**Example: Startup Synchronization**:
```c
// In startup.c:
EventGroupHandle_t task_startup_status;

void vTaskStartup(void) {
    // Initialize all devices...

    // Set startup complete flag
    xEventGroupSetBits(task_startup_status, TASK_STARTUP_DONE);

    vTaskDelete(NULL);  // Self-delete
}

// In other tasks (e.g., data_log.c):
void vTaskDataLog(void) {
    // Wait for startup to complete
    xEventGroupWaitBits(
        task_startup_status,
        TASK_STARTUP_DONE,
        pdFALSE,     // Don't clear on exit
        pdTRUE,      // Wait for all bits
        pdMS_TO_TICKS(TASK_DATA_LOG_INIT_TIMEOUT_MS)
    );

    // Proceed with normal operation...
}
```

**Example: Shared Data Access**:
```c
extern SemaphoreHandle_t sat_data_mutex;

// In Read EPS Task:
void vTaskReadEPS(void) {
    while(1) {
        eps_data_t eps_data;

        if (eps_get_data(&eps_data) == 0) {
            // Acquire mutex before writing to shared buffer
            if (xSemaphoreTake(sat_data_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                sat_data_buf.eps.timestamp = system_get_time();
                sat_data_buf.eps.data = eps_data;
                xSemaphoreGive(sat_data_mutex);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
```

---

## Power Management

### Power Modes

The OBDH firmware implements **three operational modes** to manage power consumption:

1. **NORMAL Mode**:
   - All tasks active
   - Full telemetry collection
   - Regular beacon transmission
   - CPU running at 80 MHz

2. **HIBERNATION Mode**:
   - Reduced task activity
   - Minimal telemetry collection
   - No beacon transmission (or extended interval)
   - Payloads powered off
   - Duration-based (configurable timeout)

3. **SAFE Mode** (future):
   - Minimal operation for fault recovery
   - Only critical tasks running
   - Attempt to restore communication with ground

**Mode Transitions**:
```
┌────────────────────────────────────────────────────────────┐
│                      NORMAL MODE                           │
│  - All tasks running                                       │
│  - Power: ~1.5W (OBDH only)                                │
└─────────────┬──────────────────────────┬───────────────────┘
              │                          │
              │ ENTER_HIBERNATION cmd    │ Battery < threshold
              │                          │ (via EPS monitoring)
              ↓                          ↓
┌──────────────────────────────────────────────────────────┐
│                   HIBERNATION MODE                       │
│  - Reduced activity                                      │
│  - Payloads OFF                                          │
│  - Power: ~0.8W                                          │
└─────────────┬────────────────────────────────────────────┘
              │
              │ Timeout expires OR
              │ LEAVE_HIBERNATION cmd OR
              │ Battery > threshold
              ↓
┌──────────────────────────────────────────────────────────┐
│                      NORMAL MODE                         │
└──────────────────────────────────────────────────────────┘
```

**Mode Control** (in Housekeeping Task):
```c
void vTaskHousekeeping(void) {
    while(1) {
        // Check if in hibernation mode
        if (sat_data_buf.obdh.data.mode == OBDH_MODE_HIBERNATION) {
            uint32_t elapsed = system_get_time() - sat_data_buf.obdh.data.ts_last_mode_change;

            if (elapsed >= sat_data_buf.obdh.data.mode_duration) {
                // Hibernation timeout expired, return to normal
                sat_data_buf.obdh.data.mode = OBDH_MODE_NORMAL;
                sat_data_buf.obdh.data.ts_last_mode_change = system_get_time();

                // Re-enable payloads, increase telemetry rate, etc.
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
```

### Low-Power Techniques

**1. Clock Gating**:
- Unused peripherals are clock-gated via RCC
- Peripherals enabled only when needed

**2. FreeRTOS Idle Hook**:
```c
void vApplicationIdleHook(void) {
    // Enter SLEEP mode when idle
    // Wake on any interrupt
    __WFI();  // Wait For Interrupt
}
```

**3. Dynamic Frequency Scaling** (future):
- Reduce CPU frequency during low-activity periods
- Switch from 80 MHz to 16 MHz when not processing telecommands

**4. Peripheral Power Control**:
- Payloads powered via GPIO-controlled MOSFETs
- Radio transmitter duty-cycled

---

## Error Handling and Fault Tolerance

### Watchdog System

**Hardware Watchdog** (IWDG - Independent Watchdog):
- Clocked by LSI (32 kHz internal RC oscillator)
- Timeout: ~4 seconds (configurable)
- Cannot be stopped once started
- Reset occurs if not refreshed

**Watchdog Reset Task**:
```c
void vTaskWatchdogReset(void) {
    while(1) {
        watchdog_reset();  // Kick the watchdog
        vTaskDelay(pdMS_TO_TICKS(1000));  // 1 Hz refresh rate
    }
}
```

**Failure Scenario**:
If the scheduler hangs or any task blocks indefinitely, the watchdog will not be reset, causing a hardware reset after ~4 seconds.

### Fault Handlers

**Hard Fault Handler**:
```c
void HardFault_Handler(void) {
    // Log fault information to FRAM
    uint32_t *stack_ptr = (uint32_t *)__get_MSP();
    fault_log_t fault = {
        .pc = stack_ptr[6],    // Program Counter
        .lr = stack_ptr[5],    // Link Register
        .r0 = stack_ptr[0],
        .cfsr = SCB->CFSR,     // Configurable Fault Status Register
        .hfsr = SCB->HFSR,     // Hard Fault Status Register
        .timestamp = RTC_GetCounter()
    };

    fram_write(FAULT_LOG_ADDRESS, &fault, sizeof(fault));

    // Force system reset
    NVIC_SystemReset();
}
```

**Stack Overflow Detection**:
```c
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    // Log task name and reset
    sys_log_print_event_from_module(SYS_LOG_ERROR, "FreeRTOS", "Stack overflow in task: ");
    sys_log_print_msg(pcTaskName);
    sys_log_new_line();

    // Save to FRAM
    // ...

    // Reset system
    NVIC_SystemReset();
}
```

**Malloc Failure Hook**:
```c
void vApplicationMallocFailedHook(void) {
    // Log heap exhaustion
    sys_log_print_event_from_module(SYS_LOG_ERROR, "FreeRTOS", "Heap allocation failed!");

    // Attempt to free non-critical buffers
    // ...

    // If still failing, reset
    NVIC_SystemReset();
}
```

### Error Logging

**Event Log in FRAM**:
```c
typedef struct {
    uint32_t timestamp;
    uint8_t event_type;
    uint8_t source_module;
    uint16_t error_code;
    uint8_t data[16];
} event_log_entry_t;

// Circular buffer of 256 entries in FRAM
#define EVENT_LOG_BASE_ADDR  0x0000
#define EVENT_LOG_ENTRIES    256
```

**Event Types**:
- RESET (power-on, watchdog, manual)
- FAULT (hard fault, stack overflow)
- COMM_ERROR (I2C timeout, SPI error)
- DEVICE_ERROR (sensor read fail, EPS comm fail)
- COMMAND_EXEC (telecommand execution)
- MODE_CHANGE (normal ↔ hibernation)

### Reset Cause Detection

```c
uint8_t system_get_reset_cause(void) {
    uint32_t rcc_csr = RCC->CSR;

    if (rcc_csr & RCC_CSR_LPWRRSTF)
        return RESET_CAUSE_LOW_POWER;
    if (rcc_csr & RCC_CSR_WWDGRSTF)
        return RESET_CAUSE_WINDOW_WATCHDOG;
    if (rcc_csr & RCC_CSR_IWDGRSTF)
        return RESET_CAUSE_INDEPENDENT_WATCHDOG;
    if (rcc_csr & RCC_CSR_SFTRSTF)
        return RESET_CAUSE_SOFTWARE;
    if (rcc_csr & RCC_CSR_PORRSTF)
        return RESET_CAUSE_POWER_ON;
    if (rcc_csr & RCC_CSR_PINRSTF)
        return RESET_CAUSE_EXTERNAL_PIN;

    return RESET_CAUSE_UNKNOWN;
}
```

Logged during startup task for diagnostics.

### Data Integrity

**CRC Validation**:
- All NOR Flash data packets include CRC16-CCITT
- FSP packets include CRC16 for transport error detection
- HMAC-SHA1 for telecommand authentication

**Redundancy**:
- Dual TTC radios (TTC0, TTC1) for communication redundancy
- Dual ISIS Antenna controllers (addresses 0x31, 0x32)
- Critical configuration stored in both FRAM and Internal Flash

---

## Configuration Management

### Compile-Time Configuration

**File**: `config/config.h`

**Task Enable/Disable**:
```c
#define CONFIG_TASK_STARTUP_ENABLED            1
#define CONFIG_TASK_WATCHDOG_RESET_ENABLED     1
#define CONFIG_TASK_HEARTBEAT_ENABLED          1
#define CONFIG_TASK_BEACON_ENABLED             0  // Disabled
#define CONFIG_TASK_ANTENNA_DEPLOYMENT_ENABLED 0  // Disabled
// ... etc
```

**Device Enable/Disable**:
```c
#define CONFIG_DEV_MEDIA_INT_ENABLED           1
#define CONFIG_DEV_MEDIA_FRAM_ENABLED          1
#define CONFIG_DEV_MEDIA_NOR_ENABLED           1
#define CONFIG_DEV_EPS_ENABLED                 1
#define CONFIG_DEV_PAYLOAD_EDC_ENABLED         1
// ... etc
```

**Debug Control**:
```c
#define CONFIG_DRIVERS_DEBUG_ENABLED           0  // Disable for flight
```

### Runtime Configuration

**Parameters Stored in FRAM**:
- Beacon interval
- Telemetry logging interval
- Hibernation duration
- Radio power level
- Antenna deployment retry count

**Parameter Management**:
```c
int8_t param_set(uint8_t param_id, uint32_t value) {
    switch(param_id) {
        case PARAM_BEACON_INTERVAL:
            config.beacon_interval_sec = value;
            fram_write(CONFIG_BASE_ADDR, &config, sizeof(config));
            break;
        // ...
    }
    return 0;
}

uint32_t param_get(uint8_t param_id) {
    switch(param_id) {
        case PARAM_BEACON_INTERVAL:
            return config.beacon_interval_sec;
        // ...
    }
    return 0;
}
```

**Telecommands for Parameter Management**:
- `SET_PARAM (0x4C)`: Set parameter value
- `GET_PARAM (0x4D)`: Query parameter value

### Pin Mapping Configuration

**File**: `config/pinmap_stm32l476.h`

Centralized pin definitions for hardware abstraction:
```c
// SPI0 Pins
#define SPI0_SCK_PORT                GPIOA
#define SPI0_SCK_PIN                 GPIO_PIN_5
#define SPI0_MISO_PORT               GPIOA
#define SPI0_MISO_PIN                GPIO_PIN_6
// ...

// I2C0 Pins
#define I2C0_SCL_PORT                GPIOB
#define I2C0_SCL_PIN                 GPIO_PIN_10
// ...
```

Allows easy reconfiguration for different hardware revisions.

---

## Appendix

### Glossary

| Term | Definition |
|------|------------|
| **OBDH** | On-Board Data Handling (satellite computer) |
| **EPS** | Electrical Power System |
| **TTC** | Telemetry, Tracking, and Command (radio module) |
| **EDC** | Energetic Particle Detector (payload) |
| **FSP** | FloripaSat Protocol (inter-module communication) |
| **FRAM** | Ferroelectric RAM (non-volatile memory) |
| **NOR Flash** | NOR-type Flash memory for bulk storage |
| **HAL** | Hardware Abstraction Layer |
| **ISR** | Interrupt Service Routine |
| **DMA** | Direct Memory Access |
| **NVIC** | Nested Vectored Interrupt Controller |
| **WDT** | Watchdog Timer |

### Task Summary Table

| Task Name | Priority | Period (ms) | Stack (words) | Purpose |
|-----------|----------|-------------|---------------|---------|
| Startup | 0 | One-shot | 500 | Initialize all devices |
| Watchdog Reset | 3 | 1000 | 256 | Kick hardware watchdog |
| System Reset | 2 | 60000 | 256 | Monitor reset conditions |
| Process TC | 2 | Event | 1024 | Execute telecommands |
| Heartbeat | 2 | 500 | 256 | Blink system LED |
| Read Sensors | 1 | 5000 | 500 | Read OBDH sensors |
| Read EPS | 1 | 5000 | 500 | Get EPS telemetry |
| Read TTC | 1 | 5000 | 500 | Get TTC telemetry |
| Read Antenna | 1 | 10000 | 500 | Get antenna status |
| Read EDC | 1 | Variable | 512 | Get payload data |
| Beacon | 1 | 60000 | 512 | Transmit beacon |
| Time Control | 1 | 1000 | 256 | Maintain satellite time |
| Data Log | 1 | 60000 | 512 | Store telemetry to Flash |
| Housekeeping | 1 | 10000 | 256 | Mode management |

### Communication Protocol Summary

| Protocol | Layer | Medium | Usage |
|----------|-------|--------|-------|
| FSP | Application | I2C | Inter-module (EPS, TTC, Antenna) |
| FSAT Packet | Application | RF (via TTC) | Ground-to-satellite commands |
| HMAC-SHA1 | Security | - | Telecommand authentication |
| I2C | Transport | I2C bus | Device communication |
| SPI | Transport | SPI bus | Flash, FRAM, Radio access |
| UART | Transport | Serial | Debug console, payload |

### Directory Structure Reference

```
firmware/
├── app/                      # Application layer
│   ├── tasks/                # FreeRTOS task implementations
│   ├── libs/                 # Application libraries (FSP, HMAC, packets)
│   └── structs/              # Data structures (satellite telemetry)
├── devices/                  # Device layer (subsystem abstractions)
│   ├── eps/                  # EPS interface
│   ├── ttc/                  # TTC interface
│   ├── antenna/              # Antenna control
│   ├── payload/              # Payload interface
│   ├── media/                # Storage abstraction
│   └── ...
├── drivers/                  # Driver layer (HAL)
│   ├── spi/                  # SPI driver
│   ├── i2c/                  # I2C driver
│   ├── uart/                 # UART driver
│   ├── adc/                  # ADC driver
│   ├── gpio/                 # GPIO driver
│   ├── rtc/                  # RTC driver
│   ├── flash/                # Internal flash driver
│   ├── wdt/                  # Watchdog driver
│   ├── mt25q/                # NOR Flash chip driver
│   ├── cy15x102qn/           # FRAM chip driver
│   └── ...
├── system/                   # System layer
│   ├── clocks.c              # Clock configuration
│   ├── system.c              # System utilities
│   ├── setup.c               # Hardware initialization
│   ├── hooks.c               # FreeRTOS hooks
│   └── sys_log/              # Logging subsystem
├── config/                   # Configuration files
│   ├── config.h              # Feature enable/disable
│   ├── FreeRTOSConfig.h      # RTOS configuration
│   ├── pinmap_stm32l476.h    # Pin mappings
│   └── keys.h                # Security keys
├── freertos/                 # FreeRTOS kernel
├── stm32/                    # STM32-specific files
│   ├── STM32L476RGTx_FLASH.ld    # Linker script
│   ├── startup_stm32l476xx.s     # Startup code
│   ├── system_stm32l4xx.c        # System init
│   └── ...
├── main.c                    # Main entry point (MSP430)
├── main_stm32.c              # Main entry point (STM32)
└── Makefile.stm32            # Build system
```

---

## Document Change Log

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | 2025-01-18 | Initial comprehensive architecture document | Claude AI |

---

**End of Document**
