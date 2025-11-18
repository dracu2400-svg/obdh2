# OBDH2 Firmware Architecture

## Overview

The OBDH2 firmware follows a layered architecture design, providing clear separation of concerns and abstraction levels. This document describes the architectural organization, design patterns, and component interactions within the firmware.

**Firmware Version**: 0.10.0
**Real-Time OS**: FreeRTOS v10.2.1
**Hardware Abstraction**: MSP430 DriverLib v2.91.11.01
**Target Platform**: MSP430F6659 Microcontroller

## Architectural Layers

The firmware implements a five-layer architecture, from low-level hardware to high-level application logic:

```
┌─────────────────────────────────────────────────────────────┐
│                     APPLICATION LAYER                       │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ FreeRTOS Tasks (15+ concurrent tasks)                │  │
│  │ • Startup, Beacon, Data Log                          │  │
│  │ • Process TC, Housekeeping                           │  │
│  │ • Read Sensors, TTC, EPS, Antenna                    │  │
│  │ • Time Control, Watchdog Reset                       │  │
│  │ • Heartbeat, System Reset                            │  │
│  └──────────────────────────────────────────────────────┘  │
│  Location: firmware/app/tasks/                              │
├─────────────────────────────────────────────────────────────┤
│                       DEVICE LAYER                          │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ Device Abstractions (10+ devices)                    │  │
│  │ • TTC Device (Radio communication)                   │  │
│  │ • EPS Device (Power management)                      │  │
│  │ • Media Device (Storage subsystem)                   │  │
│  │ • Antenna Device (Deployment control)                │  │
│  │ • Sensors (Temperature, Voltage, Current)            │  │
│  │ • Payloads (EDC, scientific instruments)             │  │
│  │ • Watchdog, LEDs                                     │  │
│  └──────────────────────────────────────────────────────┘  │
│  Location: firmware/devices/                                │
├─────────────────────────────────────────────────────────────┤
│                       DRIVER LAYER                          │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ Protocol Drivers                                     │  │
│  │ • UART (uart.h/c)                                    │  │
│  │ • SPI (spi.h/c)                                      │  │
│  │ • I2C (i2c.h/c)                                      │  │
│  │ • GPIO (gpio.h/c)                                    │  │
│  │                                                      │  │
│  │ Component Drivers                                    │  │
│  │ • SL_TTC2 (Radio transceiver)                        │  │
│  │ • SL_EPS2 (Power subsystem)                          │  │
│  │ • MT25Q (NOR Flash)                                  │  │
│  │ • CY15x102QN (FRAM)                                  │  │
│  │ • ISIS Antenna (Deployment)                          │  │
│  │ • TPS382x (Watchdog)                                 │  │
│  │ • TCA4311A (I2C Level Shifter)                       │  │
│  └──────────────────────────────────────────────────────┘  │
│  Location: firmware/drivers/                                │
├─────────────────────────────────────────────────────────────┤
│          HAL (Hardware Abstraction Layer)                   │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ Texas Instruments MSP430 DriverLib v2.91.11.01       │  │
│  │ • Clock System (UCS, FLL)                            │  │
│  │ • USCI_A/B (UART, SPI, I2C)                          │  │
│  │ • Timer_A/B/D                                        │  │
│  │ • ADC12_A                                            │  │
│  │ • GPIO Port Mapping                                  │  │
│  │ • Watchdog Timer (WDT_A)                             │  │
│  │ • Real-Time Clock (RTC)                              │  │
│  └──────────────────────────────────────────────────────┘  │
│  Location: firmware/hal/                                    │
├─────────────────────────────────────────────────────────────┤
│              REAL-TIME OPERATING SYSTEM                     │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ FreeRTOS v10.2.1                                     │  │
│  │ • Task Scheduler (Preemptive)                        │  │
│  │ • Memory Management (Heap 4)                         │  │
│  │ • Queue Management                                   │  │
│  │ • Semaphores & Mutexes                               │  │
│  │ • Event Groups                                       │  │
│  │ • Software Timers                                    │  │
│  └──────────────────────────────────────────────────────┘  │
│  Location: firmware/freertos/                               │
├─────────────────────────────────────────────────────────────┤
│                        HARDWARE                             │
│  MSP430F6659 Microcontroller                                │
│  • 16-bit RISC Architecture                                 │
│  • 32 MHz Main Clock (MCLK)                                 │
│  • 32.768 kHz Auxiliary Clock (ACLK)                        │
│  • 256 KB Flash, 16 KB RAM                                  │
│  • 40 KB FreeRTOS Heap                                      │
└─────────────────────────────────────────────────────────────┘
```

## Directory Structure

```
firmware/
├── main.c                          # Firmware entry point, initialization
├── version.h                       # Version control (FIRMWARE_VERSION, etc.)
├── lnk_msp430f6659.cmd            # Linker script (memory layout)
│
├── config/                         # Configuration and feature flags
│   ├── config.h                    # Master configuration file
│   ├── FreeRTOSConfig.h            # RTOS configuration
│   └── radio_config_Si4463.h       # Radio transceiver settings
│
├── system/                         # System-level management
│   ├── system.h/c                  # System init, reset cause detection
│   ├── clocks.h/c                  # Clock configuration (32 MHz setup)
│   ├── setup.c                     # Timer interrupt for FreeRTOS tick
│   ├── queue/                      # System-wide queue implementations
│   └── sys_log/                    # Logging system (UART debug output)
│       ├── sys_log.h/c             # Log API
│       └── sys_log_config.h        # Log levels and formatting
│
├── app/                            # Application layer
│   ├── tasks/                      # FreeRTOS task implementations
│   │   ├── tasks.h/c               # Task registration and management
│   │   ├── startup.h/c             # System initialization task
│   │   ├── beacon.h/c              # Periodic telemetry broadcast
│   │   ├── data_log.h/c            # Data logging to non-volatile memory
│   │   ├── process_tc.h/c          # Telecommand processing
│   │   ├── housekeeping.h/c        # Mode management, hibernation
│   │   ├── read_sensors.h/c        # OBDH sensor acquisition
│   │   ├── read_eps.h/c            # EPS telemetry collection
│   │   ├── read_ttc.h/c            # TTC status and packet reception
│   │   ├── read_antenna.h/c        # Antenna status monitoring
│   │   ├── read_edc.h/c            # Payload data collection
│   │   ├── antenna_deployment.h/c  # Deployment sequence control
│   │   ├── time_control.h/c        # System time management
│   │   ├── watchdog_reset.h/c      # Watchdog refresh task
│   │   ├── heartbeat.h/c           # Status LED blink
│   │   └── system_reset.h/c        # Controlled reset execution
│   │
│   ├── structs/                    # Global data structures
│   │   ├── satellite.h             # Master satellite data buffer
│   │   ├── obdh_data.h             # OBDH-specific telemetry
│   │   ├── eps_data.h              # EPS telemetry structure
│   │   ├── ttc_data.h              # TTC telemetry structure
│   │   ├── antenna_data.h          # Antenna status structure
│   │   └── payload_data.h          # Payload data structure
│   │
│   └── libs/                       # Application utilities
│       ├── containers/             # Data containers (buffers, queues)
│       └── fsat_pkt/               # FloripaSat packet library
│
├── devices/                        # Device abstraction layer
│   ├── antenna/                    # Antenna device interface
│   ├── current_sensor/             # Current measurement device
│   ├── eps/                        # EPS device interface
│   ├── leds/                       # LED control device
│   ├── media/                      # Storage media abstraction
│   ├── payload/                    # Generic payload interface
│   ├── temp_sensor/                # Temperature sensor device
│   ├── ttc/                        # TTC radio device interface
│   ├── voltage_sensor/             # Voltage measurement device
│   └── watchdog/                   # Watchdog timer device
│
├── drivers/                        # Hardware drivers
│   ├── uart/                       # UART protocol driver
│   ├── spi/                        # SPI protocol driver
│   ├── i2c/                        # I2C protocol driver
│   ├── gpio/                       # GPIO control driver
│   ├── sl_ttc2/                    # SpaceLab TTC 2.0 driver
│   ├── sl_eps2/                    # SpaceLab EPS 2.0 driver
│   ├── mt25q/                      # Micron NOR Flash driver
│   ├── cy15x102qn/                 # Cypress FRAM driver
│   ├── isis_antenna/               # ISIS Antenna driver
│   ├── tps382x/                    # Watchdog IC driver
│   └── tca4311a/                   # I2C level shifter driver
│
├── hal/                            # Hardware Abstraction Layer
│   └── msp430/                     # MSP430 DriverLib
│
├── freertos/                       # FreeRTOS kernel
│   ├── Source/                     # Kernel source files
│   └── portable/                   # MSP430 port-specific code
│
├── libs/                           # External libraries
│   ├── containers/                 # Generic container library
│   └── ngham/                      # NGHam protocol library
│
└── tests/                          # Unit and integration tests
    ├── devices/                    # Device layer tests
    ├── drivers/                    # Driver layer tests
    └── tasks/                      # Task layer tests
```

## Layer Responsibilities

### 1. Application Layer

**Location**: firmware/app/

**Responsibility**: High-level application logic and task orchestration

**Components**:
- **Tasks**: Periodic and event-driven operations
- **Data Structures**: Global satellite telemetry buffers
- **Libraries**: Application-level utilities and protocols

**Design Pattern**: Task-based concurrency using FreeRTOS

**Key Files**:
- firmware/app/tasks/tasks.c:1 - Task registration and initialization
- firmware/app/structs/satellite.h:1 - Global data buffer definition

**Characteristics**:
- Hardware-independent business logic
- Direct FreeRTOS API usage
- Calls device layer for hardware operations
- Implements mission-specific functionality

### 2. Device Layer

**Location**: firmware/devices/

**Responsibility**: Device abstraction and high-level hardware interfaces

**Components**:
- TTC Device: Radio communication abstraction
- EPS Device: Power subsystem interface
- Media Device: Storage subsystem (Flash, FRAM)
- Antenna Device: Deployment control
- Sensor Devices: Temperature, voltage, current measurements
- Payload Device: Generic payload interface

**Design Pattern**: Abstract device interface with multiple implementations

**Example - TTC Device** (firmware/devices/ttc/ttc.h:1):
```c
// Device initialization
int ttc_init(ttc_id_t id);

// High-level operations
int ttc_send(ttc_id_t id, uint8_t *data, uint16_t len);
int ttc_recv(ttc_id_t id, uint8_t *data, uint16_t *len);
int ttc_get_data(ttc_id_t id, ttc_telemetry_t *data);

// Configuration
int ttc_set_param(ttc_id_t id, ttc_param_t param, uint32_t value);
int ttc_get_param(ttc_id_t id, ttc_param_t param, uint32_t *value);
```

**Characteristics**:
- Hides driver complexity from application
- Provides semantic operations (send, receive, configure)
- Manages device state and error handling
- Hardware-specific but platform-portable

### 3. Driver Layer

**Location**: firmware/drivers/

**Responsibility**: Hardware-specific drivers for peripherals and components

**Categories**:

**Protocol Drivers**:
- **UART** (firmware/drivers/uart/uart.c:1): Asynchronous serial communication
- **SPI** (firmware/drivers/spi/spi.c:1): Synchronous serial peripheral interface
- **I2C** (firmware/drivers/i2c/i2c.c:1): Inter-Integrated Circuit communication
- **GPIO** (firmware/drivers/gpio/gpio.c:1): General-purpose I/O control

**Component Drivers**:
- **SL_TTC2** (firmware/drivers/sl_ttc2/sl_ttc2.c:1): SpaceLab TTC 2.0 radio
- **SL_EPS2** (firmware/drivers/sl_eps2/sl_eps2.c:1): SpaceLab EPS 2.0 power module
- **MT25Q** (firmware/drivers/mt25q/mt25q.c:1): Micron NOR Flash memory
- **CY15x102QN** (firmware/drivers/cy15x102qn/): Cypress FRAM
- **ISIS Antenna** (firmware/drivers/isis_antenna/isis_antenna.c:1): Antenna deployment
- **TPS382x** (firmware/drivers/tps382x/tps382x.c:1): External watchdog timer

**Design Pattern**: Register-level access with error handling

**Characteristics**:
- Direct hardware register manipulation
- Minimal abstraction over HAL
- Component-specific protocols and commands
- Error detection and reporting

### 4. Hardware Abstraction Layer (HAL)

**Location**: firmware/hal/

**Responsibility**: Platform-specific low-level hardware access

**Components**:
- MSP430 DriverLib v2.91.11.01
- Peripheral initialization and control
- Interrupt management
- Clock system configuration

**Provided Modules**:
- UCS (Unified Clock System)
- USCI_A/B (Universal Serial Communication Interface)
- Timer_A/B/D
- ADC12_A (12-bit Analog-to-Digital Converter)
- GPIO
- WDT_A (Watchdog Timer)
- RTC (Real-Time Clock)

**Characteristics**:
- Vendor-provided library
- Register-level definitions
- Hardware initialization functions
- Interrupt vector management

### 5. Real-Time Operating System Layer

**Location**: firmware/freertos/

**Responsibility**: Task scheduling, memory management, and synchronization

**FreeRTOS Configuration** (firmware/config/FreeRTOSConfig.h:1):
- **Version**: 10.2.1
- **CPU Frequency**: 32000000 Hz (32 MHz)
- **Tick Rate**: 1000 Hz (1 ms per tick)
- **Max Priorities**: 5
- **Heap Size**: 40960 bytes (40 KB)
- **Memory Allocation**: Heap 4 (best-fit allocator)

**Features Enabled**:
- Preemptive scheduling
- Task notifications
- Queue management
- Mutexes and semaphores
- Event groups
- Software timers
- Idle hook function
- Tick hook function

**Tick Generation**: firmware/system/setup.c:1
- Uses Timer A0 driven by ACLK (32.768 kHz)
- Generates 1 ms interrupts for task switching

## Design Patterns and Principles

### 1. Layered Architecture

**Benefit**: Clear separation of concerns, modularity, testability

**Implementation**:
- Each layer only communicates with adjacent layers
- Upper layers depend on lower layer interfaces
- Lower layers are agnostic to upper layer implementations

### 2. Device Abstraction Pattern

**Benefit**: Hardware portability, easier testing, cleaner application code

**Implementation**:
```
Application Task → Device API → Driver API → HAL → Hardware
```

Example: Reading EPS data
```
read_eps_task() → eps_get_data() → sl_eps2_read_reg() → I2C_receive() → Hardware
```

### 3. Global Data Buffer

**Benefit**: Centralized telemetry access, atomic updates, memory efficiency

**Implementation**: firmware/app/structs/satellite.h:1
```c
extern sat_data_t sat_data_buf;  // Global satellite data buffer
```

Tasks update their respective sections of the buffer, and transmission tasks read from it.

### 4. Event-Driven Task Coordination

**Benefit**: Efficient CPU usage, responsive system behavior

**Implementation**:
- **Event Groups**: `task_startup_status` signals system ready state
- **Queues**: Command queues for inter-task communication
- **Semaphores**: Resource protection and synchronization

Example: firmware/app/tasks/startup.c:1
```c
xEventGroupSetBits(task_startup_status, TASK_STARTUP_DONE);
```

### 5. Configuration-Driven Features

**Benefit**: Easy feature enable/disable, platform variants, debug control

**Implementation**: firmware/config/config.h:1

```c
#define CONFIG_TASK_BEACON_EN           0  // Beacon task disabled
#define CONFIG_TASK_DATA_LOG_EN         1  // Data logging enabled
#define CONFIG_MEDIA_INT_FLASH_EN       1  // Internal flash enabled
```

Conditional compilation eliminates unused code.

## Data Flow Architecture

### Read Path (Sensor → Buffer)

```
Sensors/Modules → Read Tasks → sat_data_buf → Transmission Tasks
```

1. Hardware generates data (temperature, voltage, etc.)
2. Periodic read tasks poll devices
3. Data stored in global buffer (`sat_data_buf`)
4. Transmission tasks access buffer for downlink

### Command Path (Uplink → Execution)

```
Radio → TTC Driver → Read TTC Task → Command Queue → Process TC Task → Action
```

1. Ground station transmits command
2. TTC driver receives packet
3. Read TTC task extracts command
4. Command queued for processing
5. Process TC task validates and executes
6. Feedback transmitted to ground

### Storage Path (Data → Non-Volatile Memory)

```
sat_data_buf → Data Log Task → Media Device → Flash Driver → NOR Flash
```

1. Housekeeping data accumulates in buffer
2. Data Log task triggers (10-minute intervals)
3. Media device formats data
4. Flash driver writes to non-volatile storage

## Memory Architecture

### Flash Memory Layout

**Linker Script**: firmware/lnk_msp430f6659.cmd:1

```
MEMORY
{
    FLASH:  256 KB  (0x4400 - 0x43FFF)
    RAM:    16 KB   (0x1C00 - 0x5BFF)
}

Sections:
- .text:     Code (firmware instructions)
- .const:    Read-only constants
- .data:     Initialized global variables
- .bss:      Uninitialized global variables
- .stack:    Call stack
- .sysmem:   Dynamic memory (FreeRTOS heap: 40 KB)
```

### RAM Usage

**Static Allocation**:
- Global variables (sat_data_buf, device state structures)
- Task stacks (individually allocated per task)

**Dynamic Allocation** (FreeRTOS Heap):
- Task control blocks (TCB)
- Queue storage
- Event groups
- Semaphores
- Dynamically allocated buffers

**Heap Algorithm**: Heap 4 (best-fit with coalescing)

## Initialization Sequence

**Entry Point**: firmware/main.c:1

```
Power On / Reset
    ↓
1. Disable Watchdog
    ↓
2. Clock Initialization (clocks.c)
   - Configure MCLK to 32 MHz
   - Configure ACLK to 32.768 kHz
    ↓
3. System Initialization (system.c)
   - Detect hardware version
   - Determine reset cause
   - Initialize logging
    ↓
4. FreeRTOS Task Creation (tasks.c)
   - Create all enabled tasks
   - Set priorities and stack sizes
    ↓
5. Start FreeRTOS Scheduler
    ↓
6. Startup Task Executes (startup.c)
   - Initialize devices
   - Verify system health
   - Signal startup complete
    ↓
7. Application Tasks Begin
   - Read sensors
   - Process commands
   - Log data
   - Monitor system
```

## Configuration Management

### Master Configuration: firmware/config/config.h:1

**Sections**:

1. **Task Configuration**
   - Enable/disable individual tasks
   - Set task periods and priorities
   - Configure stack sizes

2. **Device Configuration**
   - Enable/disable hardware modules
   - Set device IDs and addresses
   - Configure operational parameters

3. **Memory Configuration**
   - Select storage media (Flash, FRAM)
   - Set memory sizes and partitions

4. **Protocol Configuration**
   - Define packet IDs
   - Set communication parameters
   - Configure timeouts

5. **Debug Configuration**
   - Enable/disable logging
   - Set log verbosity
   - Configure UART debug output

## Error Handling Strategy

### Levels of Error Handling

1. **HAL Level**: Return error codes from hardware operations
2. **Driver Level**: Validate parameters, retry on transient failures
3. **Device Level**: Report device unavailability, fallback to safe state
4. **Application Level**: Log errors, take corrective action, notify ground station

### System Logging

**Location**: firmware/system/sys_log/sys_log.c:1

**Log Levels**:
- `SYS_LOG_NONE`: No logging
- `SYS_LOG_ERROR`: Critical errors only
- `SYS_LOG_WARNING`: Warnings and errors
- `SYS_LOG_INFO`: Informational messages
- `SYS_LOG_DEBUG`: Detailed debug information

**Output**: UART (default 115200 bps)

## Testing Architecture

**Location**: firmware/tests/

**Test Levels**:
- **Unit Tests**: Individual functions and modules
- **Integration Tests**: Inter-layer communication
- **Device Tests**: Hardware driver validation
- **System Tests**: End-to-end functionality

## Build System

**Tools**:
- **IDE**: Code Composer Studio (CCS) v9.0+
- **Compiler**: TI MSP430 GCC or TI Compiler
- **Linker Script**: firmware/lnk_msp430f6659.cmd:1
- **Build Targets**: Debug, Release

## Version Control

**Version Definition**: firmware/version.h:1

```c
#define FIRMWARE_VERSION            "0.10.0"
#define FIRMWARE_STATUS             "Development"
#define FIRMWARE_AUTHOR             "SpaceLab"
#define FIRMWARE_AUTHOR_EMAIL       "..."
```

## Summary

The OBDH2 firmware architecture provides:
- **Modularity**: Clear layer separation for maintainability
- **Portability**: Hardware abstraction for platform independence
- **Scalability**: Easy addition of new devices and tasks
- **Reliability**: FreeRTOS-based real-time scheduling
- **Configurability**: Compile-time feature control
- **Testability**: Layered design enables unit testing

This architecture supports the complex requirements of satellite operations while maintaining code quality and system reliability.
