# SkyMesh: Technical Specification

<div align="center">
  <img src="../assets/logos/skymesh-logo-full-color.svg" width="400" alt="SkyMesh Logo">
  <p><em>Technical Specification Document v1.0</em></p>
</div>

## Table of Contents

- [I. Executive Abstract](#i-executive-abstract)
- [II. Background & Motivation](#ii-background--motivation)
- [III. System Design Overview](#iii-system-design-overview)
- [IV. Mission Profile](#iv-mission-profile)
- [V. Software Stack](#v-software-stack)
- [VI. Feasibility Analysis](#vi-feasibility-analysis)
- [VII. Future Directions](#vii-future-directions)
- [VIII. Appendices](#viii-appendices)

---

## I. Executive Abstract

SkyMesh is a decentralized orbital mesh network composed of low-cost nanosatellites, designed to enable censorship-resistant global communications. The system employs a constellation of CubeSat-class devices in low Earth orbit (LEO), creating a resilient, self-healing network infrastructure that operates independently of ground-based internet systems.

This technical specification outlines the hardware, software, and operational architecture of the SkyMesh project, providing a comprehensive roadmap for development, testing, and deployment. The document serves as both a blueprint for implementation and a reference for ongoing research and development efforts.

Key innovations of the SkyMesh project include:

- Custom radiation-hardened satellite operating system optimized for mesh networking
- Novel inter-satellite communication protocol designed for LEO constraints
- Low-cost, modular hardware architecture based on commercial off-the-shelf (COTS) components
- Ground station design emphasizing accessibility and ease of deployment
- Open protocol specifications enabling third-party device integration

---

## II. Background & Motivation

### Current Infrastructure Limitations

Traditional satellite communication infrastructure suffers from several critical limitations:

- **Centralized Control**: Existing satellite networks are typically owned and operated by a single entity, creating potential points of failure and control.
- **High Costs**: Traditional communication satellites cost hundreds of millions to build and launch, limiting deployment scale.
- **Limited Coverage**: Geostationary satellites provide limited coverage at polar regions, while existing LEO constellations prioritize commercial interests over universal access.
- **Vulnerability to Censorship**: Government or corporate control over ground stations enables communication monitoring or blocking.
- **Complex Ground Equipment**: Most satellite communication requires expensive, specialized equipment for ground access.

### The SkyMesh Solution

SkyMesh addresses these limitations through:

- **Decentralized Architecture**: No single control point for the network, ensuring censorship resistance.
- **Nanosatellite Form Factor**: Using the CubeSat standard (3U configuration) drastically reduces construction and launch costs.
- **COTS Hardware**: Leveraging commercial single-board computers (SBCs) with additional radiation mitigation rather than expensive radiation-hardened components.
- **Custom Operating System**: Purpose-built OS that optimizes for the unique constraints of space-based mesh networking.
- **Open Protocol**: Publicly documented communication protocols that enable third-party device integration and community participation.
- **Simple Ground Stations**: Designed to be constructed from readily available components, lowering barriers to network access.

---

## III. System Design Overview

### Satellite Hardware Layout

The SkyMesh satellite employs a 3U CubeSat form factor (10×10×30 cm) with the following major subsystems:

![Satellite Hardware Layout](../assets/diagrams/satellite_hardware_layout.svg)

The satellite hardware consists of:

- **Onboard Computer (OBC)**: Raspberry Pi Compute Module 4 with custom carrier board
  - ARM Cortex-A72 quad-core processor at 1.5 GHz (underclocked to 1.0 GHz for thermal management)
  - 8GB LPDDR4-3200 SDRAM with ECC implementation
  - 32GB eMMC with radiation-tolerant file system
  - Custom radiation shielding and watchdog circuitry

- **Communication Subsystem**:
  - Primary UHF/VHF transceiver for ground communication (AFSK/GMSK modulation)
  - Secondary S-band transceiver for inter-satellite links (QPSK/OQPSK modulation)
  - Deployable dipole antennas for UHF/VHF
  - Patch antenna array for S-band directional communication
  - Software-defined radio (SDR) capabilities for adaptive modulation schemes
  
![Communication System Diagram](../assets/diagrams/communication_system_diagram.svg)

- **Power Subsystem**:
  - Deployable solar panels generating 30W peak power
  - 60 Wh lithium-ion battery pack with custom battery management system (BMS)
  - Maximum power point tracking (MPPT) charge controllers
  - Power distribution unit with overcurrent protection
  - Fault detection and recovery circuits

![Power Subsystem Diagram](../assets/diagrams/power_subsystem_diagram.svg)

- **Attitude Determination and Control System (ADCS)**:
  - Magnetorquers for coarse orientation control
  - Reaction wheels for fine pointing
  - Sun sensors and magnetometers for orientation determination
  - Gyroscope for rotational measurement
  - ADCS microcontroller running deterministic control algorithms

- **Structure and Thermal Management**:
  - Aluminum 6061-T6 frame with radiation shielding
  - Passive thermal control with selective surface coatings
  - Heat pipes connecting high-heat components to radiator panels
  - Thermal sensors distributed throughout satellite

### Mechanical Layout

The 3U CubeSat design follows standard dimensions while optimizing internal component placement for thermal management and center of mass considerations:

![Mechanical Layout](../assets/diagrams/mechanical_layout.svg)

### Ground Station Architecture

The SkyMesh ground station design prioritizes affordability and ease of construction while maintaining reliable satellite communication capabilities:

![Ground Station Architecture](../assets/diagrams/ground_station_architecture.svg)

Components include:
- Directional Yagi-Uda antenna for UHF/VHF reception with motorized azimuth/elevation control
- Software-defined radio (RTL-SDR or HackRF) for signal processing
- Single-board computer (Raspberry Pi 4) running ground station software
- Optional power amplifier for improved uplink capability
- Internet gateway for backhaul when available

### Satellite OS Architecture

SkyMesh OS is a custom real-time operating system designed specifically for the constraints and requirements of orbital mesh networking:

![Satellite OS Architecture](../assets/diagrams/satellite_os_architecture.svg)

Key OS components include:
- Microkernel architecture with minimal trusted computing base
- Real-time scheduler optimized for communication tasks
- Fault detection, isolation, and recovery (FDIR) system
- Memory protection with radiation-tolerant error correction
- Secure boot and software verification chain
- Low-power modes with priority-based wake mechanisms

```c
// Example of the SkyMesh OS task scheduling priority mechanism
typedef enum {
    PRIORITY_CRITICAL_COMMS = 0,    // Highest priority - maintain basic comms
    PRIORITY_SYSTEM_HEALTH = 1,     // System monitoring and error correction
    PRIORITY_SCHEDULED_COMMS = 2,   // Regular communication windows
    PRIORITY_MESH_ROUTING = 3,      // Mesh network packet routing
    PRIORITY_HOUSEKEEPING = 4,      // General maintenance tasks
    PRIORITY_PAYLOAD = 5            // Lowest priority - payload operations
} TaskPriority;

// Task initialization with safety-critical design
TaskHandle_t create_radiation_tolerant_task(
    void (*task_function)(void*),
    const char *name,
    uint32_t stack_size,
    void *parameters,
    TaskPriority priority,
    uint32_t redundancy_level
) {
    // Triple modular redundancy for critical tasks
    if (redundancy_level > 1) {
        register_redundant_task(task_function, parameters, redundancy_level);
    }
    
    // Memory protection boundary setup
    MemoryRegion_t protected_region = allocate_protected_memory(stack_size);
    
    // Watchdog registration
    uint32_t watchdog_id = register_task_watchdog(name, priority);
    
    return scheduler_create_task(task_function, name, protected_region, 
                                parameters, priority, watchdog_id);
}
```

---

## IV. Mission Profile

### Orbit Parameters

SkyMesh satellites will operate in a Low Earth Orbit (LEO) constellation with the following characteristics:

- **Orbit Type**: Sun-Synchronous Orbit (SSO)
- **Altitude**: 550 kilometers
- **Inclination**: 97.6 degrees
- **Period**: 95.4 minutes
- **Orbital Shells**: 
  - Initial deployment: Single shell with 12 satellites
  - Full constellation: 3 shells with 60 satellites total
- **Ground Track**: Repeating ground track with 14-16 orbits per day

### Expected Lifetime

The expected operational lifetime of each SkyMesh satellite is:
- **Minimum Mission Duration**: 2 years
- **Extended Mission Potential**: 3-5 years
- **Limiting Factors**: Radiation effects on electronics, battery degradation, and orbit decay

### Data Throughput

The network is designed to provide the following performance characteristics:

- **Ground-to-Satellite Link**: 
  - Uplink: 9.6 kbps using standard amateur equipment
  - Downlink: 19.2 kbps using standard amateur equipment
  - Enhanced downlink: Up to 100 kbps with specialized ground stations

- **Inter-Satellite Links**:
  - Nominal: 100 kbps with QPSK modulation
  - Burst capability: Up to 1 Mbps for short durations when power budget allows

- **Network Capacity**:
  - Initial constellation: ~50 MB per day per ground region
  - Full constellation: ~500 MB per day per ground region

- **Latency**:
  - Store-and-forward mode: Minutes to hours
  - Real-time mode: 500 ms to 2 seconds (when satellite-to-satellite connectivity permits)

### Deployment Phases

The SkyMesh deployment strategy follows a phased approach:

1. **Phase I: Technology Demonstration (6-12 months)**
   - Launch of 2 prototype satellites
   - Validation of core hardware and software systems
   - Initial testing of satellite-to-ground communications
   - Limited service for technical partners

2. **Phase II: Initial Constellation (12-18 months)**
   - Deployment of first orbital shell (12 satellites)
   - Implementation of inter-satellite mesh networking
   - Public beta service with basic messaging capabilities
   - Global coverage with revisit times of 90-120 minutes

3. **Phase III: Full Deployment (18-36 months)**
   - Launch of remaining orbital shells (48 additional satellites)
   - Full mesh network implementation
   - Commercial service activation
   - Global coverage with revisit times of 30-45 minutes

4. **Phase IV: Expansion (36+ months)**
   - Incremental replacement of aging satellites
   - Launch of enhanced satellites with upgraded capabilities
   - Increase in constellation density for improved capacity
   - Integration with partner networks and services

---

## V. Software Stack

### Operating System

SkyMesh satellites run a custom RTOS designed specifically for the constraints of space-based mesh networking:

- **Kernel**: Microkernel architecture with minimal privileged code
- **Scheduling**: Mixed criticality scheduler with hard real-time guarantees for critical tasks
- **Memory Management**: 
  - Protected memory spaces with hardware isolation where available
  - Triple Modular Redundancy (TMR) for critical memory structures
  - Scrubbing of memory with periodic Error Detection and Correction (EDAC)
- **File System**: Log-structured file system with radiation-tolerant journaling
- **Power Management**: Dynamic voltage and frequency scaling based on power availability

```c
// Example of the radiation-tolerant file system implementation
typedef struct {
    uint32_t block_id;
    uint32_t block_id_copy;    // Redundant copy for verification
    uint16_t data_size;
    uint16_t data_size_copy;   // Redundant copy for verification
    uint32_t timestamp;
    uint32_t crc32;            // CRC of the data section
    uint8_t data[BLOCK_DATA_SIZE];
    uint32_t ecc[ECC_WORDS_PER_BLOCK]; // Error correction codes
} FileSystemBlock;

int fs_write_block_redundant(FileSystemBlock *block, void *data, size_t size) {
    if (size > BLOCK_DATA_SIZE) return FS_ERROR_SIZE;
    
    // Fill primary data
    block->block_id = next_block_id();
    block->block_id_copy = block->block_id;
    block->data_size = size;
    block->data_size_copy = size;
    block->timestamp = system_time_get();
    
    // Copy data with verification
    memcpy(block->data, data, size);
    if (memcmp(block->data, data, size) != 0) return FS_ERROR_MEMCOPY;
    
    // Generate CRC
    block->crc32 = calculate_crc32(block->data, size);
    
    // Generate ECC codes
    generate_ecc_codes(block->data, size, block->ecc);
    
    // Write to primary storage
    int result = storage_write_physical_block(block);
    
    // Write redundant copy to backup location
    if (result == FS_SUCCESS) {
        result = storage_write_physical_block_backup(block);
    }
    
    return result;
}
```

### Communication Protocols

SkyMesh implements a layered communication protocol stack:

1. **Physical Layer**:
   - UHF/VHF: AFSK/GMSK modulation compliant with amateur radio standards
   - S-band: QPSK/OQPSK with forward error correction

2. **Data Link Layer**:
   - AX.25 for ground-to-satellite communication
   - Custom SkyMesh Link Protocol (SLP) for inter-satellite communication
   - Adaptive error correction based on link quality

3. **Network Layer**:
   - Mesh routing protocol optimized for orbital dynamics
   - Store-and-forward packet handling
   - Predictive routing based on orbital mechanics
   - Disruption-tolerant networking protocols

4. **Transport Layer**:
   - Reliable message delivery with selective acknowledgment
   - Quality of Service (QoS) with priority-based queuing
   - End-to-end encryption with forward secrecy

5. **Application Layer**:
   - Messaging API for text-based communication
   - Binary data transfer protocol
   - Time synchronization service
   - Network status and health reporting

### Software Update and Resilience

The SkyMesh software architecture includes multiple mechanisms for maintaining operational integrity:

- **OTA Updates**: 
  - Redundant bootloader with fallback capability
  - Atomic updates with verification before activation
  - Differential updates to minimize bandwidth usage

- **Watchdog Systems**:
  - Hardware watchdog for system-level recovery
  - Task-level watchdogs for individual process monitoring
  - Hierarchical recovery procedures based on failure severity

- **Self-Repair**:
  - Automated detection and correction of memory errors
  - Software component redundancy with voting mechanisms
  - Periodic full system verification and integrity checks

### Ground Node Software

Ground stations run an open-source software suite that includes:

- **Radio Interface**: 
  - SDR drivers with optimized signal processing
  - Automatic frequency control for Doppler correction
  - Adaptive modulation recognition

- **Network Stack**:
  - Implementation of SkyMesh protocols
  - Local message store with synchronization logic
  - Gateway functionality for internet backhaul when available

- **User Interface**:
  - Web-based administration console
  - Messaging client with store-and-forward capability
  - Network status visualization
  - REST API for third-party application integration

---

## VI. Feasibility Analysis

### Radiation Mitigation Strategies

SkyMesh employs multiple approaches to mitigate radiation effects common in the LEO environment:

- **Hardware Shielding**:
  - Aluminum 6061-T6 outer frame providing primary radiation protection
  - Spot shielding for radiation-sensitive components using tantalum and tungsten
  - Stacked PCB approach with ground planes for additional protection
  
- **Component Selection**:
  - Commercial components pre-screened for radiation tolerance
  - Overrating of components (voltage, current, temperature) for margin
  - Redundant components for critical subsystems
  
- **Circuit Design Techniques**:
  - Watchdog timers and reset circuits to recover from Single Event Upsets (SEUs)
  - Current-limiting resistors to prevent Single Event Latchups (SELs)
  - Isolated power domains to contain failure propagation
  - Radiation-tolerant power supply design with transient protection

- **Software Strategies**:
  - Triple Modular Redundancy (TMR) for critical code execution
  - Memory scrubbing with Error Detection and Correction (EDAC)
  - Cyclic Redundancy Checks (CRC) for all stored data
  - Periodic system state validation and recovery

```c
// Example implementation of Triple Modular Redundancy for critical operations
uint32_t tmr_execute_critical_operation(uint32_t (*operation)(uint32_t), uint32_t input) {
    // Execute operation three times independently
    uint32_t result1 = operation(input);
    uint32_t result2 = operation(input);
    uint32_t result3 = operation(input);
    
    // Voting mechanism
    if (result1 == result2) return result1;
    if (result1 == result3) return result1;
    if (result2 == result3) return result2;
    
    // Log disagreement and fallback strategy
    log_radiation_event(RADIATION_TMR_MISMATCH, input, result1, result2, result3);
    
    // Return majority result or trigger recovery procedure
    return majority_vote(result1, result2, result3);
}
```

### Thermal/Environmental Tolerances

The SkyMesh satellite is designed to operate within the following environmental parameters:

- **Temperature Range**:
  - Operational: -20°C to +60°C for all components
  - Survival: -40°C to +80°C
  - Temperature gradient: Maximum 20°C/minute during eclipse transitions

- **Thermal Management**:
  - Passive thermal control using selective surface materials
  - Heat pipes connecting high-dissipation components to radiator panels
  - Multi-layer insulation (MLI) on non-radiator surfaces
  - Thermal sensors providing feedback to the power management system

- **Vacuum Compatibility**:
  - All components vacuum-rated or encapsulated
  - Outgassing requirements: Total Mass Loss (TML) < 1.0%, Collected Volatile Condensable Material (CVCM) < 0.1%
  - Thermal testing in vacuum conditions prior to launch

- **Vibration and Shock**:
  - Operational vibration: 0.5g RMS
  - Launch vibration tolerance: 14g RMS
  - Shock tolerance: 1500g, 0.5ms half-sine pulse

Thermal simulations and vacuum chamber testing confirm that under worst-case conditions (maximum power draw in direct sunlight or minimum power in eclipse), all components remain within their operational temperature ranges.

### Launch Plan and Regulatory Path

#### Launch Vehicle Compatibility

SkyMesh satellites are designed to be compatible with multiple launch vehicles as secondary payloads:

- **Compatible Launch Vehicles**:
  - SpaceX Falcon 9 (rideshare program)
  - Rocket Lab Electron
  - Virgin Orbit LauncherOne
  - ISRO PSLV
  - Other small satellite launch providers

- **Deployment Systems**:
  - Compatible with standard CubeSat deployment systems:
    - NanoRacks CubeSat Deployer
    - Planetary Systems Corporation Canisterized Satellite Dispenser
    - ISIPOD CubeSat Deployer
    - ISIS QuadPack

- **Orbit Insertion Requirements**:
  - Target orbit: 550 km, sun-synchronous
  - Acceptable range: 500-600 km altitude
  - Inclination: 97.6° ± 0.5°

#### Regulatory Compliance

The SkyMesh constellation requires several regulatory approvals and has been designed to comply with all relevant laws and standards:

- **Frequency Allocations**:
  - UHF/VHF amateur radio bands (coordinated through IARU)
  - S-band experimental license for inter-satellite links
  - All transmissions use licensed spectrum with appropriate coordination

- **Space Debris Mitigation**:
  - Compliant with Inter-Agency Space Debris Coordination Committee (IADC) guidelines
  - Post-mission disposal plan: Natural deorbit within 25 years (calculated: approximately 7-10 years)
  - End-of-life maneuver: Passivation of all energy sources
  - No debris release during normal operations

- **Export Controls**:
  - ITAR/EAR compliance for technology transfer
  - Open source elements carefully scoped to avoid controlled technologies
  - Appropriate licensing for international team collaboration

- **National Licensing**:
  - Filing with national authorities (FCC in US, UK Space Agency, etc.)
  - Registration with United Nations Registry of Objects Launched into Outer Space
  - Compliance with liability provisions of Outer Space Treaty

---

## VII. Future Directions

### Inter-Satellite Routing Advancement

Future development of the SkyMesh constellation will focus on enhanced inter-satellite routing capabilities:

- **Predictive Link State Protocol (PLSP)**:
  - Orbital mechanics-aware routing protocol that predicts connectivity windows
  - Pre-calculation of optimal routing paths based on constellation state
  - Dynamic packet prioritization based on anticipated link availability
  - Link quality estimation incorporating environmental factors

- **Mesh Density Optimization**:
  - Analysis of optimal satellite density for different coverage requirements
  - Variable constellation architecture with multiple orbital planes
  - Adaptive coverage focusing on high-demand regions
  - Heterogeneous constellation with specialized satellites for backbone routes

- **Protocol Enhancements**:
  - Extension of DTN (Disruption-Tolerant Networking) protocols
  - Cross-layer optimization between physical, link, and network layers
  - Quality of Service guarantees with bandwidth reservations
  - Multi-path routing with load balancing

### AI and Onboard ML Inference

SkyMesh plans to incorporate AI and machine learning capabilities in future satellite generations:

- **Autonomous Operation**:
  - Self-managing constellation with minimal ground intervention
  - Anomaly detection and response without human input
  - Dynamic resource allocation based on usage patterns
  - Predictive failure prevention through component telemetry analysis

- **Onboard Inference**:
  - Low-power ML accelerators for edge processing
  - Computer vision for Earth observation applications
  - Signal classification for interference detection
  - Compression algorithms optimized for transmitted data

- **Distributed Learning**:
  - Federated learning across the constellation
  - Model updates shared during satellite contacts
  - Collaborative sensing and distributed intelligence
  - Swarm behavior algorithms for constellation optimization

```python
# Example future ML-based anomaly detection
class SatelliteAnomalyDetector:
    def __init__(self, model_path, confidence_threshold=0.85):
        self.model = load_quantized_model(model_path)
        self.threshold = confidence_threshold
        self.telemetry_history = RollingBuffer(capacity=1000)
        
    def process_telemetry(self, telemetry_vector):
        # Add to history buffer
        self.telemetry_history.add(telemetry_vector)
        
        # Extract features for inference
        features = self.extract_features(self.telemetry_history)
        
        # Run inference on optimized model
        anomaly_score, anomaly_type = self.model.infer(features)
        
        if anomaly_score > self.threshold:
            # Proactively mitigate the detected issue
            self.trigger_mitigation(anomaly_type)
            
            # Report to ground when communication available
            self.queue_anomaly_report(anomaly_type, anomaly_score, telemetry_vector)
```

### Application Examples and Use Cases

The SkyMesh platform will enable a variety of applications once fully deployed:

- **Offline Messaging and Communication**:
  - Resilient global text messaging, even in remote areas
  - Emergency communications during natural disasters
  - Connectivity for humanitarian operations in infrastructure-limited regions
  - Censorship-resistant communication channels

- **Internet of Things (IoT) Connectivity**:
  - Low-power, long-range IoT data collection for remote sensors
  - Environmental monitoring in remote regions (forests, oceans, polar areas)
  - Asset tracking for global supply chains
  - Agricultural monitoring in developing regions

- **Distributed Time Synchronization**:
  - Precision time protocol distribution
  - Alternative to GPS for time synchronization
  - Byzantine fault-tolerant distributed clock
  - Secure timestamping for distributed applications

- **Scientific Applications**:
  - Distributed radio telescope (interferometry)
  - Space weather monitoring
  - Upper atmosphere research
  - Low-cost Earth observation platform

Each of these applications will be enabled through open APIs and protocol specifications, allowing third-party developers to build on the SkyMesh platform.

---

## VIII. Appendices

### Appendix A: Technical Schematics

Detailed technical schematics for all major subsystems can be found in the following files:

- **Power Subsystem**: [Power Distribution Board Schematic](../assets/schematics/power_distribution_schematic.pdf)
- **OBC**: [Main Computer Carrier Board](../assets/schematics/obc_carrier_schematic.pdf)
- **Communications**: [RF Frontend Design](../assets/schematics/rf_frontend_schematic.pdf)
- **ADCS**: [Attitude Control System](../assets/schematics/adcs_schematic.pdf)

### Appendix B: OS Kernel Architecture

The SkyMesh OS kernel architecture follows a layered design with formal verification of critical components:

```
┌──────────────────────────────────────────────────┐
│                 Applications                      │
├──────────────────────────────────────────────────┤
│             IPC / Message Bus                     │
├──────────┬───────────────┬───────────┬───────────┤
│ File     │ Network       │ Device    │ Memory    │
│ System   │ Stack         │ Drivers   │ Manager   │
├──────────┴───────────────┴───────────┼───────────┤
│           Hardware Abstraction Layer │ Security  │
├──────────────────────────────────────┼ Monitor   │
│             Microkernel              │           │
├──────────────────────────────────────┴───────────┤
│                  Hardware                         │
└──────────────────────────────────────────────────┘
```

Key principles of the kernel design:

1. **Minimal TCB (Trusted Computing Base)**:
   - Only essential services run in privileged mode
   - Formal verification of critical kernel components
   - Strong isolation between kernel and user space

2. **Real-Time Guarantees**:
   - Bounded interrupt latency (< 5 μs)
   - Deterministic scheduling with priority inheritance
   - Predictable memory allocation

3. **Radiation Tolerance**:
   - Kernel state checkpointing
   - Safe restart mechanisms
   - Redundant execution paths for critical operations

The kernel source code is available in the [satellite-os/kernel](https://github.com/skymesh/satellite-os) repository.

### Appendix C: Power Budget Calculations

Detailed power budget calculations for different operational modes:

| Subsystem         | Peak Power (W) | Duty Cycle (%) | Avg Power (W) |
|-------------------|----------------|----------------|---------------|
| OBC               | 3.5            | 100            | 3.5           |
| S-band Comms TX   | 8.0            | 5              | 0.4           |
| S-band Comms RX   | 1.0            | 25             | 0.25          |
| UHF/VHF Comms TX  | 5.0            | 5              | 0.25          |
| UHF/VHF Comms RX  | 0.8            | 40             | 0.32          |
| ADCS              | 2.2            | 80             | 1.76          |
| Payload           | 1.5            | 30             | 0.45          |
| Thermal Control   | 1.0            | 20             | 0.2           |
| Power System      | 0.7            | 100            | 0.7           |
| **Total**         | **23.7**       | **-**          | **7.83**      |

**Power Generation:**

- Solar array effective area: 0.06 m² × 4 panels = 0.24 m²
- Solar energy conversion efficiency: 30%
- Solar flux at LEO: ~1367 W/m²
- Generated power (normal incidence): 0.24 m² × 1367 W/m² × 30% = 98.4 W
- Average generation considering orbit and attitude: ~30 W

**Energy Storage:**

- Battery capacity: 60 Wh
- Maximum eclipse duration: 35 minutes
- Required eclipse power: 7.83 W × 35/60 h = 4.57 Wh
- Depth of discharge limit: 20%
- Energy margin: (60 Wh × 20% - 4.57 Wh) / 4.57 Wh = 162%

### Appendix D: RF Frequency Plan

The SkyMesh satellites operate on the following frequencies:

**UHF/VHF Ground Communications:**
- Uplink: 144-146 MHz (2m amateur band)
- Downlink: 435-438 MHz (70cm amateur band)
- Modulation: AFSK at 1200 baud, GMSK at 9600 baud
- Bandwidth: 15 kHz per channel
- TX Power: 5W maximum, software adjustable

**S-band Inter-satellite Links:**
- Frequency: 2400-2450 MHz
- Modulation: QPSK/OQPSK
- Bandwidth: 1 MHz
- Data rate: 100 kbps nominal, 1 Mbps burst
- TX Power: 2W maximum, software adjustable

**Frequency Coordination:**
- Amateur frequency coordination through IARU
- Non-interference verification per ITU regulations
- Dynamic frequency selection to avoid interference
- All transmissions digitally signed for authentication

The detailed link budget calculations and fade margins are available in the [RF Link Budget spreadsheet](../assets/calculations/rf_link_budget.xlsx).

