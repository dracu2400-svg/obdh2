# OBDH2 FreeRTOS Tasks

## Overview

The OBDH2 firmware implements a task-based architecture using FreeRTOS v10.2.1. This document provides detailed specifications for each task, including responsibilities, timing, priorities, and interactions.

**FreeRTOS Version**: 10.2.1
**Total Tasks**: 15+ tasks (configurable)
**Scheduler**: Preemptive priority-based
**Tick Rate**: 1000 Hz (1 ms per tick)
**Total Heap**: 40 KB

## Task Management

**Location**: firmware/app/tasks/tasks.c:1
**Configuration**: firmware/config/config.h:1
**FreeRTOS Config**: firmware/config/FreeRTOSConfig.h:1

### Task Registration

All tasks are registered and created during system initialization:

```c
void tasks_create(void) {
    // Create all enabled tasks based on config.h flags
    #if CONFIG_TASK_STARTUP_EN
        xTaskCreate(task_startup, ...);
    #endif

    #if CONFIG_TASK_BEACON_EN
        xTaskCreate(task_beacon, ...);
    #endif

    // ... additional tasks
}
```

## Priority Levels

FreeRTOS supports 5 priority levels (0-4):

| Priority | Level | Usage |
|----------|-------|-------|
| 0 | Idle | FreeRTOS idle task |
| 1 | Low | Background operations |
| 2 | Below Normal | Lower priority reads |
| 3 | Normal | Standard tasks (sensors, logging) |
| 4 | High | Critical tasks (commands, watchdog, TTC) |
| 5 | Very High | System startup (temporary) |

## Task List Overview

| Task | Priority | Period | Stack | Status | Purpose |
|------|----------|--------|-------|--------|---------|
| Startup | 5 | One-shot | 350 B | Enabled | System initialization |
| Watchdog Reset | 4 | 500 ms | 200 B | Enabled | Prevent system reset |
| Heartbeat | 4 | 1000 ms | 200 B | Enabled | Status LED |
| System Reset | 4 | Continuous | 200 B | Enabled | Monitor reset conditions |
| Time Control | 3 | 1000 ms | 200 B | Enabled | System time counter |
| Read Sensors | 3 | 1000 ms | 250 B | Enabled | OBDH sensor acquisition |
| Read EPS | 3 | 1000 ms | 250 B | Enabled | EPS telemetry |
| Read TTC | 4 | 100 ms | 200 B | Enabled | Radio monitoring |
| Read Antenna | 2 | 1000 ms | 200 B | Enabled | Antenna status |
| Read EDC | 3 | 1000 ms | 250 B | Config | Payload data |
| Beacon | 3 | 60000 ms | 250 B | Disabled | Periodic telemetry broadcast |
| Data Log | 3 | 600000 ms | 225 B | Enabled | Data archival |
| Process TC | 4 | 5 ms | 500 B | Enabled | Telecommand processing |
| Antenna Deployment | 3 | Event | 250 B | Disabled | Deployment sequence |
| Housekeeping | 3 | Variable | 250 B | Enabled | Mode management |

## Detailed Task Specifications

### 1. Startup Task

**Location**: firmware/app/tasks/startup.c:1

**Purpose**: One-time system initialization and verification

**Configuration**:
- **Priority**: 5 (Highest)
- **Stack Size**: 350 bytes
- **Execution**: Single-shot (deleted after completion)
- **Status**: Enabled

**Responsibilities**:
1. Initialize all hardware devices
2. Verify hardware version
3. Check reset cause
4. Initialize media subsystem (Flash, FRAM)
5. Initialize sensors
6. Initialize communication modules (EPS, TTC, Antenna)
7. Report clock frequencies
8. Signal startup completion via event group
9. Self-delete task

**Execution Flow**:
```
┌─────────────────────────────────────────────┐
│ Startup Task (One-shot, Priority 5)        │
├─────────────────────────────────────────────┤
│                                             │
│  1. Wait for scheduler to start            │
│     └─> vTaskDelay(100ms)                  │
│                                             │
│  2. Log startup message                     │
│     └─> sys_log(INFO, "Starting OBDH2")    │
│                                             │
│  3. Detect hardware version                 │
│     └─> system_get_hw_version()            │
│                                             │
│  4. Detect reset cause                      │
│     └─> system_get_reset_cause()           │
│                                             │
│  5. Initialize devices                      │
│     └─> media_init()                       │
│     └─> leds_init()                        │
│     └─> temp_sensor_init()                 │
│     └─> voltage_sensor_init()              │
│     └─> current_sensor_init()              │
│     └─> eps_init()                         │
│     └─> ttc_init(TTC_0)                    │
│     └─> ttc_init(TTC_1)                    │
│     └─> antenna_init()                     │
│                                             │
│  6. Report system clocks                    │
│     └─> Log MCLK, SMCLK, ACLK              │
│                                             │
│  7. Set startup complete flag               │
│     └─> xEventGroupSetBits(                │
│             task_startup_status,           │
│             TASK_STARTUP_DONE)             │
│                                             │
│  8. Delete self                             │
│     └─> vTaskDelete(NULL)                  │
│                                             │
└─────────────────────────────────────────────┘
```

**Event Signaling**:
- Sets `TASK_STARTUP_DONE` bit in `task_startup_status` event group
- Other tasks wait for this event before beginning operations

### 2. Watchdog Reset Task

**Location**: firmware/app/tasks/watchdog_reset.c:1

**Purpose**: Periodically refresh external watchdog timer to prevent system reset

**Configuration**:
- **Priority**: 4 (High)
- **Stack Size**: 200 bytes
- **Period**: 500 ms
- **Status**: Enabled

**Responsibilities**:
1. Wait for startup completion
2. Refresh TPS382x watchdog timer every 500 ms
3. Prevent watchdog timeout and forced reset

**Execution Flow**:
```
┌─────────────────────────────────────────────┐
│ Watchdog Reset Task (500 ms, Priority 4)   │
├─────────────────────────────────────────────┤
│                                             │
│  Loop every 500 ms:                         │
│                                             │
│  1. Wait for startup completion             │
│     └─> xEventGroupWaitBits(               │
│             task_startup_status,           │
│             TASK_STARTUP_DONE, ...)        │
│                                             │
│  2. Refresh watchdog                        │
│     └─> watchdog_reset()                   │
│         └─> GPIO toggle on WDT pin         │
│                                             │
│  3. Sleep until next cycle                  │
│     └─> vTaskDelay(500 ms)                 │
│                                             │
└─────────────────────────────────────────────┘
```

**Critical Importance**:
- Failure to refresh watchdog causes system reset
- High priority ensures timely execution
- Hardware watchdog provides ultimate fault recovery

### 3. Heartbeat Task

**Location**: firmware/app/tasks/heartbeat.c:1

**Purpose**: Toggle LED for visual system-alive indication

**Configuration**:
- **Priority**: 4 (High)
- **Stack Size**: 200 bytes
- **Period**: 1000 ms (1 Hz)
- **Status**: Enabled

**Responsibilities**:
1. Wait for startup completion
2. Toggle heartbeat LED every second
3. Provide visual confirmation of system operation

**Execution Flow**:
```
┌─────────────────────────────────────────────┐
│ Heartbeat Task (1000 ms, Priority 4)       │
├─────────────────────────────────────────────┤
│                                             │
│  Loop every 1 second:                       │
│                                             │
│  1. Wait for startup completion             │
│     └─> xEventGroupWaitBits(...)           │
│                                             │
│  2. Toggle LED                              │
│     └─> leds_toggle(LED_HEARTBEAT)         │
│                                             │
│  3. Sleep until next cycle                  │
│     └─> vTaskDelay(1000 ms)                │
│                                             │
└─────────────────────────────────────────────┘
```

### 4. System Reset Task

**Location**: firmware/app/tasks/system_reset.c:1

**Purpose**: Monitor for reset conditions and execute controlled system reset

**Configuration**:
- **Priority**: 4 (High)
- **Stack Size**: 200 bytes
- **Period**: Continuous monitoring
- **Status**: Enabled

**Responsibilities**:
1. Monitor reset request flag
2. Execute controlled system shutdown
3. Trigger watchdog reset or software reset

**Execution Flow**:
```
┌─────────────────────────────────────────────┐
│ System Reset Task (Continuous, Priority 4) │
├─────────────────────────────────────────────┤
│                                             │
│  Continuous loop:                           │
│                                             │
│  1. Check reset request flag                │
│     └─> if (system_reset_requested)        │
│                                             │
│  2. Perform safe shutdown                   │
│     └─> Close open files                   │
│     └─> Flush buffers                      │
│     └─> Save state to FRAM                 │
│                                             │
│  3. Execute reset                           │
│     └─> Trigger watchdog reset             │
│         OR                                  │
│     └─> Software reset (PMMCTL0)           │
│                                             │
│  4. Sleep briefly                           │
│     └─> vTaskDelay(100 ms)                 │
│                                             │
└─────────────────────────────────────────────┘
```

**Reset Sources**:
- Telecommand (Force Reset 0x4A)
- Critical error detection
- Commanded by ground station

### 5. Time Control Task

**Location**: firmware/app/tasks/time_control.c:1

**Purpose**: Maintain system time counter

**Configuration**:
- **Priority**: 3 (Normal)
- **Stack Size**: 200 bytes
- **Period**: 1000 ms
- **Status**: Enabled

**Responsibilities**:
1. Increment system time counter every second
2. Provide timestamp for telemetry and logs
3. Update `sat_data_buf.obdh.system_time`

**Execution Flow**:
```
┌─────────────────────────────────────────────┐
│ Time Control Task (1000 ms, Priority 3)    │
├─────────────────────────────────────────────┤
│                                             │
│  Loop every 1 second:                       │
│                                             │
│  1. Wait for startup completion             │
│     └─> xEventGroupWaitBits(...)           │
│                                             │
│  2. Increment system time                   │
│     └─> system_time++                      │
│     └─> sat_data_buf.obdh.system_time =   │
│             system_time                    │
│                                             │
│  3. Sleep until next cycle                  │
│     └─> vTaskDelay(1000 ms)                │
│                                             │
└─────────────────────────────────────────────┘
```

**Time Format**: Seconds since boot (uint32_t, wraps after 136 years)

### 6. Read Sensors Task

**Location**: firmware/app/tasks/read_sensors.c:1

**Purpose**: Acquire OBDH internal sensor data

**Configuration**:
- **Priority**: 3 (Normal)
- **Stack Size**: 250 bytes
- **Period**: 1000 ms
- **Status**: Enabled

**Responsibilities**:
1. Read MCU temperature sensor (ADC)
2. Read input voltage sensor (ADC)
3. Read input current sensor (ADC)
4. Update `sat_data_buf.obdh` structure

**Execution Flow**:
```
┌─────────────────────────────────────────────┐
│ Read Sensors Task (1000 ms, Priority 3)    │
├─────────────────────────────────────────────┤
│                                             │
│  Loop every 1 second:                       │
│                                             │
│  1. Wait for startup completion             │
│     └─> xEventGroupWaitBits(...)           │
│                                             │
│  2. Read temperature sensor                 │
│     └─> temp_sensor_get_data(&temp_k)      │
│     └─> sat_data_buf.obdh.temperature =    │
│             temp_k                         │
│                                             │
│  3. Read voltage sensor                     │
│     └─> voltage_sensor_get_data(&volt_mv)  │
│     └─> sat_data_buf.obdh.voltage =        │
│             volt_mv                        │
│                                             │
│  4. Read current sensor                     │
│     └─> current_sensor_get_data(&curr_ma)  │
│     └─> sat_data_buf.obdh.current =        │
│             curr_ma                        │
│                                             │
│  5. Log sensor data (optional)              │
│     └─> sys_log(DEBUG, "Sensors read")     │
│                                             │
│  6. Sleep until next cycle                  │
│     └─> vTaskDelay(1000 ms)                │
│                                             │
└─────────────────────────────────────────────┘
```

**Data Types**:
- Temperature: Kelvin (uint16_t)
- Voltage: Millivolts (uint16_t)
- Current: Milliamperes (uint16_t)

### 7. Read EPS Task

**Location**: firmware/app/tasks/read_eps.c:1

**Purpose**: Acquire EPS (power subsystem) telemetry

**Configuration**:
- **Priority**: 3 (Normal)
- **Stack Size**: 250 bytes
- **Period**: 1000 ms
- **Status**: Enabled

**Responsibilities**:
1. Read EPS telemetry via I2C
2. Extract battery, solar panel, and power data
3. Update `sat_data_buf.eps` structure
4. Handle communication errors

**Execution Flow**:
```
┌─────────────────────────────────────────────┐
│ Read EPS Task (1000 ms, Priority 3)        │
├─────────────────────────────────────────────┤
│                                             │
│  Loop every 1 second:                       │
│                                             │
│  1. Wait for startup completion             │
│     └─> xEventGroupWaitBits(...)           │
│                                             │
│  2. Read EPS data                           │
│     └─> eps_get_data(&eps_telemetry)       │
│         └─> I2C read from EPS (0x36)       │
│         └─> Parse register data            │
│                                             │
│  3. Update global buffer                    │
│     └─> sat_data_buf.eps = eps_telemetry   │
│                                             │
│  4. Handle errors                           │
│     └─> if (error)                         │
│         └─> sys_log(ERROR, "EPS comm")     │
│         └─> Increment error counter        │
│                                             │
│  5. Sleep until next cycle                  │
│     └─> vTaskDelay(1000 ms)                │
│                                             │
└─────────────────────────────────────────────┘
```

**EPS Telemetry**:
- Battery voltage/current
- Solar panel voltages/currents (3 axes)
- EPS temperature
- Module status flags
- Heater status

### 8. Read TTC Task

**Location**: firmware/app/tasks/read_ttc.c:1

**Purpose**: Monitor TTC radio modules and receive uplink commands

**Configuration**:
- **Priority**: 4 (High)
- **Stack Size**: 200 bytes
- **Period**: 100 ms (10 Hz, high frequency)
- **Status**: Enabled

**Responsibilities**:
1. Poll both TTC modules (TTC_0, TTC_1)
2. Read radio housekeeping data (RSSI, temperature)
3. Check for received uplink packets
4. Queue telecommands for processing
5. Update `sat_data_buf.ttc_0` and `sat_data_buf.ttc_1`

**Execution Flow**:
```
┌─────────────────────────────────────────────┐
│ Read TTC Task (100 ms, Priority 4)         │
├─────────────────────────────────────────────┤
│                                             │
│  Loop every 100 ms:                         │
│                                             │
│  1. Wait for startup completion             │
│     └─> xEventGroupWaitBits(...)           │
│                                             │
│  2. For each TTC module (0, 1):             │
│                                             │
│     a. Read housekeeping data               │
│        └─> ttc_get_data(TTC_x, &data)      │
│            └─> SPI transaction             │
│            └─> Read RSSI, temp, counters   │
│        └─> sat_data_buf.ttc_x = data       │
│                                             │
│     b. Check for received packets           │
│        └─> ttc_recv(TTC_x, rx_buf, &len)   │
│            └─> Read RX buffer              │
│                                             │
│     c. If packet received:                  │
│        └─> Validate packet format          │
│        └─> Extract telecommand             │
│        └─> xQueueSend(tc_queue, &cmd)      │
│                                             │
│  3. Sleep until next cycle                  │
│     └─> vTaskDelay(100 ms)                 │
│                                             │
└─────────────────────────────────────────────┘
```

**High Frequency Justification**:
- Responsive command reception (100 ms latency)
- Frequent RSSI monitoring
- Dual radio redundancy management

### 9. Read Antenna Task

**Location**: firmware/app/tasks/read_antenna.c:1

**Purpose**: Monitor antenna deployment status

**Configuration**:
- **Priority**: 2 (Below Normal)
- **Stack Size**: 200 bytes
- **Period**: 1000 ms
- **Status**: Enabled

**Responsibilities**:
1. Read antenna deployment status via I2C
2. Read antenna temperature
3. Update `sat_data_buf.antenna`
4. Monitor deployment counter

**Execution Flow**:
```
┌─────────────────────────────────────────────┐
│ Read Antenna Task (1000 ms, Priority 2)    │
├─────────────────────────────────────────────┤
│                                             │
│  Loop every 1 second:                       │
│                                             │
│  1. Wait for startup completion             │
│     └─> xEventGroupWaitBits(...)           │
│                                             │
│  2. Read antenna data                       │
│     └─> antenna_get_data(&ant_data)        │
│         └─> I2C read from ISIS Antenna     │
│         └─> Parse deployment status        │
│         └─> Read temperature               │
│                                             │
│  3. Update global buffer                    │
│     └─> sat_data_buf.antenna = ant_data    │
│                                             │
│  4. Sleep until next cycle                  │
│     └─> vTaskDelay(1000 ms)                │
│                                             │
└─────────────────────────────────────────────┘
```

### 10. Read EDC (Payload) Task

**Location**: firmware/app/tasks/read_edc.c:1

**Purpose**: Acquire payload telemetry data

**Configuration**:
- **Priority**: 3 (Normal)
- **Stack Size**: 250 bytes
- **Period**: 1000 ms
- **Status**: Configurable (enabled per payload)

**Responsibilities**:
1. Read payload data from EDC-1, EDC-2, Payload-X, HARSH
2. Each payload provides up to 220 bytes
3. Update respective payload sections in `sat_data_buf`

**Execution Flow**:
```
┌─────────────────────────────────────────────┐
│ Read EDC Task (1000 ms, Priority 3)        │
├─────────────────────────────────────────────┤
│                                             │
│  Loop every 1 second:                       │
│                                             │
│  1. Wait for startup completion             │
│     └─> xEventGroupWaitBits(...)           │
│                                             │
│  2. For each enabled payload:               │
│                                             │
│     a. Read EDC-1 data                      │
│        └─> payload_get_data(EDC_0, &data)  │
│        └─> sat_data_buf.edc_0 = data       │
│                                             │
│     b. Read EDC-2 data                      │
│        └─> payload_get_data(EDC_1, &data)  │
│        └─> sat_data_buf.edc_1 = data       │
│                                             │
│     c. Read Payload-X data                  │
│        └─> payload_get_data(PLD_X, &data)  │
│        └─> sat_data_buf.payload_x = data   │
│                                             │
│     d. Read HARSH data                      │
│        └─> payload_get_data(HARSH, &data)  │
│        └─> sat_data_buf.harsh = data       │
│                                             │
│  3. Sleep until next cycle                  │
│     └─> vTaskDelay(1000 ms)                │
│                                             │
└─────────────────────────────────────────────┘
```

### 11. Beacon Task

**Location**: firmware/app/tasks/beacon.c:1

**Purpose**: Periodic telemetry broadcast to ground station

**Configuration**:
- **Priority**: 3 (Normal)
- **Stack Size**: 250 bytes
- **Period**: 60000 ms (60 seconds, configurable)
- **Status**: DISABLED by default (CONFIG_TASK_BEACON_EN = 0)

**Responsibilities**:
1. Periodically create telemetry packet
2. Format packet with fsat_pkt library
3. Transmit via TTC module
4. Include current satellite status

**Execution Flow**:
```
┌─────────────────────────────────────────────┐
│ Beacon Task (60000 ms, Priority 3)         │
├─────────────────────────────────────────────┤
│                                             │
│  Loop every 60 seconds:                     │
│                                             │
│  1. Wait for startup completion             │
│     └─> xEventGroupWaitBits(...)           │
│                                             │
│  2. Create beacon packet                    │
│     └─> packet_id = 0x20 (General Telem)   │
│                                             │
│  3. Populate packet payload                 │
│     └─> Copy sat_data_buf.obdh             │
│     └─> Copy sat_data_buf.eps              │
│     └─> Copy sat_data_buf.ttc_0            │
│     └─> Copy sat_data_buf.antenna          │
│     └─> Add timestamp                      │
│                                             │
│  4. Format packet                           │
│     └─> fsat_pkt_encode(&beacon_pkt)       │
│     └─> Add header, checksum               │
│     └─> NGHam encoding                     │
│                                             │
│  5. Transmit packet                         │
│     └─> ttc_send(TTC_0, beacon_pkt, len)   │
│                                             │
│  6. Log transmission                        │
│     └─> sys_log(INFO, "Beacon sent")       │
│                                             │
│  7. Sleep until next cycle                  │
│     └─> vTaskDelay(60000 ms)               │
│                                             │
└─────────────────────────────────────────────┘
```

**Note**: Disabled by default to conserve power and reduce RF emissions.

### 12. Data Log Task

**Location**: firmware/app/tasks/data_log.c:1

**Purpose**: Periodic data archival to non-volatile memory

**Configuration**:
- **Priority**: 3 (Normal)
- **Stack Size**: 225 bytes
- **Period**: 600000 ms (10 minutes, configurable)
- **Status**: Enabled

**Responsibilities**:
1. Collect housekeeping data every 10 minutes
2. Format log entry with timestamp
3. Write to non-volatile memory (NOR Flash)
4. Manage storage space (wrap-around if full)

**Execution Flow**:
```
┌─────────────────────────────────────────────┐
│ Data Log Task (600000 ms, Priority 3)      │
├─────────────────────────────────────────────┤
│                                             │
│  Loop every 10 minutes:                     │
│                                             │
│  1. Wait for startup completion             │
│     └─> xEventGroupWaitBits(...)           │
│                                             │
│  2. Collect housekeeping data               │
│     └─> Copy sat_data_buf.obdh             │
│     └─> Copy sat_data_buf.eps              │
│     └─> Copy sat_data_buf.ttc_0            │
│     └─> Add timestamp                      │
│                                             │
│  3. Format log entry                        │
│     └─> Create entry header                │
│     └─> Add entry sequence number          │
│                                             │
│  4. Write to non-volatile memory            │
│     └─> media_write(MEDIA_NOR,             │
│             log_addr, entry, len)          │
│                                             │
│  5. Verify write operation                  │
│     └─> if (error)                         │
│         └─> sys_log(ERROR, "Log failed")   │
│                                             │
│  6. Update write pointer                    │
│     └─> log_addr += entry_size             │
│     └─> Handle wrap-around                 │
│                                             │
│  7. Sleep until next cycle                  │
│     └─> vTaskDelay(600000 ms)              │
│                                             │
└─────────────────────────────────────────────┘
```

**Storage**: MT25Q NOR Flash (up to 256 MB capacity)

### 13. Process TC (Telecommand) Task

**Location**: firmware/app/tasks/process_tc.c:1

**Purpose**: Parse and execute telecommands from ground station

**Configuration**:
- **Priority**: 4 (High)
- **Stack Size**: 500 bytes (largest stack due to command processing)
- **Period**: 5 ms (200 Hz, very high frequency)
- **Initial Delay**: 1000 ms (allow system initialization)
- **Status**: Enabled

**Responsibilities**:
1. Dequeue received telecommands
2. Parse packet ID and parameters
3. Validate command authority
4. Execute command action
5. Generate feedback packet
6. Log command execution

**Execution Flow**:
```
┌─────────────────────────────────────────────┐
│ Process TC Task (5 ms, Priority 4)         │
├─────────────────────────────────────────────┤
│                                             │
│  Initial delay: 1 second                    │
│  Loop every 5 ms:                           │
│                                             │
│  1. Check command queue                     │
│     └─> xQueueReceive(tc_queue, &cmd, 0)   │
│                                             │
│  2. If command received:                    │
│                                             │
│     a. Parse packet                         │
│        └─> Extract packet_id               │
│        └─> Extract parameters              │
│                                             │
│     b. Validate command                     │
│        └─> Check packet_id range           │
│        └─> Verify parameter validity       │
│                                             │
│     c. Execute command                      │
│        └─> switch(packet_id) {             │
│            case 0x40: // Ping              │
│                send_ping_answer()          │
│            case 0x41: // Data Request      │
│                handle_data_request()       │
│            case 0x43: // Enter Hibernation │
│                enter_hibernation()         │
│            case 0x45: // Activate Module   │
│                activate_module()           │
│            case 0x4A: // Force Reset       │
│                trigger_system_reset()      │
│            ...                             │
│            }                               │
│                                             │
│     d. Generate feedback packet (0x25)     │
│        └─> Create feedback with result     │
│        └─> ttc_send(TTC_0, feedback)       │
│                                             │
│     e. Log command execution                │
│        └─> sat_data_buf.obdh.last_valid_tc│
│                = packet_id                 │
│        └─> sys_log(INFO, "TC executed")    │
│                                             │
│  3. Sleep until next cycle                  │
│     └─> vTaskDelay(5 ms)                   │
│                                             │
└─────────────────────────────────────────────┘
```

**Supported Commands**: See firmware/config/config.h:1 (Packet IDs 0x40-0x4D)

### 14. Antenna Deployment Task

**Location**: firmware/app/tasks/antenna_deployment.c:1

**Purpose**: Execute antenna deployment sequence

**Configuration**:
- **Priority**: 3 (Normal)
- **Stack Size**: 250 bytes
- **Execution**: Event-driven (triggered by command)
- **Status**: Disabled by default

**Responsibilities**:
1. Execute deployment sequence on command
2. Monitor deployment status
3. Retry on failure (up to 10 attempts)
4. Implement safety delays (45-minute hibernation after failure)

**Execution Flow**:
```
┌─────────────────────────────────────────────┐
│ Antenna Deployment Task (Event-driven)     │
├─────────────────────────────────────────────┤
│                                             │
│  1. Wait for deployment trigger             │
│     └─> xSemaphoreTake(deploy_sem, ∞)      │
│                                             │
│  2. Execute deployment sequence             │
│     └─> antenna_deploy(DEPLOY_ALL)         │
│         └─> Independent: 10s burn          │
│         └─> Sequential: 20s burn           │
│                                             │
│  3. Monitor deployment                      │
│     └─> Poll antenna_get_status()          │
│     └─> Check deployment confirmation      │
│                                             │
│  4. Handle deployment failure               │
│     └─> if (failed && attempts < 10)       │
│         └─> Increment attempt counter      │
│         └─> Hibernation (45 min)           │
│         └─> Retry deployment               │
│                                             │
│  5. Report deployment result                │
│     └─> Log success/failure                │
│     └─> Update sat_data_buf.antenna        │
│                                             │
└─────────────────────────────────────────────┘
```

**Safety Parameters**:
- Max attempts: 10
- Burn time (independent): 10 seconds
- Burn time (sequential): 20 seconds
- Failure hibernation: 45 minutes

### 15. Housekeeping Task

**Location**: firmware/app/tasks/housekeeping.c:1

**Purpose**: Operational mode management (Normal / Hibernation)

**Configuration**:
- **Priority**: 3 (Normal)
- **Stack Size**: 250 bytes
- **Period**: Variable (depends on mode)
- **Status**: Enabled

**Responsibilities**:
1. Manage hibernation mode entry/exit
2. Monitor hibernation duration
3. Transition between operational modes
4. Reduce power consumption during hibernation

**Execution Flow**:
```
┌─────────────────────────────────────────────┐
│ Housekeeping Task (Variable period)        │
├─────────────────────────────────────────────┤
│                                             │
│  Continuous loop:                           │
│                                             │
│  1. Check operational mode                  │
│     └─> if (mode == MODE_HIBERNATION)      │
│                                             │
│  2. In Hibernation mode:                    │
│     a. Disable non-essential tasks          │
│     b. Reduce sensor polling frequency      │
│     c. Monitor hibernation timer            │
│     d. If timer expired:                    │
│        └─> Exit hibernation                │
│        └─> Re-enable tasks                 │
│        └─> mode = MODE_NORMAL              │
│                                             │
│  3. In Normal mode:                         │
│     └─> All tasks active                   │
│     └─> Normal operation                   │
│                                             │
│  4. Sleep                                   │
│     └─> vTaskDelay(variable)               │
│                                             │
└─────────────────────────────────────────────┘
```

**Modes**:
- **Normal**: Full functionality, all tasks active
- **Hibernation**: Low-power, essential tasks only, configurable duration

## Task Synchronization

### Event Groups

**Startup Synchronization**: `task_startup_status`

```c
EventGroupHandle_t task_startup_status;
#define TASK_STARTUP_DONE   (1 << 0)

// Startup task sets bit when complete
xEventGroupSetBits(task_startup_status, TASK_STARTUP_DONE);

// Other tasks wait for startup
xEventGroupWaitBits(task_startup_status,
                    TASK_STARTUP_DONE,
                    pdFALSE, pdTRUE, portMAX_DELAY);
```

### Queues

**Telecommand Queue**: `tc_queue`

```c
QueueHandle_t tc_queue;

// Read TTC task enqueues commands
xQueueSend(tc_queue, &telecommand, 0);

// Process TC task dequeues commands
xQueueReceive(tc_queue, &cmd, 0);
```

### Semaphores

Used for event-driven tasks (e.g., antenna deployment trigger)

```c
SemaphoreHandle_t deploy_sem;

// Trigger deployment
xSemaphoreGive(deploy_sem);

// Deployment task waits
xSemaphoreTake(deploy_sem, portMAX_DELAY);
```

## Task Timing Summary

| Frequency | Period | Tasks |
|-----------|--------|-------|
| 200 Hz | 5 ms | Process TC |
| 10 Hz | 100 ms | Read TTC |
| 2 Hz | 500 ms | Watchdog Reset |
| 1 Hz | 1000 ms | Heartbeat, Time Control, Read Sensors, Read EPS, Read Antenna, Read EDC |
| 0.0167 Hz | 60000 ms | Beacon (disabled) |
| 0.00167 Hz | 600000 ms | Data Log |

## Memory Usage

### Stack Sizes

| Task | Stack (bytes) | Justification |
|------|--------------|---------------|
| Startup | 350 | Device initialization, logging |
| Process TC | 500 | Command parsing, largest stack |
| Read EPS | 250 | I2C communication, data buffers |
| Read Sensors | 250 | ADC operations, data buffers |
| Read EDC | 250 | Payload communication |
| Beacon | 250 | Packet formatting |
| Antenna Deployment | 250 | I2C communication |
| Housekeeping | 250 | Mode management |
| Data Log | 225 | Flash write operations |
| Watchdog Reset | 200 | Minimal operations |
| Heartbeat | 200 | Simple LED toggle |
| System Reset | 200 | Reset operations |
| Time Control | 200 | Simple counter increment |
| Read Antenna | 200 | I2C read |
| Read TTC | 200 | SPI communication |

**Total Stack Allocation**: ~3.6 KB (for all tasks)

### Heap Usage

**Total FreeRTOS Heap**: 40 KB

**Allocations**:
- Task Control Blocks (TCBs): ~15 tasks × ~100 bytes = 1.5 KB
- Task stacks: 3.6 KB
- Queues: Variable (depends on queue length)
- Event groups: ~24 bytes each
- Semaphores: ~80 bytes each

**Available for dynamic allocation**: ~33 KB

## Task Configuration

**Configuration File**: firmware/config/config.h:1

```c
// Task enable/disable flags
#define CONFIG_TASK_STARTUP_EN              1
#define CONFIG_TASK_BEACON_EN               0  // DISABLED
#define CONFIG_TASK_DATA_LOG_EN             1
#define CONFIG_TASK_PROCESS_TC_EN           1
#define CONFIG_TASK_WATCHDOG_RESET_EN       1
#define CONFIG_TASK_HEARTBEAT_EN            1
#define CONFIG_TASK_SYSTEM_RESET_EN         1
#define CONFIG_TASK_TIME_CONTROL_EN         1
#define CONFIG_TASK_READ_SENSORS_EN         1
#define CONFIG_TASK_READ_EPS_EN             1
#define CONFIG_TASK_READ_TTC_EN             1
#define CONFIG_TASK_READ_ANTENNA_EN         1
#define CONFIG_TASK_READ_EDC_EN             1
#define CONFIG_TASK_ANTENNA_DEPLOYMENT_EN   0  // DISABLED
#define CONFIG_TASK_HOUSEKEEPING_EN         1
```

## Summary

The OBDH2 task architecture provides:

1. **Responsiveness**: High-priority tasks for critical operations (commands, watchdog)
2. **Modularity**: Clear task separation by function
3. **Scalability**: Easy addition/removal of tasks via configuration
4. **Reliability**: Event-driven synchronization and error handling
5. **Efficiency**: Optimized stack sizes and task periods
6. **Mission Support**: Comprehensive coverage of satellite operations

All tasks coordinate through FreeRTOS primitives (queues, event groups, semaphores) to ensure reliable, real-time satellite operations.
