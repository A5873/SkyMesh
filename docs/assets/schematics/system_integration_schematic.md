# SkyMesh System Integration - Technical Schematic

## Overview

This document describes the system integration architecture for the SkyMesh satellite, detailing the interconnections between all major subsystems: Power Distribution Board (PDB), Onboard Computer (OBC) Carrier Board, Radio Frequency (RF) Frontend, and Attitude Determination and Control System (ADCS). The integration design emphasizes reliable power distribution, robust data interfaces, proper signal routing, and thermal management for the 3U CubeSat form factor.

## Integration Architecture

The SkyMesh satellite follows a stacked PCB architecture with the following arrangement (from -Z to +Z):

1. Solar Panel Assembly (Bottom)
2. Power Distribution Board
3. OBC Carrier Board (Core Computing Module)
4. RF Frontend
5. ADCS Board
6. Solar Panel Assembly (Top)

Side panels (+/-X, +/-Y) contain additional solar panels, deployable UHF/VHF antennas, and S-band patch antennas.

## Interconnection Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              SOLAR PANELS                                    │
│                          │                │                                  │
│                       ┌──▼──┐          ┌──▼──┐                              │
│                       │ +X  │          │ +Y  │                              │
│                       └──┬──┘          └──┬──┘                              │
│                          │                │                                  │
└──────────────────────────┼────────────────┼──────────────────────────────────┘
                           │                │
                  ┌────────▼────────────────▼───────────┐
                  │                                     │
                  │      POWER DISTRIBUTION BOARD       │◄───── Battery Pack
                  │                                     │       (Internal)
                  │ ┌─────────┐  ┌─────────┐  ┌──────┐ │
                  │ │ 3.3V    │  │ 5V      │  │ 12V  │ │
                  │ │ Reg     │  │ Reg     │  │ Boost│ │
                  │ └────┬────┘  └────┬────┘  └───┬──┘ │
                  │      │            │           │    │
                  └──────┼────────────┼───────────┼────┘
                         │            │           │
       ┌─────────────────┼────────────┼───────────┼─────────────────┐
       │  ┌──────────────┼────────────┼───────────┼───────────────┐ │
       │  │              │            │           │               │ │
       │  │           ┌──▼──┐      ┌──▼──┐     ┌──▼──┐           │ │
       │  │           │3.3V │      │ 5V  │     │ 12V │           │ │
       │  │           └──┬──┘      └──┬──┘     └──┬──┘           │ │
       │  │              │            │           │               │ │
       │  │     OBC CARRIER BOARD (COMPUTE MODULE)                │ │
       │  │                                                       │ │
       │  │  ┌───────────┐ ┌───────────┐ ┌──────────────────┐    │ │
       │  │  │ CM4 SOC   │ │ SmartFus. │ │ Memory & Storage │    │ │
       │  │  │ (Primary) │ │ (Backup)  │ │                  │    │ │
       │  │  └─────┬─────┘ └─────┬─────┘ └──────────────────┘    │ │
       │  │        │             │                               │ │
       │  │        │  ┌──────────┴────────────┐                  │ │
       │  │        │  │                       │                  │ │
       │  │  ┌─────▼──▼───────┐    ┌──────────▼──────────────┐   │ │
       │  │  │ Data Interface │    │ System Control & Mgmt   │   │ │
       │  │  │ ▪ SPI          │    │ ▪ Control               │   │ │
       │  │  │ ▪ I2C          │    │ ▪ System Monitors      │   │ │
       │  │  │ ▪ UART         │    │ ▪ Watchdog             │   │ │
       │  │  └───────┬─────┬──┘    └──────────┬─────────────┘   │ │
       │  │          │     │                  │                 │ │
       │  └──────────┼─────┼──────────────────┼─────────────────┘ │
       │             │     │                  │                   │
       └─────────────┼─────┼──────────────────┼───────────────────┘
                     │     │                  │
┌────────────────────┼─────┼──────────────────┼───────────────────────┐
│  ┌──────────────┬──┼─────┼──────────────────┼───────────────────┐   │
│  │              │  │     │                  │                   │   │
│  │           ┌──▼──┼─────▼──┐            ┌──▼──┐               │   │
│  │           │Data │Control │            │Power│               │   │
│  │           └──┬──┼────────┘            └──┬──┘               │   │
│  │              │  │                        │                   │   │
│  │                RF FRONTEND BOARD                             │   │
│  │                                                              │   │
│  │  ┌───────────────┐ ┌───────────────┐  ┌───────────────────┐  │   │
│  │  │ UHF/VHF       │ │ S-Band        │  │ Signal Processing │  │   │
│  │  │ Transceiver   │ │ Transceiver   │  │                   │  │   │
│  │  └─────┬─────────┘ └─────┬─────────┘  └───────────────────┘  │   │
│  │        │                 │                                    │   │
│  │     ┌──▼─────────────────▼──┐                                │   │
│  │     │ RF Routing & Switching│                                │   │
│  │     └──┬─────────────────┬──┘                                │   │
│  │        │                 │                                    │   │
│  └────────┼─────────────────┼────────────────────────────────────┘   │
│           │                 │                                        │
│     ┌─────▼─────┐     ┌─────▼─────┐                                  │
│     │ UHF/VHF   │     │ S-Band    │                                  │
│     │ Antenna   │     │ Antenna   │                                  │
│     └───────────┘     └───────────┘                                  │
└───────────────────────────────────────────────────────────────────────┘

┌───────────────────────────────────────────────────────────────────────┐
│  ┌──────────────┬────────────────────┬───────────────────────────┐    │
│  │              │                    │                           │    │
│  │           ┌──▼──┐              ┌──▼──┐                        │    │
│  │           │Data │              │Power│                        │    │
│  │           └──┬──┘              └──┬──┘                        │    │
│  │              │                    │                           │    │
│  │                  ADCS BOARD                                   │    │
│  │                                                               │    │
│  │  ┌────────────────┐  ┌─────────────────┐  ┌──────────────┐    │    │
│  │  │ Sensors        │  │ Processing      │  │ Actuators    │    │    │
│  │  │ ▪ Sun          │  │ ▪ STM32H7       │  │ ▪ Motor      │    │    │
│  │  │ ▪ Mag          │  │ ▪ Control Alg.  │  │   Drivers    │    │    │
│  │  │ ▪ Gyro         │  │                 │  │ ▪ Coil       │    │    │
│  │  └────────────────┘  └─────────────────┘  │   Drivers    │    │    │
│  │                                            └──────────────┘    │    │
│  │                                                                │    │
│  └────────────────────────────────────────────────────────────────┘    │
│                                                                        │
│                             Reaction                                   │
│                             Wheels                                     │
└───────────────────────────────────────────────────────────────────────┘
```

## Power Distribution Paths

### Primary Power Supply Routing

The Power Distribution Board (PDB) is the central power management system with the following power distribution paths:

1. **3.3V Bus (Digital Logic)**
   - Source: TPS62142 Buck Converter on PDB
   - Current Capacity: 3A maximum
   - Subsystem Distribution:
     - OBC: 800mA (nominal), 1.2A (peak)
     - RF Frontend: 500mA (nominal)
     - ADCS: 400mA (nominal), 600mA (peak)
   - Protection: Individual polyfuses for each subsystem line
   - Monitoring: INA226 current sensors per output

2. **5V Bus (Interface Power)**
   - Source: TPS62110 Buck Converter on PDB
   - Current Capacity: 2A maximum
   - Subsystem Distribution:
     - OBC: 500mA
     - RF Frontend: 800mA (nominal), 1.2A (peak)
     - ADCS: 300mA (nominal), 1A (peak)
   - Protection: TPS25944 eFuse with current limit
   - Enable Control: GPIO from OBC

3. **12V Bus (RF & Actuation)**
   - Source: TPS61088 Boost Converter on PDB
   - Current Capacity: 1A maximum
   - Subsystem Distribution:
     - RF Frontend: 350mA (for power amplifiers)
     - ADCS: 400mA (for reaction wheels)
   - Protection: Current-limited switches
   - Enable Control: Power sequencing from OBC

### Power Sequencing

The power-up sequence follows this order to ensure system stability:
1. 3.3V Power Rail (Core Logic)
2. 5V Power Rail (Interfaces)
3. 12V Power Rail (Actuators & RF)

Power-down sequence is the reverse, controlled by the MAX16050 sequencer on the PDB.

### Backup Power

Critical systems (OBC core, RF beacon) have direct emergency power lines from the battery through the PDB with:
- Diode-OR power combining
- Supercapacitor backup for safe shutdown (2x 1F)
- Low-voltage disconnect at 6.2V (battery protection)

## Data Interfaces

### Primary Data Bus

The I²C bus serves as the primary system management bus:
- Bus Master: OBC CM4
- Bus Speed: 400 kHz
- Pull-up Resistors: 4.7kΩ to 3.3V
- Connected Devices:
  - PDB: Status monitoring, power control
  - RF Frontend: Configuration, status reporting
  - ADCS: Command and telemetry

### High-Speed Interfaces

1. **SPI Bus (OBC to RF Frontend)**
   - Clock: 20 MHz
   - Mode: 0 (CPOL=0, CPHA=0)
   - CS Lines: Dedicated per device
   - Connected Devices:
     - UHF/VHF Transceiver (CS0)
     - S-band Transceiver (CS1)
   - Signal Integrity: Series termination resistors (33Ω)

2. **SPI Bus (OBC to ADCS)**
   - Clock: 10 MHz
   - Mode: 0 (CPOL=0, CPHA=0)
   - CS Lines: Dedicated per sensor bank
   - Connected Devices:
     - Gyroscope (CS0)
     - Magnetometer (CS1)
   - Signal Integrity: Series termination resistors (33Ω)

3. **UART Debug Interfaces**
   - Baud Rate: 115200 bps
   - Voltage Level: 3.3V
   - Flow Control: None
   - Usage: Debug and maintenance only

### Isolation and Protection

All inter-board data connections include:
- TVS diodes for ESD protection
- Series resistance (33Ω) for current limiting
- Critical interfaces (RF to OBC) include digital isolators (ISO1540)

## RF Signal Routing

### UHF/VHF System

- **Antenna Connection**:
  - Dual deployable dipole antennas (VHF - 2m band, UHF - 70cm band)
  - Connection to RF Frontend via U.FL connectors
  - Mounting: Deployable mechanism on -Z face

- **RF Routing**:
  - Semi-rigid coaxial cable (RG-405)
  - Impedance: 50Ω
  - Loss: < 0.5dB
  - Strain relief at board connections

### S-band System

- **Antenna Connection**:
  - Patch antenna array on +X and +Y faces
  - Connection to RF Frontend via U.FL connectors
  - Mounting: Fixed to structural panels

- **RF Routing**:
  - Semi-rigid coaxial cable (RG-405)
  - Impedance: 50Ω
  - Loss: < 1.0dB at 2.4GHz
  - RF shielding around connectors and routing paths

## Control and Monitoring Lines

### System Control

1. **Reset Lines**:
   - Master Reset: OBC to all subsystems
   - Warm Reset: Subsystem specific
   - Implementation: Open-drain with pull-up

2. **Fault Detection**:
   - Subsystem Fault Lines to OBC
   - Watchdog signals (bi-directional between OBC and critical subsystems)
   - Over-temperature warnings

3. **Mode Control**:
   - Power mode signals (3 GPIO lines)
   - Operational

