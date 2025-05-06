# SkyMesh System Architecture

## Overview

SkyMesh is a decentralized orbital mesh network composed of low-cost nano-satellites, each powered by radiation-tolerant single-board computers running a custom lightweight satellite OS. The system is designed with three primary layers:

1. **Orbital Layer**: A constellation of CubeSat-class nanosatellites in low Earth orbit (LEO)
2. **Protocol Layer**: Software stack enabling secure mesh communication
3. **Ground Layer**: Network of terrestrial stations and user gateways

## System Diagram

```
                     [ORBITAL LAYER]
                           /|\
                          / | \
                         /  |  \
                        /   |   \
     ★ Satellite  ★ Satellite  ★ Satellite
           \         |         /
            \        |        /
             \       |       /
              \      |      /
               \     |     /
                \    |    /
             [PROTOCOL LAYER]
         (Mesh Routing & Security)
                /    |    \
               /     |     \
              /      |      \
             /       |       \
            /        |        \
      [GROUND LAYER]
      /      |      \
     /       |       \
    /        |        \
┌─────────┐ ┌─────────┐ ┌─────────┐
│ Gateway │ │ Gateway │ │ Gateway │
└────┬────┘ └────┬────┘ └────┬────┘
     |           |           |
     └───────────┼───────────┘
                 |
       ┌─────────┴─────────┐
       │   End Users/IoT    │
       └───────────────────┘
```

## Orbital Layer

### Satellite Constellation

- **Platform**: CubeSat-class nanosatellites (1U-6U form factors)
- **Orbit**: Low Earth Orbit (LEO), multiple orbital planes for global coverage
- **Formation**: Self-organizing mesh network with dynamic routing capabilities
- **Typical Altitude**: 400-600 km
- **Number of Satellites**: 
  - Phase 1: 6-12 satellites (minimum viable constellation)
  - Phase 2: 30-50 satellites (regional coverage)
  - Phase 3: 100+ satellites (global coverage)

### Satellite Hardware Architecture

```
┌─────────────────────────────────────────────────┐
│                  NANOSATELLITE                  │
│                                                 │
│  ┌───────────────┐       ┌───────────────────┐  │
│  │ Power System  │◄─────►│  Radiation-        │  │
│  │ Solar + Batt  │       │  Hardened SBC     │  │
│  └───────┬───────┘       └─────────┬─────────┘  │
│          │                         │            │
│          │      ┌──────────────────┼────────┐   │
│          │      │                  │        │   │
│          ▼      ▼                  ▼        ▼   │
│  ┌─────────────┐  ┌────────────┐  ┌──────────┐  │
│  │ Attitude &  │  │ Transceiver│  │Payload   │  │
│  │ Control Sys │  │ UHF/S-band │  │(Optional)│  │
│  └─────────────┘  └────────────┘  └──────────┘  │
└─────────────────────────────────────────────────┘
```

### Onboard Computer

- **Primary Computer**:
  - Development: Radiation-tolerant COTS (Commercial Off-The-Shelf) SBC
  - Production: Custom-designed radiation-hardened SoC with RISC-V architecture
  - Memory: ECC RAM with radiation-tolerant design
  - Storage: Redundant flash storage with error correction
  
- **Power Requirements**:
  - 2-5W operational power
  - Solar panel array with battery backup
  - Power management subsystem with sleep modes

- **Thermal Management**:
  - Passive thermal control system
  - Operating temperature range: -40°C to +85°C

## Protocol Layer

### SkyMesh Network Protocol Stack

```
┌─────────────────────────────────────────┐
│       APPLICATION LAYER                 │
│  ┌─────────────┐  ┌─────────────────┐   │
│  │ Data Relay  │  │ IoT/Sensor Data │   │
│  └─────────────┘  └─────────────────┘   │
├─────────────────────────────────────────┤
│       MESH ROUTING LAYER                │
│  ┌─────────────┐  ┌─────────────────┐   │
│  │ Dynamic     │  │ Store & Forward │   │
│  │ Routing     │  │ Protocol        │   │
│  └─────────────┘  └─────────────────┘   │
├─────────────────────────────────────────┤
│       LINK LAYER                        │
│  ┌─────────────┐  ┌─────────────────┐   │
│  │ Error       │  │ Flow Control    │   │
│  │ Correction  │  │                 │   │
│  └─────────────┘  └─────────────────┘   │
├─────────────────────────────────────────┤
│       PHYSICAL LAYER                    │
│  ┌─────────────┐  ┌─────────────────┐   │
│  │ UHF/VHF/    │  │ S-band          │   │
│  │ Transceivers│  │ Communications  │   │
│  └─────────────┘  └─────────────────┘   │
└─────────────────────────────────────────┘
```

### Key Protocol Features

- **Mesh Networking Protocol**: Custom delay-tolerant networking protocol
- **Node Discovery**: Automatic satellite-to-satellite discovery mechanism
- **Routing Algorithm**: Dynamic path optimization based on:
  - Link quality
  - Power availability
  - Orbital position
  - Network congestion
- **Store and Forward**: Data persistence during connectivity gaps
- **Encryption**: End-to-end encryption for all transmissions
- **QoS Management**: Priority-based packet scheduling
- **Error Correction**: Forward Error Correction (FEC) optimized for space environment

## Ground Layer

### Ground Station Architecture

```
┌──────────────────────────────────────────────────┐
│               GROUND STATION                     │
│                                                  │
│  ┌───────────────┐       ┌───────────────────┐   │
│  │ Tracking      │◄─────►│ Control Computer  │   │
│  │ Antenna System│       │ & Network Server  │   │
│  └───────┬───────┘       └─────────┬─────────┘   │
│          │                         │             │
│          │       ┌─────────────────┘             │
│          │       │                               │
│          ▼       ▼                               │
│  ┌─────────────┐ ┌────────────┐  ┌────────────┐  │
│  │ Radio       │ │ Power &    │  │ Internet   │  │
│  │ Transceivers│ │ Backup Sys │  │ Uplink     │  │
│  └─────────────┘ └────────────┘  └────────────┘  │
└──────────────────────────────────────────────────┘
```

### Ground Station Types

1. **Professional Stations**:
   - High-gain directional antenna systems
   - Multiple frequency band capabilities
   - 24/7 operation with redundant systems
   - Used for primary network uplink/downlink

2. **Community Gateways**:
   - Medium-gain antenna systems
   - Lower cost, deployable by technical users
   - Regional coverage and relay capabilities
   - Open-source design

3. **Personal User Terminals**:
   - Low-cost, portable systems
   - Limited capabilities (primarily downlink)
   - Emergency communication focus
   - Mobile and IoT device connectivity

### Ground Network Services

- **Uplink/Downlink Management**: Scheduled communication with satellites
- **Data Routing**: Connection to internet backbone and local networks
- **Local Caching**: Store-and-forward for disconnected operation
- **Emergency Services**: Priority channels for disaster response
- **API Gateway**: Developer access to network capabilities

## Integration Architecture

The three-layer architecture provides:

1. **Resilience**: No single point of failure
2. **Scalability**: Incremental deployment of satellites and ground stations
3. **Accessibility**: Multiple tiers of access points
4. **Autonomy**: Self-organizing network that adapts to changing conditions

This architecture enables SkyMesh to provide global communication coverage with minimal ground infrastructure, making it ideal for emergency services, remote areas, and as a resilient backup to terrestrial networks.

