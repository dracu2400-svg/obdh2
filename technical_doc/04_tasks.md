# OBDH 2.0 FreeRTOS Tasks

## Overview

The OBDH 2.0 firmware uses FreeRTOS for real-time multitasking. This document provides detailed descriptions of each task, including purpose, timing, priorities, and interactions.

**RTOS:** FreeRTOS v10.2.1
**Scheduling:** Preemptive, priority-based
**Tick Rate:** 1000 Hz (1 ms resolution)
**Max Priorities:** 5 (0 = lowest, 4 = highest)

---

## Task Summary Table

| Task Name | Period | Priority | Stack (bytes) | Source File | Status |
|-----------|--------|----------|---------------|-------------|--------|
| Startup | One-shot | 5 | 350 | `startup_task.c` | Enabled |
| Watchdog Reset | ~100 ms | 1 | 150 | `watchdog_reset_task.c` | Enabled |
| Heartbeat | 1 Hz | 1 | 160 | `heartbeat_task.c` | Enabled |
| System Reset | On-demand | 2 | 128 | `system_reset_task.c` | Enabled |
| Read Sensors | 10 Hz | 3 | 140 | `read_sensors_task.c` | Enabled |
| Time Control | High-freq | 3 | 128 | `time_control_task.c` | Enabled |
| Read EPS | 60 sec | 3 | 384 | `read_eps_task.c` | Enabled |
| Read TTC | 60 sec | 3 | 384 | `read_ttc_task.c` | Enabled |
| Read Antenna | On-demand | 2 | 150 | `read_antenna_task.c` | Enabled |
| Read EDC | On-demand | 3 | 300 | `read_edc_task.c` | Enabled |
| Data Log | 5 min | 3 | 225 | `data_log_task.c` | Enabled |
| Process TC | Variable | 4 | 500 | `process_tc_task.c` | Enabled |
| Beacon | 60 sec | 5 | 1000 | `beacon_task.c` | Enabled |
| Antenna Deploy | One-shot | 5 | 150 | `antenna_deploy_task.c` | Disabled by default |

**Note:** Task enable/disable status is configurable in `config/config.h`

---

## Task Descriptions

### 1. Startup Task

**File:** `app/tasks/startup_task.c/h`

**Purpose:** System initialization and hardware setup

**Priority:** 5 (Highest - runs first)

**Stack Size:** 350 bytes

**Execution:** One-shot (runs once at boot, then suspends)

**Responsibilities:**
1. Enable watchdog timer
2. Initialize debug UART (sys_log)
3. Print startup banner and version
4. Initialize SPI buses (for NOR Flash, FRAM)
5. Initialize I2C buses (for EPS, TTC, sensors)
6. Initialize storage media (FRAM, NOR Flash)
7. Load configuration from FRAM
8. Initialize EPS interface
9. Initialize TTC interfaces (both modules)
10. Initialize antenna interface
11. Initialize payload interfaces
12. Check reset cause and increment boot counter
13. Perform self-test (device connectivity checks)
14. Set `task_startup_status` event flag to signal completion
15. Suspend itself

**Pseudocode:**
```c
void startup_task(void *pvParameters) {
    // Enable watchdog
    wdt_init();

    // Initialize debug UART
    sys_log_init();
    sys_log_print("OBDH 2.0 Starting...");
    sys_log_print("Version: 0.10.0");

    // Initialize communication buses
    spi_init_all();
    i2c_init_all();

    // Initialize storage
    fram_init();
    nor_flash_init();

    // Load configuration
    load_config_from_fram();

    // Initialize subsystems
    eps_init();
    ttc_init(TTC_0);
    ttc_init(TTC_1);
    antenna_init();
    payload_init();

    // System diagnostics
    check_reset_cause();
    increment_boot_counter();
    self_test();

    // Signal startup complete
    xEventGroupSetBits(task_startup_status, STARTUP_DONE_BIT);

    sys_log_print("Startup complete");

    // Suspend (never runs again)
    vTaskSuspend(NULL);
}
```

**Dependencies:** None (runs first)

**Signals:** Sets `task_startup_status` event group when complete

---

### 2. Watchdog Reset Task

**File:** `app/tasks/watchdog_reset_task.c/h`

**Purpose:** Periodically reset watchdog timer to prevent system reset

**Priority:** 1 (Low - but must run regularly)

**Stack Size:** 150 bytes

**Period:** ~100 milliseconds

**Responsibilities:**
1. Reset watchdog timer before timeout
2. Monitor system health (optional: check if critical tasks are running)

**Pseudocode:**
```c
void watchdog_reset_task(void *pvParameters) {
    // Wait for startup to complete
    xEventGroupWaitBits(task_startup_status, STARTUP_DONE_BIT, ...);

    while (1) {
        // Reset watchdog
        wdt_reset();

        // Sleep for 100 ms
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

**Critical:** This task must run at least every 100 ms, or the watchdog will trigger a system reset.

**Watchdog Timeout:** Typically 1-5 seconds (configurable)

**Failure Mode:** If this task fails to run (due to system lockup), watchdog triggers reset, restoring system operation.

---

### 3. Heartbeat Task

**File:** `app/tasks/heartbeat_task.c/h`

**Purpose:** Visual system status indicator (LED blink)

**Priority:** 1 (Low - not mission-critical)

**Stack Size:** 160 bytes

**Period:** 1 Hz (1 second on, 1 second off, or 0.5 Hz blink)

**Responsibilities:**
1. Toggle status LED at regular interval
2. Indicate system is alive and running

**Pseudocode:**
```c
void heartbeat_task(void *pvParameters) {
    // Wait for startup
    xEventGroupWaitBits(task_startup_status, STARTUP_DONE_BIT, ...);

    while (1) {
        // Toggle LED
        led_toggle(LED_HEARTBEAT);

        // Sleep for 1 second
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

**LED Pattern:**
- **Normal Mode:** Steady 1 Hz blink
- **Hibernation Mode:** Slower blink or off (power saving)
- **Error:** Rapid blink or different pattern

---

### 4. System Reset Task

**File:** `app/tasks/system_reset_task.c/h`

**Purpose:** Controlled system reset on command

**Priority:** 2 (Medium)

**Stack Size:** 128 bytes

**Execution:** Event-driven (triggered by telecommand or error condition)

**Responsibilities:**
1. Wait for reset command event
2. Save critical state to FRAM
3. Log reset reason
4. Perform software reset

**Pseudocode:**
```c
void system_reset_task(void *pvParameters) {
    // Wait for startup
    xEventGroupWaitBits(task_startup_status, STARTUP_DONE_BIT, ...);

    while (1) {
        // Wait for reset event
        xEventGroupWaitBits(system_reset_event, RESET_REQUESTED_BIT, ...);

        // Save state
        save_state_to_fram();

        // Log reset
        sys_log_print("System reset requested");

        // Perform reset
        system_reset();

        // (Never reached)
    }
}
```

**Trigger Sources:**
- Telecommand (force reset)
- Critical error detection
- Unrecoverable fault

**Reset Types:**
- Software reset (warm reset)
- Watchdog reset (if software reset fails)

---

### 5. Read Sensors Task

**File:** `app/tasks/read_sensors_task.c/h`

**Purpose:** Collect OBDH internal telemetry (temperature, voltage, current)

**Priority:** 3 (Medium-high)

**Stack Size:** 140 bytes

**Period:** 100 milliseconds (10 Hz)

**Responsibilities:**
1. Read microcontroller temperature (ADC)
2. Read input voltage (ADC)
3. Read input current (ADC)
4. Update `sat_data_buf.obdh.data`
5. Update timestamp

**Pseudocode:**
```c
void read_sensors_task(void *pvParameters) {
    // Wait for startup
    xEventGroupWaitBits(task_startup_status, STARTUP_DONE_BIT, ...);

    while (1) {
        // Read ADC channels
        uint16_t temp_raw = adc_read_channel(ADC_CHANNEL_TEMP);
        uint16_t volt_raw = adc_read_channel(ADC_CHANNEL_VOLTAGE);
        uint16_t curr_raw = adc_read_channel(ADC_CHANNEL_CURRENT);

        // Convert to engineering units
        uint16_t temp_k10 = convert_temp_to_kelvin(temp_raw);
        uint16_t volt_mv = convert_to_millivolts(volt_raw);
        uint16_t curr_ma = convert_to_milliamps(curr_raw);

        // Update data buffer (with mutex)
        xSemaphoreTake(sat_data_mutex, portMAX_DELAY);
        sat_data_buf.obdh.data.temperature_uc = temp_k10;
        sat_data_buf.obdh.data.input_voltage = volt_mv;
        sat_data_buf.obdh.data.input_current = curr_ma;
        sat_data_buf.obdh.timestamp = get_system_time();
        xSemaphoreGive(sat_data_mutex);

        // Sleep for 100 ms
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

**Data Updates:**
- `temperature_uc`: Microcontroller temperature (Kelvin * 10)
- `input_voltage`: OBDH input voltage (mV)
- `input_current`: OBDH input current (mA)

---

### 6. Time Control Task

**File:** `app/tasks/time_control_task.c/h`

**Purpose:** Maintain system time and handle time synchronization

**Priority:** 3 (Medium-high)

**Stack Size:** 128 bytes

**Period:** High-frequency (1 Hz or faster)

**Responsibilities:**
1. Increment system time counter (milliseconds)
2. Synchronize with RTC (Real-Time Clock)
3. Handle time set commands
4. Update `sat_data_buf.obdh.data.time_counter`

**Pseudocode:**
```c
void time_control_task(void *pvParameters) {
    // Wait for startup
    xEventGroupWaitBits(task_startup_status, STARTUP_DONE_BIT, ...);

    while (1) {
        // Increment time counter
        system_time_ms += 1;  // Or read from RTC

        // Update data buffer
        xSemaphoreTake(sat_data_mutex, portMAX_DELAY);
        sat_data_buf.obdh.data.time_counter = system_time_ms;
        xSemaphoreGive(sat_data_mutex);

        // Sleep for 1 ms (or synced to RTC interrupt)
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
```

**Time Synchronization:**
- Ground station can send "Set Time" telecommand
- System time updated via Process TC Task
- Time Control Task reads new value and continues

---

### 7. Read EPS Task

**File:** `app/tasks/read_eps_task.c/h`

**Purpose:** Collect telemetry from Electrical Power System

**Priority:** 3 (Medium-high)

**Stack Size:** 384 bytes

**Period:** 60 seconds

**Responsibilities:**
1. Request telemetry from EPS via I2C
2. Parse EPS data structure
3. Update `sat_data_buf.eps`
4. Check for low battery (hibernation trigger)

**Pseudocode:**
```c
void read_eps_task(void *pvParameters) {
    // Wait for startup
    xEventGroupWaitBits(task_startup_status, STARTUP_DONE_BIT, ...);

    while (1) {
        // Request EPS telemetry
        eps_data_t eps_data;
        int result = sl_eps2_request_telemetry(&eps_data);

        if (result == 0) {
            // Update data buffer
            xSemaphoreTake(sat_data_mutex, portMAX_DELAY);
            memcpy(&sat_data_buf.eps.data, &eps_data, sizeof(eps_data_t));
            sat_data_buf.eps.timestamp = get_system_time();
            xSemaphoreGive(sat_data_mutex);

            // Check battery voltage for hibernation
            if (eps_data.battery_voltage < LOW_BATTERY_THRESHOLD) {
                enter_hibernation_mode();
            }
        } else {
            sys_log_print("EPS read failed");
        }

        // Sleep for 60 seconds
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}
```

**EPS Data Includes:**
- Battery voltage, current, charge, temperature
- Solar panel voltages and currents (6 panels)
- Power bus voltages (3.3V, 5V, 12V)
- Heater status

**Error Handling:**
- If I2C communication fails, log error and retry next period
- EPS data in `sat_data_buf` remains stale (timestamp not updated)

---

### 8. Read TTC Task

**File:** `app/tasks/read_ttc_task.c/h`

**Purpose:** Collect telemetry from TTC modules and check for uplink packets

**Priority:** 3 (Medium-high)

**Stack Size:** 384 bytes

**Period:** 60 seconds (or more frequent for uplink checking)

**Responsibilities:**
1. Request telemetry from TTC 0 and TTC 1 via I2C
2. Update `sat_data_buf.ttc[0]` and `ttc[1]`
3. Check for received packets
4. Trigger Process TC Task if packet available

**Pseudocode:**
```c
void read_ttc_task(void *pvParameters) {
    // Wait for startup
    xEventGroupWaitBits(task_startup_status, STARTUP_DONE_BIT, ...);

    while (1) {
        // Read TTC 0
        ttc_data_t ttc0_data;
        if (sl_ttc2_request_telemetry(TTC_PORT_0, &ttc0_data) == 0) {
            xSemaphoreTake(sat_data_mutex, portMAX_DELAY);
            memcpy(&sat_data_buf.ttc[0].data, &ttc0_data, sizeof(ttc_data_t));
            sat_data_buf.ttc[0].timestamp = get_system_time();
            xSemaphoreGive(sat_data_mutex);
        }

        // Read TTC 1
        ttc_data_t ttc1_data;
        if (sl_ttc2_request_telemetry(TTC_PORT_1, &ttc1_data) == 0) {
            xSemaphoreTake(sat_data_mutex, portMAX_DELAY);
            memcpy(&sat_data_buf.ttc[1].data, &ttc1_data, sizeof(ttc_data_t));
            sat_data_buf.ttc[1].timestamp = get_system_time();
            xSemaphoreGive(sat_data_mutex);
        }

        // Check for uplink packets
        if (sl_ttc2_packet_available(TTC_PORT_0) ||
            sl_ttc2_packet_available(TTC_PORT_1)) {
            // Signal Process TC Task
            xEventGroupSetBits(tc_event_group, TC_AVAILABLE_BIT);
        }

        // Sleep for 60 seconds (or shorter for uplink polling)
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}
```

**TTC Data Includes:**
- Radio status, temperature
- Transmission power, frequency, bitrate
- Last RSSI (signal strength)
- Packet counts (TX/RX)

**Uplink Polling:**
- Check for received packets more frequently (e.g., every 1-5 seconds)
- Separate polling loop or higher frequency execution

---

### 9. Read Antenna Task

**File:** `app/tasks/read_antenna_task.c/h`

**Purpose:** Monitor antenna deployment status

**Priority:** 2 (Medium)

**Stack Size:** 150 bytes

**Execution:** On-demand (triggered by telecommand or periodic check)

**Responsibilities:**
1. Request deployment status from ISIS antenna module
2. Read temperature
3. Update `sat_data_buf.antenna`

**Pseudocode:**
```c
void read_antenna_task(void *pvParameters) {
    // Wait for startup
    xEventGroupWaitBits(task_startup_status, STARTUP_DONE_BIT, ...);

    while (1) {
        // Wait for trigger event
        xEventGroupWaitBits(antenna_event_group, READ_ANTENNA_BIT, ...);

        // Request antenna status
        antenna_data_t ant_data;
        isis_antenna_get_status(&ant_data);

        // Update data buffer
        xSemaphoreTake(sat_data_mutex, portMAX_DELAY);
        memcpy(&sat_data_buf.antenna.data, &ant_data, sizeof(antenna_data_t));
        sat_data_buf.antenna.timestamp = get_system_time();
        xSemaphoreGive(sat_data_mutex);

        // Sleep (or wait for next trigger)
        vTaskDelay(pdMS_TO_TICKS(60000));  // Periodic fallback
    }
}
```

**Antenna Data:**
- Deployment status for each of 4 antennas
- Temperature
- Burn time and attempt count

---

### 10. Read EDC Task

**File:** `app/tasks/read_edc_task.c/h`

**Purpose:** Collect data from EDC (Earth Data Collection) payload

**Priority:** 3 (Medium-high)

**Stack Size:** 300 bytes

**Execution:** On-demand or periodic (payload-dependent)

**Responsibilities:**
1. Request data from EDC module via UART
2. Receive and buffer EDC data
3. Update `sat_data_buf.payload[EDC_0]` or `payload[EDC_1]`

**Pseudocode:**
```c
void read_edc_task(void *pvParameters) {
    // Wait for startup
    xEventGroupWaitBits(task_startup_status, STARTUP_DONE_BIT, ...);

    while (1) {
        // Check if payload enabled
        if (!sat_data_buf.payload[PAYLOAD_EDC_1].enabled) {
            vTaskDelay(pdMS_TO_TICKS(60000));
            continue;
        }

        // Request data from EDC
        uint8_t edc_data[220];
        int len = edc_request_data(edc_data, sizeof(edc_data));

        if (len > 0) {
            // Update data buffer
            xSemaphoreTake(sat_data_mutex, portMAX_DELAY);
            memcpy(sat_data_buf.payload[PAYLOAD_EDC_1].data, edc_data, len);
            sat_data_buf.payload[PAYLOAD_EDC_1].timestamp = get_system_time();
            xSemaphoreGive(sat_data_mutex);
        }

        // Sleep (or wait for next trigger)
        vTaskDelay(pdMS_TO_TICKS(300000));  // 5 minutes
    }
}
```

**Payload Data:**
- Up to 220 bytes of payload-specific data
- Format depends on payload type

---

### 11. Data Log Task

**File:** `app/tasks/data_log_task.c/h`

**Purpose:** Periodically save telemetry to non-volatile storage

**Priority:** 3 (Medium-high)

**Stack Size:** 225 bytes

**Period:** 5 minutes (300 seconds)

**Responsibilities:**
1. Create snapshot of `sat_data_buf`
2. Serialize data into packet format
3. Write to FRAM (recent data) and NOR Flash (long-term archive)
4. Manage circular buffers

**Pseudocode:**
```c
void data_log_task(void *pvParameters) {
    // Wait for startup
    xEventGroupWaitBits(task_startup_status, STARTUP_DONE_BIT, ...);

    while (1) {
        // Acquire mutex and copy data
        sat_data_t snapshot;
        xSemaphoreTake(sat_data_mutex, portMAX_DELAY);
        memcpy(&snapshot, &sat_data_buf, sizeof(sat_data_t));
        xSemaphoreGive(sat_data_mutex);

        // Serialize data
        uint8_t packet[256];
        int len = serialize_telemetry(&snapshot, packet);

        // Write to FRAM (fast, recent buffer)
        fram_write_circular(packet, len);

        // Write to NOR Flash (long-term archive)
        nor_flash_append_log(packet, len);

        // Sleep for 5 minutes
        vTaskDelay(pdMS_TO_TICKS(300000));
    }
}
```

**Storage Strategy:**
- **FRAM:** Last N entries (e.g., 100 entries)
- **NOR Flash:** Historical archive (circular buffer, days/weeks)

---

### 12. Process TC Task

**File:** `app/tasks/process_tc_task.c/h`

**Purpose:** Process uplink telecommands from ground station

**Priority:** 4 (High - second highest)

**Stack Size:** 500 bytes

**Execution:** Event-driven (triggered by Read TTC Task)

**Responsibilities:**
1. Wait for uplink packet event
2. Retrieve packet from TTC module
3. Validate packet (CRC, HMAC)
4. Parse telecommand
5. Execute command
6. Generate feedback packet
7. Queue feedback for downlink

**Pseudocode:**
```c
void process_tc_task(void *pvParameters) {
    // Wait for startup
    xEventGroupWaitBits(task_startup_status, STARTUP_DONE_BIT, ...);

    while (1) {
        // Wait for TC available event
        xEventGroupWaitBits(tc_event_group, TC_AVAILABLE_BIT, ...);

        // Retrieve packet
        uint8_t packet[256];
        int len = ttc_receive_packet(packet, sizeof(packet));

        // Validate CRC
        if (!validate_crc(packet, len)) {
            sys_log_print("TC CRC failed");
            continue;
        }

        // Validate HMAC
        if (!validate_hmac(packet, len)) {
            sys_log_print("TC HMAC failed");
            continue;
        }

        // Parse telecommand
        uint8_t tc_id = packet[1];  // Packet ID

        // Execute command
        execute_telecommand(tc_id, packet);

        // Update last TC info
        sat_data_buf.obdh.data.last_valid_tc = tc_id;
        sat_data_buf.obdh.data.rssi_last_tc = get_last_rssi();

        // Generate feedback
        generate_tc_feedback(tc_id, STATUS_SUCCESS);
    }
}
```

**Telecommand Types:**
- Ping, Data Request, Broadcast
- Hibernation control
- Payload enable/disable
- Parameter read/write
- System reset, time sync, memory erase

**Execution Examples:**

**Ping Command:**
```c
case TC_PING:
    // Echo data back
    queue_downlink_packet(PKT_PING_REPLY, packet_payload);
    break;
```

**Hibernation Command:**
```c
case TC_ENTER_HIBERNATION:
    // Set mode
    sat_data_buf.obdh.data.mode = MODE_HIBERNATION;
    // Disable non-critical tasks
    enter_hibernation_mode();
    break;
```

**Payload Enable:**
```c
case TC_PAYLOAD_ENABLE:
    uint8_t payload_id = packet[5];
    sat_data_buf.payload[payload_id].enabled = 1;
    break;
```

---

### 13. Beacon Task

**File:** `app/tasks/beacon_task.c/h`

**Purpose:** Periodically transmit status beacon to ground station

**Priority:** 5 (Highest)

**Stack Size:** 1000 bytes

**Period:** 60 seconds

**Responsibilities:**
1. Create beacon packet from `sat_data_buf`
2. Format packet (FloripaSat Packet format)
3. Calculate CRC and HMAC
4. Transmit via TTC modules (both for redundancy)

**Pseudocode:**
```c
void beacon_task(void *pvParameters) {
    // Wait for startup
    xEventGroupWaitBits(task_startup_status, STARTUP_DONE_BIT, ...);

    while (1) {
        // Copy data buffer
        sat_data_t snapshot;
        xSemaphoreTake(sat_data_mutex, portMAX_DELAY);
        memcpy(&snapshot, &sat_data_buf, sizeof(sat_data_t));
        xSemaphoreGive(sat_data_mutex);

        // Format beacon packet
        uint8_t beacon[256];
        int len = format_beacon_packet(&snapshot, beacon);

        // Add CRC
        append_crc(beacon, len);

        // Add HMAC (if secure beacon)
        append_hmac(beacon, len);

        // Transmit via TTC 0
        sl_ttc2_send_packet(TTC_PORT_0, beacon, len);

        // Transmit via TTC 1 (redundancy)
        sl_ttc2_send_packet(TTC_PORT_1, beacon, len);

        // Sleep for 60 seconds
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}
```

**Beacon Contents:**
- OBDH status (uptime, temperature, voltage, current)
- EPS battery status
- TTC status
- Operational mode
- Last telecommand

**Beacon Format:** FloripaSat Packet with ID 0x20 (Telemetry)

---

### 14. Antenna Deploy Task

**File:** `app/tasks/antenna_deploy_task.c/h`

**Purpose:** Automated antenna deployment on first boot

**Priority:** 5 (Highest)

**Stack Size:** 150 bytes

**Execution:** One-shot (first boot only)

**Configuration:** Disabled by default (`TASK_ANTENNA_ENABLED = 0`)

**Responsibilities:**
1. Check if antennas already deployed
2. Wait deployment delay (30-60 minutes after launch)
3. Execute deployment sequence
4. Verify deployment
5. Retry if failed (up to 10 attempts)
6. Enter hibernation on repeated failure

**Pseudocode:**
```c
void antenna_deploy_task(void *pvParameters) {
    // Wait for startup
    xEventGroupWaitBits(task_startup_status, STARTUP_DONE_BIT, ...);

    // Check if already deployed
    if (is_antenna_deployed()) {
        sys_log_print("Antennas already deployed");
        vTaskSuspend(NULL);
    }

    // Wait deployment delay (e.g., 60 minutes)
    vTaskDelay(pdMS_TO_TICKS(60 * 60 * 1000));

    int attempts = 0;
    while (attempts < MAX_DEPLOY_ATTEMPTS) {
        // Deploy sequence
        isis_antenna_deploy(ANTENNA_1, 10);  // 10 second burn
        vTaskDelay(pdMS_TO_TICKS(15000));

        isis_antenna_deploy(ANTENNA_2, 10);
        vTaskDelay(pdMS_TO_TICKS(15000));

        isis_antenna_deploy(ANTENNA_3, 10);
        vTaskDelay(pdMS_TO_TICKS(15000));

        isis_antenna_deploy(ANTENNA_4, 10);
        vTaskDelay(pdMS_TO_TICKS(15000));

        // Verify deployment
        if (is_antenna_deployed()) {
            sys_log_print("Antennas deployed successfully");
            vTaskSuspend(NULL);
        }

        attempts++;
    }

    // Deployment failed
    sys_log_print("Antenna deployment failed");
    enter_hibernation_mode();  // 45 minutes
    vTaskSuspend(NULL);
}
```

**Deployment Parameters:**
- **Independent Burn:** 10 seconds per antenna
- **Sequential Burn:** 20 seconds total
- **Max Attempts:** 10
- **Hibernation on Failure:** 45 minutes

**Safety:** Disabled by default to prevent accidental deployment during testing

---

## Task Synchronization

### Event Groups

**Startup Event:**
```c
EventGroupHandle_t task_startup_status;
#define STARTUP_DONE_BIT (1 << 0)

// Startup task sets bit when done
xEventGroupSetBits(task_startup_status, STARTUP_DONE_BIT);

// Other tasks wait for startup
xEventGroupWaitBits(task_startup_status, STARTUP_DONE_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
```

**Telecommand Event:**
```c
EventGroupHandle_t tc_event_group;
#define TC_AVAILABLE_BIT (1 << 0)

// Read TTC task sets bit when packet available
xEventGroupSetBits(tc_event_group, TC_AVAILABLE_BIT);

// Process TC task waits for bit
xEventGroupWaitBits(tc_event_group, TC_AVAILABLE_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
```

---

### Mutexes

**Data Buffer Protection:**
```c
SemaphoreHandle_t sat_data_mutex;

// Create mutex in main()
sat_data_mutex = xSemaphoreCreateMutex();

// Acquire before accessing sat_data_buf
xSemaphoreTake(sat_data_mutex, portMAX_DELAY);
// ... access sat_data_buf ...
xSemaphoreGive(sat_data_mutex);
```

---

## Task Timing Analysis

### CPU Utilization Estimate

| Task | Period | Execution Time | CPU % |
|------|--------|----------------|-------|
| Watchdog Reset | 100 ms | ~0.1 ms | 0.1% |
| Heartbeat | 1000 ms | ~0.2 ms | 0.02% |
| Read Sensors | 100 ms | ~2 ms | 2% |
| Read EPS | 60000 ms | ~10 ms | 0.02% |
| Read TTC | 60000 ms | ~10 ms | 0.02% |
| Data Log | 300000 ms | ~50 ms | 0.02% |
| Beacon | 60000 ms | ~20 ms | 0.03% |

**Total Estimated CPU:** ~2-5% (mostly idle)

**Idle Task:** FreeRTOS idle task runs when no other tasks are ready, allowing low-power modes.

---

## Task Configuration

**File:** `config/config.h`

```c
// Task enable/disable flags
#define TASK_STARTUP_ENABLED            1
#define TASK_WATCHDOG_RESET_ENABLED     1
#define TASK_HEARTBEAT_ENABLED          1
#define TASK_SYSTEM_RESET_ENABLED       1
#define TASK_READ_SENSORS_ENABLED       1
#define TASK_TIME_CONTROL_ENABLED       1
#define TASK_READ_EPS_ENABLED           1
#define TASK_READ_TTC_ENABLED           1
#define TASK_READ_ANTENNA_ENABLED       1
#define TASK_READ_EDC_ENABLED           1
#define TASK_DATA_LOG_ENABLED           1
#define TASK_PROCESS_TC_ENABLED         1
#define TASK_BEACON_ENABLED             1
#define TASK_ANTENNA_DEPLOY_ENABLED     0  // Disabled by default
```

**Task Creation (in main.c):**
```c
#if TASK_STARTUP_ENABLED == 1
    xTaskCreate(startup_task, "Startup", 350, NULL, 5, NULL);
#endif

#if TASK_WATCHDOG_RESET_ENABLED == 1
    xTaskCreate(watchdog_reset_task, "WDT", 150, NULL, 1, NULL);
#endif

// ... (other tasks)
```

---

## Related Documentation

- `00_functionalities_of_system.md` - System overview
- `01_firmware_architecture.md` - Firmware architecture
- `02_datapath.md` - Data structures
- `03_interfaces.md` - Communication interfaces
- `05_pin_mapping.md` - Pin assignments
- `06_hardware_components_and_architecture.md` - Hardware details
