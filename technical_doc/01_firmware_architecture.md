# OBDH 2.0 Firmware Architecture

## Overview

The OBDH 2.0 firmware is built on a layered architecture that promotes modularity, maintainability, and portability. The design follows best practices for embedded real-time systems with clear separation of concerns between hardware abstraction, device drivers, and application logic.

**Architecture Type:** Layered with RTOS-based multitasking
**Programming Language:** C
**Real-Time Operating System:** FreeRTOS v10.2.1
**Hardware Abstraction:** Texas Instruments MSP430 DriverLib v2.91.11.01
**Build System:** Code Composer Studio (CCS) v9.0+, Makefile support

---

## Architectural Layers

```
┌─────────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                         │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Tasks (app/tasks/)                                  │   │
│  │  - Startup, Watchdog, Sensors, TTC, EPS, Beacon     │   │
│  │  - Process TC, Data Log, Time Control               │   │
│  └──────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Protocol Libraries (app/libs/)                      │   │
│  │  - fsat_pkt: Packet formatting                       │   │
│  │  - fsp: FloripaSat Protocol                          │   │
│  │  - hmac: Authentication/Encryption                   │   │
│  └──────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Data Structures (app/structs/)                      │   │
│  │  - satellite.h: Central data buffer                  │   │
│  │  - obdh_data.h: OBDH telemetry structures           │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                             ↓
┌─────────────────────────────────────────────────────────────┐
│                     DEVICES LAYER                            │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  High-level Device Interfaces (devices/)             │   │
│  │  - eps/, ttc/, antenna/, media/                      │   │
│  │  - temp_sensor/, current_sensor/, voltage_sensor/    │   │
│  │  - leds/, payload/, watchdog/                        │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                             ↓
┌─────────────────────────────────────────────────────────────┐
│                     DRIVERS LAYER                            │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Communication Drivers                               │   │
│  │  - uart/, spi/, i2c/                                 │   │
│  └──────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Peripheral Drivers                                  │   │
│  │  - gpio/, adc/, flash/, rtc/, wdt/                   │   │
│  └──────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  External Device Drivers                             │   │
│  │  - mt25q/ (NOR Flash), cy15x102qn/ (FRAM)            │   │
│  │  - isis_antenna/, sl_eps/, sl_eps2/                  │   │
│  │  - sl_ttc2/, edc/, tps382x/, tca4311a/              │   │
│  │  - phj/, px/                                         │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                             ↓
┌─────────────────────────────────────────────────────────────┐
│            HARDWARE ABSTRACTION LAYER (HAL)                  │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  MSP430 DriverLib v2.91.11.01                        │   │
│  │  - Register-level abstractions                       │   │
│  │  - Clock system, interrupts, low-level peripherals   │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                             ↓
┌─────────────────────────────────────────────────────────────┐
│              FREERTOS KERNEL v10.2.1                         │
│  - Task Scheduler (Preemptive, Priority-based)               │
│  - Synchronization (Mutexes, Semaphores, Event Groups)       │
│  - Memory Management (Heap 4, 40 KB)                         │
│  - Timers, Queues                                            │
└─────────────────────────────────────────────────────────────┘
                             ↓
┌─────────────────────────────────────────────────────────────┐
│                       HARDWARE                               │
│  Texas Instruments MSP430F6659 Microcontroller               │
│  - 16-bit RISC, 32 MHz, 64 KB RAM, 640 KB Flash              │
└─────────────────────────────────────────────────────────────┘
```

---

## Directory Structure

```
/home/user/obdh2/firmware/
│
├── app/                              # Application Layer
│   ├── libs/                         # Protocol and utility libraries
│   │   ├── fsat_pkt/                # FloripaSat packet handler
│   │   │   ├── fsat_pkt.c/h         # Packet encode/decode
│   │   │   └── README.md
│   │   ├── fsp/                     # FloripaSat Protocol
│   │   │   ├── fsp.c/h              # Protocol implementation
│   │   │   └── README.md
│   │   └── hmac/                    # HMAC-SHA cryptography
│   │       ├── hmac.c/h             # Authentication functions
│   │       └── README.md
│   │
│   ├── structs/                     # Data structure definitions
│   │   ├── obdh_data.h              # OBDH-specific telemetry
│   │   ├── satellite.h              # Main satellite data buffer
│   │   └── README.md
│   │
│   └── tasks/                       # FreeRTOS tasks
│       ├── startup_task.c/h         # System initialization
│       ├── watchdog_reset_task.c/h  # Watchdog management
│       ├── heartbeat_task.c/h       # Status indicator
│       ├── system_reset_task.c/h    # Controlled reset
│       ├── read_sensors_task.c/h    # Sensor polling
│       ├── time_control_task.c/h    # Time synchronization
│       ├── read_eps_task.c/h        # EPS telemetry
│       ├── read_ttc_task.c/h        # TTC telemetry
│       ├── read_antenna_task.c/h    # Antenna status
│       ├── read_edc_task.c/h        # Payload data
│       ├── data_log_task.c/h        # Storage management
│       ├── process_tc_task.c/h      # Telecommand processing
│       ├── beacon_task.c/h          # Beacon transmission
│       ├── antenna_deploy_task.c/h  # Antenna deployment
│       └── README.md
│
├── devices/                          # Device Abstraction Layer
│   ├── eps/                         # Electrical Power System
│   ├── ttc/                         # Telemetry/Telecommand
│   ├── antenna/                     # ISIS Antenna
│   ├── media/                       # Storage devices
│   ├── temp_sensor/                 # Temperature sensors
│   ├── current_sensor/              # Current measurement
│   ├── voltage_sensor/              # Voltage measurement
│   ├── leds/                        # Status LEDs
│   ├── payload/                     # Payload interfaces
│   └── watchdog/                    # Watchdog timer
│
├── drivers/                          # Driver Layer
│   ├── Communication Peripherals
│   │   ├── uart/                    # UART (3 ports)
│   │   ├── spi/                     # SPI (6 ports)
│   │   └── i2c/                     # I2C (3 ports)
│   │
│   ├── MCU Peripherals
│   │   ├── gpio/                    # GPIO (47 pins)
│   │   ├── adc/                     # ADC module
│   │   ├── flash/                   # Internal flash
│   │   ├── rtc/                     # Real-time clock
│   │   └── wdt/                     # Watchdog timer
│   │
│   └── External Devices
│       ├── mt25q/                   # MT25Q NOR flash (256MB)
│       ├── cy15x102qn/              # FRAM (256KB)
│       ├── isis_antenna/            # ISIS antenna deployer
│       ├── sl_eps/                  # SpaceLab EPS v1
│       ├── sl_eps2/                 # SpaceLab EPS v2
│       ├── sl_ttc2/                 # SpaceLab TTC v2
│       ├── edc/                     # Earth observation module
│       ├── tps382x/                 # Voltage monitor
│       ├── tca4311a/                # I2C level shifter
│       ├── phj/                     # PHJ connector interface
│       └── px/                      # PX CubeSat adapter
│
├── system/                           # System Services
│   ├── clocks.h                     # Clock configuration
│   ├── system.h/c                   # System control functions
│   └── sys_log/                     # Debug logging system
│       ├── sys_log.c/h
│       └── README.md
│
├── hal/                             # Hardware Abstraction Layer
│   └── (MSP430 DriverLib headers and source files)
│
├── freertos/                        # FreeRTOS Kernel
│   ├── include/                     # FreeRTOS headers
│   │   ├── FreeRTOS.h
│   │   ├── task.h
│   │   ├── queue.h
│   │   ├── semphr.h
│   │   └── ...
│   └── portable/                    # Port-specific code
│       └── CCS/                     # Code Composer Studio port
│           └── MSP430X/             # MSP430X architecture
│
├── config/                          # Configuration Files
│   ├── config.h                     # Feature flags, packet IDs
│   ├── FreeRTOSConfig.h             # RTOS configuration
│   ├── keys.h                       # Cryptographic keys
│   └── radio_config_Si4463.h        # Radio configuration
│
├── tests/                           # Unit Tests
│   └── ...
│
├── main.c                           # Entry point
├── lnk_msp430f6659.cmd              # Linker script
├── Makefile                         # Build configuration
└── .ccsproject / .cproject          # CCS IDE project files
```

**File Statistics:**
- **Total C/H files:** 343
- **C implementation files:** 178
- **Header files:** 165

---

## Layer Descriptions

### 1. Application Layer

**Purpose:** Implements mission-specific logic and high-level functionality

**Components:**

#### Tasks (`app/tasks/`)
- **Type:** FreeRTOS tasks (threads)
- **Count:** 12-14 tasks (configurable)
- **Responsibilities:**
  - System initialization (Startup)
  - Periodic operations (Watchdog, Heartbeat, Sensors)
  - Subsystem communication (EPS, TTC, Antenna, EDC)
  - Command processing (Process TC)
  - Data management (Data Log, Beacon)
  - Time management (Time Control)

**Key Files:**
- `startup_task.c`: 350 bytes stack, priority 5, one-shot initialization
- `process_tc_task.c`: 500 bytes stack, priority 4, telecommand handler
- `beacon_task.c`: 1000 bytes stack, priority 5, periodic beacon

#### Protocol Libraries (`app/libs/`)
- **fsat_pkt:** Packet encoding/decoding for FloripaSat custom format
  - Maximum 220-byte payload
  - Header/footer/CRC management
- **fsp:** FloripaSat Protocol state machine
  - Reliable packet transmission
  - ACK/NACK handling
- **hmac:** HMAC-SHA256 authentication
  - Telecommand validation
  - Secure communication

#### Data Structures (`app/structs/`)
- **satellite.h:** Defines `sat_data_t` - central data repository
- **obdh_data.h:** OBDH-specific telemetry structure
- Shared across all tasks via global buffer `sat_data_buf`

---

### 2. Devices Layer

**Purpose:** High-level device abstractions hiding driver complexity

**Design Pattern:** Each device provides:
- Initialization function: `device_init()`
- Configuration function: `device_config()`
- Read/Write functions: `device_read()`, `device_write()`
- Status query: `device_get_status()`

**Examples:**

#### EPS Device (`devices/eps/`)
```c
int eps_init(eps_config_t config);
int eps_read_telemetry(eps_data_t *data);
int eps_enable_module(uint8_t module_id);
```

#### TTC Device (`devices/ttc/`)
```c
int ttc_init(ttc_port_t port, ttc_config_t config);
int ttc_send_packet(ttc_port_t port, uint8_t *data, uint16_t len);
int ttc_receive_packet(ttc_port_t port, uint8_t *buffer, uint16_t *len);
int ttc_read_telemetry(ttc_port_t port, ttc_data_t *data);
```

#### Media Device (`devices/media/`)
- Abstraction over multiple storage types: INT_FLASH, FRAM, NOR
```c
int media_init(media_type_t type);
int media_write(media_type_t type, uint32_t addr, uint8_t *data, uint32_t len);
int media_read(media_type_t type, uint32_t addr, uint8_t *buffer, uint32_t len);
```

**Benefits:**
- Tasks don't need to know hardware details
- Easy to swap hardware implementations
- Consistent API across devices

---

### 3. Drivers Layer

**Purpose:** Low-level hardware control and peripheral management

#### Communication Drivers

**UART Driver (`drivers/uart/`):**
- 3 ports (eUSCI_A modules)
- Configurable baud rate (default 115200 bps)
- Interrupt-driven or polling modes
- TX/RX buffer management

**SPI Driver (`drivers/spi/`):**
- 6 ports (eUSCI_A0-A2, eUSCI_B0-B2)
- Master/Slave modes
- Configurable clock speed
- Multi-slave chip select management

**I2C Driver (`drivers/i2c/`):**
- 3 ports (eUSCI_B modules)
- Master mode (100/400 kHz)
- Multi-device bus support
- Error handling and recovery

#### Peripheral Drivers

**GPIO Driver (`drivers/gpio/`):**
- Abstraction of 47 GPIO pins across ports P1-P6
- Input/output configuration
- Interrupt support
- Pull-up/pull-down resistors

**ADC Driver (`drivers/adc/`):**
- 12-bit ADC (ADC12_A module)
- Multiple channel support
- Reference voltage configuration
- Single-shot or continuous mode

**Flash Driver (`drivers/flash/`):**
- Internal MSP430 flash operations
- Segment erase and write
- Info memory access (INFOA-INFOD)
- Write protection

**RTC Driver (`drivers/rtc/`):**
- 32.768 kHz crystal-based timekeeping
- Calendar mode (seconds, minutes, hours, date)
- Alarm functionality
- Interrupt generation

**WDT Driver (`drivers/wdt/`):**
- Watchdog timer configuration
- Periodic reset generation
- Timeout interval setting

#### External Device Drivers

**MT25Q NOR Flash (`drivers/mt25q/`):**
- 256 MB serial NOR flash
- SPI interface (up to 133 MHz, typically 1 MHz)
- Sector/block erase operations
- Page programming (256 bytes)
- Quad SPI support (future enhancement)

**CY15x102QN FRAM (`drivers/cy15x102qn/`):**
- 256 KB FRAM
- SPI interface
- Byte-level read/write
- Unlimited write endurance
- Fast write (no erase needed)

**ISIS Antenna (`drivers/isis_antenna/`):**
- I2C interface
- Antenna deployment commands
- Status readback
- Temperature monitoring

**SpaceLab EPS v2 (`drivers/sl_eps2/`):**
- I2C communication
- Telemetry request commands
- Power module control

**SpaceLab TTC v2 (`drivers/sl_ttc2/`):**
- I2C communication
- Packet transmission/reception
- Configuration management
- RSSI and status readback

---

### 4. Hardware Abstraction Layer (HAL)

**Purpose:** Vendor-provided low-level hardware access

**MSP430 DriverLib v2.91.11.01:**
- Texas Instruments official library
- Register-level abstractions
- Peripheral initialization helpers
- Clock system configuration
- Interrupt management

**Modules Used:**
- `driverlib.h` - Master include
- Clock System (UCS/FLL)
- eUSCI (UART/SPI/I2C)
- GPIO (Digital I/O)
- ADC12_A (Analog-to-Digital)
- Flash Memory Controller
- Real-Time Clock (RTC_B)
- Watchdog Timer (WDT_A)
- DMA (Direct Memory Access)

**Benefits:**
- Hardware portability across MSP430 family
- Battle-tested code from TI
- Reduced development time

---

### 5. FreeRTOS Kernel

**Purpose:** Real-time task scheduling and synchronization

**Version:** 10.2.1

**Configuration (FreeRTOSConfig.h):**
```c
configUSE_PREEMPTION              = 1     // Preemptive scheduling
configCPU_CLOCK_HZ                = 32000000  // 32 MHz
configTICK_RATE_HZ                = 1000  // 1 ms tick
configMAX_PRIORITIES              = 5     // Priority levels 0-4
configMINIMAL_STACK_SIZE          = 128   // Bytes
configTOTAL_HEAP_SIZE             = 40960 // 40 KB
configUSE_MUTEXES                 = 1
configUSE_COUNTING_SEMAPHORES     = 1
configUSE_TIMERS                  = 1
configUSE_EVENT_GROUPS            = 1
```

**Scheduler:**
- Preemptive, priority-based
- Tick interrupt at 1 kHz (1 ms resolution)
- Context switching optimized for MSP430X

**Synchronization Primitives:**
- **Mutexes:** Protect shared resources (e.g., I2C bus access)
- **Semaphores:** Binary and counting for signaling
- **Event Groups:** Task synchronization (e.g., `task_startup_status`)
- **Queues:** Inter-task message passing
- **Timers:** Software timers for delayed/periodic operations

**Memory Management:**
- Heap 4 algorithm (coalescence support)
- 40 KB heap allocated from MSP430 RAM
- Static allocation for tasks and RTOS structures

---

## Firmware Initialization Flow

```
Power-On / Reset
    ↓
main() Entry Point
    ↓
1. Disable Watchdog (temporary)
    ↓
2. System Clock Initialization
   - MCLK: 32 MHz (DCO + FLL)
   - SMCLK: 32 MHz
   - ACLK: 32.768 kHz (XT1)
    ↓
3. Hardware Initialization
   - GPIO default states
   - Peripheral power-up
    ↓
4. FreeRTOS Task Creation
   - Create all enabled tasks
   - Allocate stacks
   - Set priorities
    ↓
5. Start FreeRTOS Scheduler
   - Enable interrupts
   - Begin task execution
    ↓
┌─────────────────────────────┐
│   Startup Task Executes     │
│   (Highest Priority: 5)     │
└─────────────────────────────┘
    ↓
6. Device Initialization (in Startup Task)
   - Enable Watchdog
   - Initialize Debug UART
   - Initialize SPI/I2C buses
   - Initialize Storage (FRAM, NOR Flash)
   - Initialize EPS interface
   - Initialize TTC interfaces
   - Initialize Antenna interface
   - Initialize Payloads
   - Load configuration from FRAM
    ↓
7. System Self-Test
   - Verify device connectivity
   - Check reset cause
   - Increment boot counter
    ↓
8. Set Event Flag: Startup Complete
   - Set task_startup_status event group
   - Signal to other tasks
    ↓
9. Startup Task Suspends
    ↓
┌─────────────────────────────┐
│  Other Tasks Begin Normal   │
│  Periodic Execution          │
└─────────────────────────────┘
```

---

## Key Architectural Patterns

### 1. Central Data Repository Pattern

**Implementation:** Global `sat_data_buf` of type `sat_data_t`

**Structure:**
```c
sat_data_t sat_data_buf = {
    .obdh = { .timestamp, .data },
    .eps = { .timestamp, .data },
    .ttc[0] = { .timestamp, .data },
    .ttc[1] = { .timestamp, .data },
    .antenna = { .timestamp, .data },
    .payload[0..3] = { .timestamp, .enabled, .data }
};
```

**Benefits:**
- Single source of truth for satellite state
- Easy snapshot for telemetry packets
- Simplified task communication

**Access Control:**
- Mutex protection for concurrent access
- Read tasks update their respective sections
- Beacon/Downlink tasks read consolidated data

### 2. Task-Based Decomposition

**Pattern:** Each major function is a separate FreeRTOS task

**Advantages:**
- Independent execution and timing
- Clear responsibility boundaries
- Easy to enable/disable features via config
- Parallel execution on single core via time-slicing

**Example:** Read EPS Task
- Runs every 60 seconds
- Requests telemetry from EPS via I2C
- Updates `sat_data_buf.eps`
- Suspends until next period

### 3. Layered Driver Model

**Pattern:** Multiple abstraction levels for hardware

**Levels:**
1. **HAL:** Register operations (`GPIO_setOutputHighOnPin()`)
2. **Driver:** Protocol logic (`i2c_write_bytes()`)
3. **Device:** Application interface (`eps_read_telemetry()`)
4. **Task:** Mission logic (collect and store EPS data)

**Example Data Path:**
```
Beacon Task
    ↓ Call
ttc_send_packet(ttc_port, packet_data)
    ↓ Call
sl_ttc2_send(ttc_i2c_addr, data)
    ↓ Call
i2c_write(I2C_PORT_0, addr, buffer, len)
    ↓ Call
EUSCI_B_I2C_masterSendMultiByteStart(...)
    ↓ Register access
HWREG16(UCBxTXBUF) = data
```

### 4. Configuration-Driven Compilation

**Pattern:** `config/config.h` feature flags

**Examples:**
```c
#define TASK_STARTUP_ENABLED            1
#define TASK_WATCHDOG_RESET_ENABLED     1
#define TASK_BEACON_ENABLED             1
#define DEVICE_MEDIA_INT_FLASH_ENABLED  1
#define DEVICE_MEDIA_FRAM_ENABLED       1
#define DEVICE_MEDIA_NOR_ENABLED        1
```

**Benefits:**
- Compile out unused features (reduce code size)
- Easy mission customization
- Test individual subsystems

---

## Memory Map

### MSP430F6659 Memory Layout

```
┌────────────────────────────────────┐ 0xFFFF
│  Interrupt Vectors                 │
├────────────────────────────────────┤ 0xFF80
│  Info Memory D                     │
├────────────────────────────────────┤ 0xFF00
│  Info Memory C                     │
├────────────────────────────────────┤ 0xFF80
│  Info Memory B                     │
├────────────────────────────────────┤ 0xFF00
│  Info Memory A                     │
├────────────────────────────────────┤ 0xFB00
│  RAM (Extended)                    │ 0xF0000 - 0xFBFFF
│  (48 KB)                           │
├────────────────────────────────────┤ 0xF0000
│         ...                        │
├────────────────────────────────────┤ 0x87FFF
│  Flash (Extended)                  │ 0x10000 - 0x87FFF
│  (480 KB)                          │
├────────────────────────────────────┤ 0x10000
│         ...                        │
├────────────────────────────────────┤ 0xFFFF
│  Flash (Main)                      │ 0x8000 - 0xFF7F
│  (32 KB - code and constants)     │
├────────────────────────────────────┤ 0x8000
│  RAM (Main)                        │ 0x2400 - 0x6400
│  (16 KB)                           │
├────────────────────────────────────┤ 0x2400
│  USB RAM                           │ 0x1C00 - 0x23FF
├────────────────────────────────────┤ 0x1C00
│  Peripherals                       │ 0x0100 - 0x1BFF
├────────────────────────────────────┤ 0x0100
│  Special Function Registers        │ 0x0000 - 0x00FF
└────────────────────────────────────┘ 0x0000
```

**RAM Usage:**
- FreeRTOS Heap: 40 KB (configurable in FreeRTOSConfig.h)
- Task Stacks: ~4 KB total for 12 tasks
- Global Variables: `sat_data_buf`, configuration, etc.
- Remaining: Available for dynamic allocation

**Flash Usage:**
- Application code: Main firmware logic
- FreeRTOS kernel: ~10-20 KB
- Driver libraries: MSP430 DriverLib, device drivers
- Constants: Packet IDs, configuration defaults
- Cryptographic keys

---

## Build System

### Code Composer Studio (CCS)

**Project Files:**
- `.ccsproject` - CCS project metadata
- `.cproject` - Eclipse CDT project configuration
- `lnk_msp430f6659.cmd` - Linker command file (memory sections)

**Build Steps:**
1. Preprocessor: Expand macros, include headers
2. Compiler: C to MSP430 assembly
3. Assembler: Assembly to object code
4. Linker: Combine objects, resolve symbols, generate .out executable
5. Post-build: Generate .hex/.bin for programming

**Optimization Levels:**
- Debug: -O0 (no optimization, full debug symbols)
- Release: -O2 or -O3 (size/speed optimization)

### Makefile

**Alternative Build Method:**
```bash
make            # Build all
make clean      # Remove build artifacts
make flash      # Program device (requires MSP-FET)
```

**Dependencies:**
- `msp430-gcc` - GCC toolchain for MSP430
- `mspdebug` - Debugger/programmer interface

---

## Development Workflow

### 1. Code Development
- Edit source files in CCS or text editor
- Follow coding standards (see repository guidelines)

### 2. Build and Test
- Build in CCS: `Ctrl+B`
- Fix compilation errors
- Review warnings

### 3. Programming
- Connect MSP-FET programmer to JTAG header
- Flash device: `Run → Debug` in CCS
- Verify upload success

### 4. Debugging
- Set breakpoints in tasks
- Inspect variables (`sat_data_buf`, task states)
- Monitor UART debug output (115200 bps)
- Check FreeRTOS task statistics

### 5. Version Control
- Commit changes to Git
- Follow branch naming conventions
- Create pull requests for review

---

## Design Principles

1. **Modularity:** Clear interfaces between layers, easy to replace components
2. **Configurability:** Feature flags for mission customization
3. **Robustness:** Watchdog, reset tracking, error handling
4. **Efficiency:** Optimized for low-power embedded systems
5. **Maintainability:** Well-documented, consistent coding style
6. **Testability:** Unit tests, mockups, hardware-in-the-loop support

---

## Related Documentation

- `00_functionalities_of_system.md` - System overview and capabilities
- `02_datapath.md` - Data structures and flow
- `03_interfaces.md` - Communication protocols
- `04_tasks.md` - Detailed task descriptions
- `05_pin_mapping.md` - Hardware pin assignments
- `06_hardware_components_and_architecture.md` - Hardware details
