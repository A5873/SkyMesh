# SkyMesh OBC Carrier Board - Technical Schematic

## Overview

This document describes the Onboard Computer (OBC) Carrier Board schematic for the SkyMesh satellite. The OBC is based on the Raspberry Pi Compute Module 4 (CM4) with additional radiation protection circuitry, redundant memory systems, and robust monitoring capabilities designed for operation in a space environment.

## Key Specifications

- Compute: Raspberry Pi CM4 (4GB RAM, 32GB eMMC)
- Radiation Protection: Triple redundant monitoring and voting logic
- Memory: External radiation-tolerant NAND flash with ECC
- Power Input: 3.3V, 5V from Power Distribution Board
- Interfaces: I2C, SPI, UART, GPIO, PCIe, USB
- Dimensions: 90mm x 95mm
- PCB: 8-layer with radiation shielding
- Operating Temperature: -40°C to +85°C

## Block Diagram

```
                  ┌────────────────────────────────────────────────┐
                  │              POWER MANAGEMENT                  │
                  │                                                │
                  │ ┌────────────┐   ┌────────────┐   ┌──────────┐ │
                  │ │ 3.3V Input │   │ 5V Input   │   │1.8V LDO  │ │
                  │ │ Protection │   │ Protection │   │TPS7A0133 │ │
                  │ └─────┬──────┘   └──────┬─────┘   └────┬─────┘ │
                  │       │                 │              │       │
                  │       ▼                 ▼              ▼       │
┌──────────┐      │ ┌────────────┐   ┌────────────┐   ┌──────────┐ │      ┌───────────┐
│          │      │ │ Power      │   │ Triple     │   │ Backup   │ │      │           │
│  Power   │ 3.3V │ │ Sequencing │   │ Redundant  │   │ Power    │ │      │  Status   │
│  Distri- ├─────►│ │ MAX16050   │   │ Monitoring │   │ Circuit  │ │      │  LEDs     │
│  bution  │      │ └─────┬──────┘   └──────┬─────┘   └────┬─────┘ │      │           │
│  Board   │ 5V   │       │                 │              │       │      └─────┬─────┘
│          ├─────►│       │                 │              │       │            │
└──────────┘      └───────┼─────────────────┼──────────────┼───────┘            │
                          │                 │              │                    │
                          ▼                 ▼              ▼                    ▼
     ┌────────────────────────────────────────────────────────────────────────────┐
     │                                                                            │
     │                     COMPUTE MODULE INTERFACE                               │
     │                                                                            │
     │  ┌──────────────────────────────────────────────────────────────────────┐  │
     │  │                                                                      │  │
     │  │                    Raspberry Pi Compute Module 4                     │  │
     │  │                                                                      │  │
     │  │    ┌───────────┐  ┌─────────────┐  ┌──────────┐  ┌──────────────┐    │  │
     │  │    │ BCM2711   │  │ 4GB LPDDR4  │  │ 32GB     │  │ Wireless     │    │  │
     │  │    │ CPU       │  │ RAM         │  │ eMMC     │  │ (disabled)   │    │  │
     │  │    └─────┬─────┘  └─────┬───────┘  └────┬─────┘  └──────────────┘    │  │
     │  │          │              │               │                            │  │
     │  └──────────┼──────────────┼───────────────┼────────────────────────────┘  │
     │             │              │               │                               │
     │             ▼              ▼               ▼                               │
     │  ┌─────────────────────────────────────────────────────────────────────┐   │
     │  │                       DDR4 SODIMM Connector                         │   │
     │  └─────────────────────────────────────────────────────────────────────┘   │
     │             │              │               │                               │
     └─────────────┼──────────────┼───────────────┼───────────────────────────────┘
                   │              │               │
                   ▼              ▼               ▼
     ┌────────────────────────────────────────────────────────────────────────────┐
     │                           RADIATION PROTECTION                             │
     │                                                                            │
     │  ┌────────────────┐    ┌─────────────────┐    ┌───────────────────────┐    │
     │  │ Triple         │    │ Watchdog        │    │ Reset Management      │    │
     │  │ Modular        │    │ MAX6369         │    │ Supervisor           │    │
     │  │ Redundancy     │    │ Cascade         │    │ ADM6320              │    │
     │  └───────┬────────┘    └────────┬────────┘    └──────────┬────────────┘    │
     │          │                      │                        │                  │
     │          ▼                      ▼                        ▼                  │
     │  ┌──────────────────────────────────────────────────────────────────────┐  │
     │  │                      Voting Logic & Control                          │  │
     │  │                      FPGA: Microsemi SmartFusion2                    │  │
     │  └──────────────────────────────────────────────────────────────────────┘  │
     │          │                      │                        │                  │
     └──────────┼──────────────────────┼────────────────────────┼──────────────────┘
                │                      │                        │
                ▼                      ▼                        ▼
     ┌────────────────────────────────────────────────────────────────────────────┐
     │                            MEMORY INTERFACES                               │
     │                                                                            │
     │  ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────────┐    │
     │  │ Radiation-      │    │ Secure          │    │ External RAM        │    │
     │  │ Tolerant Flash  │    │ Boot ROM        │    │ with ECC            │    │
     │  │ S70FL01GS       │    │ SST38VF6401     │    │ IS46DR16320D        │    │
     │  └───────┬─────────┘    └────────┬────────┘    └──────────┬──────────┘    │
     │          │                       │                        │                │
     │  ┌───────▼───────────────────────▼────────────────────────▼──────────┐    │
     │  │                     Error Detection & Correction                   │    │
     │  │                           Logic                                    │    │
     │  └────────────────────────────────────────────────────────────────────┘    │
     │                                   │                                        │
     └───────────────────────────────────┼────────────────────────────────────────┘
                                         │
                                         ▼
     ┌────────────────────────────────────────────────────────────────────────────┐
     │                          EXTERNAL INTERFACES                               │
     │                                                                            │
     │  ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────────┐    │
     │  │ GPIO with       │    │ PCIe            │    │ System              │    │
     │  │ Protection      │    │ Interface       │    │ Monitoring          │    │
     │  └───────┬─────────┘    └────────┬────────┘    └──────────┬──────────┘    │
     │          │                       │                        │                │
     │  ┌───────▼─────┐    ┌────────────▼───────┐    ┌───────────▼─────────┐     │
     │  │ Isolation   │    │ PCIe Switch        │    │ Temp Sensors        │     │
     │  │ Buffers     │    │ PEX 8605           │    │ & ADC               │     │
     │  └───────┬─────┘    └────────┬───────────┘    └───────────┬─────────┘     │
     │          │                   │                            │               │
     │          ▼                   ▼                            ▼               │
     │  ┌──────────────┐    ┌───────────────┐              ┌─────────────┐      │
     │  │ I2C/SPI/UART │    │ USB Interfaces │              │ Debug       │      │
     │  │ Headers      │    │ & Headers      │              │ Headers     │      │
     │  └──────────────┘    └───────────────┘              └─────────────┘      │
     └────────────────────────────────────────────────────────────────────────────┘
```

## Detailed Circuit Design

### 1. Power Management Section

The power management section receives 3.3V and 5V from the Power Distribution Board and includes:

- **Input Protection**:
  - TVS diodes (SMAJ5.0A) for transient voltage protection
  - Polyfuses (PTS120616V050) for overcurrent protection
  - EMI filters (BNX002-01) for noise suppression

- **Power Sequencing**:
  - MAX16050 sequencer controlling power-up/down order
  - Sequence: 3.3V core → 1.8V I/O → 5V peripherals
  - Power good monitoring with 10ms delay between stages

- **Triple Redundant Monitoring**:
  - Three independent TPS3840 voltage supervisors
  - Voting logic implemented in FPGA
  - Monitors all critical rails (3.3V, 1.8V, 5V)
  - Automatic reset on voltage anomalies

- **Backup Power Circuit**:
  - Super capacitors (2x 1F) for short-term power backup
  - LTC3643 supercapacitor charger/balancer
  - Enables safe shutdown during power interruptions

### 2. Compute Module Interface

The compute module interface is designed for the Raspberry Pi CM4:

- **DDR4 SODIMM Connector**:
  - 260-pin MEC6-RA connector compatible with CM4
  - Impedance-controlled traces (90Ω differential, 50Ω single-ended)
  - 1V8 termination power with separate filtering

- **Signal Integrity**:
  - PCB length matching for critical signals
  - Ground planes for signal integrity
  - Series termination resistors for high-speed signals (33Ω)
  - Differential pair routing for PCIe, USB, and HDMI

- **Power Filtering**:
  - Local decoupling capacitors (0.1μF, 1μF, 10μF)
  - Bulk capacitance (100μF per power rail)
  - Low-ESR capacitors for high-frequency noise rejection

### 3. Radiation Protection Section

The radiation protection circuitry is critical for reliable operation in space:

- **Triple Modular Redundancy (TMR)**:
  - Three identical processing paths for critical functions
  - Microsemi SmartFusion2 M2S025 FPGA for voting logic
  - Radiation-hardened by design (RHBD) techniques
  - Scrubbing of FPGA configuration memory

- **Watchdog Circuits**:
  - MAX6369 watchdog timer cascade (three in series)
  - Nominal timeout: 1.6s → 100ms → 10ms
  - Automatic reset if watchdog not serviced
  - Independent from main CPU to ensure reliability

- **Reset Management**:
  - ADM6320 supervisor with manual reset input
  - Glitch filtering on reset signals
  - Reset signal distribution with fan-out buffer
  - Power-on reset circuit with delayed enable

- **SEU (Single Event Upset) Mitigation**:
  - Triple voting on critical control signals
  - Error detection and correction for memory
  - Periodic refresh of configuration registers
  - Current limiting resistors on I/O lines

### 4. Memory Interfaces

The memory subsystem provides radiation-tolerant storage:

- **Radiation-Tolerant Flash**:
  - Cypress S70FL01GS 1Gbit NOR Flash
  - SPI interface with quad-data support
  - Built-in ECC for single-bit error correction
  - Redundant storage with checksum verification

- **Secure Boot ROM**:
  - SST38VF6401 64Mbit flash for boot code
  - Write-protected after programming
  - Redundant copies of boot code
  - CRC verification before execution

- **External RAM with ECC**:
  - Integrated Device Technology IS46DR16320D
  - 512MB DDR3 with on-chip ECC
  - Supports scrubbing operations
  - Refresh rate monitoring and control

- **Error Detection & Correction**:
  - Hardware ECC implementation in FPGA
  - Reed-Solomon code for multi-bit error detection
  - Periodic memory scrubbing
  - Error logging and reporting

### 5. External Interfaces

The OBC connects to other satellite subsystems via:

- **GPIO with Protection**:
  - Level shifters (TXB0108) for voltage translation
  - Current limiting resistors (series 100Ω)
  - ESD protection diodes (TPD4E05U06)
  - Opto-isolators (6N137) for critical signals

- **PCIe Interface**:
  - PEX 8605 PCIe switch
  - x1 Gen2 link to CM4
  - Expandable to 4 endpoints
  - Clock buffer and filtering

- **System Monitoring**:
  - TMP175 temperature sensors (5 locations)
  - INA226 current/voltage monitors
  - MAX11645 ADC for analog sensor inputs
  - Fault logging in non-volatile memory

- **Communication Headers**:
  - I2C bus with isolation (ISO1540)
  - SPI with protection circuitry
  - UART console

