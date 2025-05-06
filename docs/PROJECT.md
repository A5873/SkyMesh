# SkyMesh Project Documentation

## Architecture Overview

SkyMesh employs a multi-layered architecture designed to create a resilient, decentralized orbital mesh network:

### 1. Orbital Layer

The orbital layer consists of a constellation of CubeSat-class nanosatellites deployed in Low Earth Orbit (LEO). These satellites:

- Operate at altitudes between 400-600km
- Are organized in multiple orbital planes for optimal coverage
- Utilize radiation-tolerant single-board computers (SBCs)
- Form dynamic peer-to-peer connections based on visibility and orbital mechanics
- Autonomously reconfigure network topology as satellites move in their orbits

### 2. Protocol Layer

The protocol layer provides the networking foundation for communications between all system components:

- Custom mesh routing protocol optimized for orbital dynamics
- Delay/disruption-tolerant networking capabilities
- End-to-end encryption for all transmissions
- Quality-of-service management for prioritizing critical data
- Store-and-forward mechanisms for handling connectivity gaps
- Bandwidth-efficient data compression and transmission

### 3. Ground Layer

The ground layer connects Earth-based users and systems to the orbital network:

- Open-source ground station software for uplink/downlink operations
- Gateway nodes that provide internet backhaul connections
- User-deployable antenna designs for different capability/cost points
- Edge caching for improved efficiency and reduced latency
- Application interfaces for third-party service integration

## Subsystem Implementation

### Satellite Operating System

The SkyMesh OS (SMOS) is a lightweight, real-time capable operating system optimized for resource-constrained, radiation-exposed environments:

- Microkernel architecture with minimal trusted computing base
- Real-time thread scheduling for critical operations
- Process isolation to contain radiation-induced errors
- Redundant state management across multiple memory regions
- File system with journaling and corruption detection
- Over-the-air update system with fallback safeguards
- Minimal power states: Full, Limited, Beacon, and Hibernation

Implementation:
```rust
// Example of SMOS power management module
pub enum PowerState {
    Full,      // All systems operational
    Limited,   // Non-critical systems powered down
    Beacon,    // Only periodic beacon transmission
    Hibernate, // Minimal power consumption, wake on timer
}

pub struct PowerManager {
    current_state: PowerState,
    battery_level: f32, // 0.0 to 1.0
    solar_input: f32,   // Watts
    power_budget: HashMap<String, f32>,
}

impl PowerManager {
    pub fn transition(&mut self, target_state: PowerState) -> Result<(), PowerError> {
        // State transition logic with safety checks
        // ...
    }
    
    pub fn get_estimated_runtime(&self) -> Duration {
        // Calculate remaining operational time based on current state
        // ...
    }
}
```

### RF Communications

The communication subsystem manages all radio frequency transmissions between satellites and ground stations:

- UHF/VHF (433/145 MHz) low-bandwidth control channels
- S-band (2.4 GHz) high-bandwidth data transmission
- Software-defined radio capabilities for flexible modulation
- Adaptive data rates based on link quality and distance
- Multiple antenna designs for different communication patterns
- Interference detection and avoidance

Implementation:
```c
// Example of S-band transmitter control
typedef struct {
    uint32_t frequency_hz;
    uint8_t modulation_scheme;  // BPSK, QPSK, etc.
    uint8_t coding_rate;        // 1/2, 3/4, etc.
    uint16_t bandwidth_khz;
    int8_t tx_power_dbm;
    uint32_t symbol_rate;
} rf_config_t;

rf_status_t rf_configure_transmitter(rf_config_t* config) {
    // Configure SDR parameters
    // ...
    
    // Verify power constraints
    if (config->tx_power_dbm > MAX_TX_POWER_DBM) {
        return RF_ERROR_POWER_EXCEEDED;
    }
    
    // Apply settings to hardware
    // ...
    
    return RF_STATUS_OK;
}
```

### Power Management

The power subsystem ensures efficient energy generation, storage, and distribution:

- Triple-junction solar panels with maximum power point tracking
- Lithium-ion battery packs with radiation shielding
- Intelligent load shedding based on power budget and priorities
- Battery health monitoring and cycle management
- Power distribution with overcurrent protection
- Thermal management integrated with power systems

Implementation:
```python
# Example of power budget calculation
class PowerBudget:
    def __init__(self):
        self.subsystems = {}
        self.available_power = 0.0
        self.required_power = 0.0
        
    def register_subsystem(self, name, nominal_power, priority):
        self.subsystems[name] = {
            'nominal': nominal_power,
            'current': nominal_power,
            'priority': priority,
            'active': True
        }
        self.required_power += nominal_power
        
    def update_available_power(self, solar_input, battery_level):
        # Calculate available power based on solar input and battery state
        # ...
        
    def balance_budget(self):
        """Shed loads if required power exceeds available power"""
        if self.required_power <= self.available_power:
            return True
            
        # Sort subsystems by priority (lower number = higher priority)
        sorted_systems = sorted(self.subsystems.items(), 
                               key=lambda x: x[1]['priority'])
        
        # Shed loads until budget is balanced
        for name, system in reversed(sorted_systems):
            if not system['active'] or system['priority'] <= 2:  # Never shed critical systems
                continue
                
            system['active'] = False
            self.required_power -= system['current']
            if self.required_power <= self.available_power:
                return True
                
        return False  # Could not balance budget
```

### Health Monitoring System

The health monitoring subsystem continuously assesses satellite status:

- Sensor data collection from all subsystems
- Anomaly detection using simple machine learning models
- Automated error recovery procedures
- Performance metric tracking and trend analysis
- Telemetry compression for efficient downlink
- Long-term health records for fleet management

Implementation:
```c++
// Example of health monitoring system
class HealthMonitor {
private:
    std::vector<Sensor*> sensors;
    CircularBuffer<HealthRecord> history;
    AnomalyDetector anomalyDetector;
    
public:
    void registerSensor(Sensor* sensor) {
        sensors.push_back(sensor);
    }
    
    HealthStatus performHealthCheck() {
        HealthRecord record;
        record.timestamp = getCurrentTime();
        
        for (auto sensor : sensors) {
            SensorReading reading = sensor->read();
            record.readings[sensor->getId()] = reading;
            
            if (anomalyDetector.isAnomaly(sensor->getId(), reading)) {
                triggerAlert(sensor->getId(), reading);
            }
        }
        
        history.add(record);
        return evaluateOverallHealth(record);
    }
    
    TelemetryPacket generateTelemetry() {
        // Compress and package health data for transmission
        // ...
    }
};
```

### Orbital Task Manager

The task manager coordinates satellite operations based on orbital position:

- Orbital prediction and position awareness
- Scheduling of location-specific tasks
- Contact planning for satellite-to-satellite links
- Ground station visibility prediction
- Coordination of distributed observations
- Resource allocation across the constellation

Implementation:
```java
// Example of orbital task scheduling
public class OrbitalTaskScheduler {
    private final Map<UUID, Task> pendingTasks = new HashMap<>();
    private final OrbitalPredictor predictor;
    
    public void scheduleTask(Task task) {
        TaskConstraints constraints = task.getConstraints();
        
        // Find next execution window based on orbital mechanics
        List<TimeWindow> windows = predictor.findExecutionWindows(
            constraints.getGeographicConstraints(),
            constraints.getVisibilityRequirements(),
            constraints.getEnergyRequirements(),
            task.getDuration()
        );
        
        if (windows.isEmpty()) {
            task.setStatus(TaskStatus.UNSCHEDULABLE);
            return;
        }
        
        // Assign task to best window
        TimeWindow bestWindow = optimizeWindowSelection(windows);
        task.setExecutionWindow(bestWindow);
        pendingTasks.put(task.getId(), task);
    }
    
    public List<Task> getActiveTasksForCurrentPosition() {
        Position currentPosition = predictor.getCurrentPosition();
        return pendingTasks.values().stream()
            .filter(task -> task.isActiveAt(currentPosition))
            .collect(Collectors.toList());
    }
}
```

## Radiation Hardening Approach

### Triple Modular Redundancy (TMR)

SkyMesh employs TMR at multiple levels to mitigate single-event upsets:

- Critical computations performed three times and majority-voted
- Redundant memory regions for program code and critical data
- Redundant microcontrollers for system-critical functions
- Distributed consensus across multiple satellites for network-level decisions

Implementation:
```c
// Example of TMR memory protection
typedef struct {
    uint32_t data_a;
    uint32_t data_b;
    uint32_t data_c;
} tmr_uint32_t;

uint32_t tmr_read(tmr_uint32_t* value) {
    if (value->data_a == value->data_b && value->data_a == value->data_c) {
        return value->data_a;  // All values agree
    } else if (value->data_a == value->data_b) {
        value->data_c = value->data_a;  // Correct the divergent copy
        return value->data_a;
    } else if (value->data_a == value->data_c) {
        value->data_b = value->data_a;  // Correct the divergent copy
        return value->data_a;
    } else if (value->data_b == value->data_c) {
        value->data_a = value->data_b;  // Correct the divergent copy
        return value->data_b;
    } else {
        // Triple disagreement - major error case
        trigger_error_recovery();
        return ERROR_VALUE;
    }
}

void tmr_write(tmr_uint32_t* value, uint32_t new_data) {
    value->data_a = new_data;
    value->data_b = new_data;
    value->data_c = new_data;
}
```

### Error Detection and Correction

Beyond TMR, several error detection and correction mechanisms are employed:

- Error-Correcting Code (ECC) memory for single-bit error correction
- Cyclic Redundancy Checks (CRC) for data transmission and storage
- Watchdog timers to detect processor lockups
- Periodic memory scrubbing to correct accumulated errors
- System-wide checksums for configuration validation
- Safe mode triggers when error rates exceed thresholds

Implementation:
```c
// Example of memory scrubbing
void memory_scrub_task(void* parameters) {
    memory_region_t* regions = (memory_region_t*)parameters;
    
    while (1) {
        for (int i = 0; regions[i].address != NULL; i++) {
            uint32_t* start = (uint32_t*)regions[i].address;
            size_t words = regions[i].size / sizeof(uint32_t);
            
            for (size_t j = 0; j < words; j++) {
                uint32_t value = start[j];
                uint32_t ecc = compute_ecc(value);
                
                if (ecc != start[j + words]) {  // ECC mismatch
                    if (can_correct_ecc(value, ecc)) {
                        start[j] = correct_from_ecc(value, ecc);
                        start[j + words] = ecc;
                        log_corrected_error(&start[j]);
                    } else {
                        log_uncorrectable_error(&start[j]);
                    }
                }
            }
        }
        
        // Sleep for a while before next scrubbing pass
        task_delay(SCRUB_INTERVAL_MS);
    }
}
```

### Radiation-Aware Software Design

Software design practices that enhance radiation tolerance:

- Stateless design where possible to minimize corruption impact
- Regular state validation against known-good values
- Defensive programming with bounds checking and input validation
- Control flow monitoring to detect execution path errors
- Strategic use of EEPROM/Flash for critical configuration storage
- Graceful degradation paths for all subsystems

## Communication Protocols

### Physical Layer

UHF/VHF Communication:
- Frequency: 433 MHz (UHF) / 145 MHz (VHF) amateur satellite bands
- Modulation: GMSK or 2-FSK with robust coding
- Data rates: 1.2 - 9.6 kbps
- Link budget: Designed for reliable communication at up to 2500 km
- Primary use: Command, control, and telemetry

S-band Communication:
- Frequency: 2400-2450 MHz in ISM band
- Modulation: OQPSK with adaptive coding
- Data rates: 100 kbps - 2 Mbps depending on link quality
- Link budget: Optimized for satellite-to-satellite and high-quality ground links
- Primary use: Mesh data transfer and high-volume downlink

### Network Layer

The SkyMesh network protocol provides:

- Dynamic topology discovery with orbital awareness
- Position-based routing optimizations
- Store-and-forward capabilities for disruption tolerance
- Quality of Service with prioritization for critical traffic
- Congestion control adapted to orbital dynamics
- Multicast support for efficient data distribution

Implementation:
```python
# Example of mesh routing protocol
class MeshRoutingTable:
    def __init__(self):
        self.routes = {}  # destination -> next_hop
        self.metrics = {}  # destination -> metric
        self.position_data = {}  # node_id -> (position, velocity, timestamp)
        
    def update_position(self, node_id, position, velocity, timestamp):
        self.position_data[node_id] = (position, velocity, timestamp)
        self._recalculate_predicted_routes()
        
    def _recalculate_predicted_routes(self):
        """Recalculates routes based on current and predicted positions"""
        current_time = get_current_time()
        
        # Predict positions 5 minutes into the future
        future_positions = {}
        for node, (pos, vel, ts) in self.position_data.items():
            # Simple linear extrapolation - actual implementation uses orbital mechanics
            dt = current_time - ts
            predicted_pos = pos + vel * (dt + 300)  # current + 5min prediction
            future_positions[node] = predicted_pos
        
        # Calculate routes based on current and predicted connectivity
        # ...
        
    def get_next_hop(self, destination, message_priority):
        if destination in self.routes:
            return self.routes[destination]
        # Fall back to store-and-forward if no route available
        

