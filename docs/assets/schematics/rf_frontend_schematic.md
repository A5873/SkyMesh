# SkyMesh RF Frontend - Technical Schematic

## Overview

This document describes the Radio Frequency (RF) Frontend schematic for the SkyMesh satellite. The RF system provides dual-band communication capability with UHF/VHF for ground station links and S-band for inter-satellite mesh networking. The design emphasizes radiation tolerance, power efficiency, and reliable operation in the space environment.

## Key Specifications

- **UHF/VHF System**:
  - Frequency: 435-438 MHz (downlink), 145-146 MHz (uplink)
  - Modulation: GMSK/AFSK (1200/9600 baud)
  - Power Output: 0.5W to 5W (adjustable)
  - Sensitivity: -120 dBm

- **S-band System**:
  - Frequency: 2400-2450 MHz
  - Modulation: QPSK/OQPSK
  - Data Rate: 100 kbps to 1 Mbps
  - Power Output: 0.1W to 2W (adjustable)

- **Power Requirements**:
  - 5V @ 1.2A (peak TX)
  - 3.3V @ 500mA (digital logic)
  - 12V @ 350mA (RF amplifiers)

- **Size**: 80mm x 90mm PCB
- **Radiation Tolerance**: Total Dose > 15 kRad
- **Temperature Range**: -40°C to +85°C

## Block Diagram

```
                                                ┌────────────────┐
                                                │  Deployable    │
                                     ┌─────────►│  Dipole        │
                                     │          │  UHF/VHF       │
                                     │          │  Antenna       │
                                     │          └────────────────┘
                                     │
┌─────────────────────────────────────────────────────────────────────┐
│                                    │                                 │
│                     RF SWITCHING & ROUTING                           │
│                                    │                                 │
│   ┌────────────────┐     ┌─────────▼────────┐    ┌────────────────┐ │
│   │                │     │                   │    │                │ │
│   │  Diplexer      │◄───►│   RF Switch      │◄───►  S-band Patch  │ │
│   │  MACOM         │     │   SKY13330       │    │  Antenna Array │ │
│   │  MAPD-011027   │     │                   │    │                │ │
│   └───────┬────────┘     └─────────┬────────┘    └────────────────┘ │
│           │                        │                                 │
└───────────┼────────────────────────┼─────────────────────────────────┘
            │                        │
            │                        │
┌───────────▼────────────┐ ┌─────────▼─────────────────────────────────┐
│                        │ │                                            │
│     UHF/VHF SYSTEM     │ │              S-BAND SYSTEM                │
│                        │ │                                            │
│ ┌────────────────────┐ │ │ ┌────────────────────┐  ┌───────────────┐ │
│ │                    │ │ │ │                    │  │               │ │
│ │  RF Frontend       │ │ │ │  RF Frontend       │  │ LO Generator  │ │
│ │  ▪ LNA: SKY67151   │ │ │ │  ▪ LNA: PMA2-83LN │  │ MAX2871       │ │
│ │  ▪ Bandpass Filter │ │ │ │  ▪ Bandpass Filter │  │               │ │
│ │  ▪ RF Switching    │ │ │ │  ▪ RF Switching    │  └───────┬───────┘ │
│ └─────────┬──────────┘ │ │ └─────────┬──────────┘          │         │
│           │            │ │           │                     │         │
│ ┌─────────▼──────────┐ │ │ ┌─────────▼──────────┐  ┌───────▼───────┐ │
│ │                    │ │ │ │                    │  │               │ │
│ │  Transceiver       │ │ │ │  Transceiver       │  │ Up/Down       │ │
│ │  AX5043            │ │ │ │  AD9361            │  │ Converter     │ │
│ │  ▪ UHF/VHF Modem   │ │ │ │  ▪ RF Agile        │  │ ADL5801       │ │
│ │  ▪ FSK/GMSK        │ │ │ │    Transceiver     │  │               │ │
│ └─────────┬──────────┘ │ │ └─────────┬──────────┘  └───────────────┘ │
│           │            │ │           │                               │
│ ┌─────────▼──────────┐ │ │ ┌─────────▼──────────┐                    │
│ │                    │ │ │ │                    │                    │
│ │  Power Amplifier   │ │ │ │  Power Amplifier   │                    │
│ │  RFPA1011         │ │ │ │  RFPA3800          │                    │
│ │  ▪ UHF: 5W         │ │ │ │  ▪ S-band: 2W      │                    │
│ │  ▪ Class AB        │ │ │ │  ▪ Power Control   │                    │
│ └──────────┬─────────┘ │ │ └──────────┬─────────┘                    │
│            │           │ │            │                              │
└────────────┼───────────┘ └────────────┼──────────────────────────────┘
             │                          │
             │                          │
┌────────────▼──────────────────────────▼──────────────────────────────┐
│                                                                       │
│                         SIGNAL PROCESSING                             │
│                                                                       │
│ ┌─────────────────────┐   ┌─────────────────────┐  ┌───────────────┐ │
│ │                     │   │                     │  │               │ │
│ │  ADC/DAC            │   │  Digital Signal     │  │  Buffer       │ │
│ │  Interface          │   │  Processing         │  │  Memory       │ │
│ │  ▪ ADC: AD9629      │   │  ▪ FPGA: Microsemi  │  │  IS45S16320D │ │
│ │  ▪ DAC: AD9744      │   │    SmartFusion2     │  │               │ │
│ └──────────┬──────────┘   └──────────┬──────────┘  └───────┬───────┘ │
│            │                         │                     │         │
└────────────┼─────────────────────────┼─────────────────────┼─────────┘
             │                         │                     │
             ▼                         ▼                     ▼
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                         CONTROL INTERFACE                           │
│                                                                     │
│ ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────────┐   │
│ │                 │  │                 │  │                     │   │
│ │  OBC Interface  │  │ Power Management│  │ Status Monitoring   │   │
│ │  ▪ SPI: 20MHz   │  │ ▪ DC-DC: TPS62111│  │ ▪ LM75 Temp Sensors│   │
│ │  ▪ I2C: 400kHz  │  │ ▪ LDO: TPS7A8300│  │ ▪ INA219 Power Mon │   │
│ └────────┬────────┘  └────────┬────────┘  └─────────┬───────────┘   │
│          │                    │                     │               │
│          ▼                    ▼                     ▼               │
│ ┌─────────────────────────────────────────────────────────────────┐ │
│ │                   Fault Protection & Isolation                   │ │
│ │      ▪ TVS Arrays   ▪ Current Limiting   ▪ Thermal Shutdown     │ │
│ └─────────────────────────────────────────────────────────────────┘ │
│                              │                                       │
└──────────────────────────────┼───────────────────────────────────────┘
                               │
                               ▼
                        To OBC & Power Board
```

## Detailed Circuit Design

### 1. Dual-Band Communication System

#### 1.1 UHF/VHF Transceiver Section

The UHF/VHF communication system uses the AX5043 integrated transceiver with the following design elements:

- **RF Frontend**:
  - Low-Noise Amplifier: SKY67151-396LF 
    - Noise Figure: 0.7 dB
    - Gain: 19 dB
    - P1dB: +22 dBm
  - Bandpass Filter: BFCN-142+ (VHF) and BFCN-435+ (UHF)
    - Insertion Loss: < 2 dB
    - Out-of-band Rejection: > 40 dB
  - Transmit/Receive Switch: SKY13350-385LF
    - Isolation: > 25 dB
    - Insertion Loss: < 0.45 dB

- **Transceiver**:
  - AX5043 Ultra-Low Power RF Transceiver
    - Frequency Range: 70-1050 MHz
    - Modulation: (G)FSK, (G)MSK, GMSK, 4-FSK
    - Sensitivity: -137 dBm (FSK, 1 kbps)
    - Power Consumption: 7.5 mA RX, 16 mA TX at 0 dBm
    - Interface: SPI to FPGA

- **Power Amplifier**:
  - RFPA1011 UHF Power Amplifier
    - Output Power: Up to 37 dBm (5W)
    - Efficiency: 55%
    - Gain: 27 dB
    - Power Control: 20 dB range via DAC
    - Thermal Management: Copper pour with thermal vias

#### 1.2 S-band System

The S-band system is built around the AD9361 integrated RF agile transceiver:

- **RF Frontend**:
  - Low-Noise Amplifier: PMA2-83LN+
    - Frequency Range: 2.3-8.0 GHz
    - Noise Figure: 1.3 dB
    - Gain: 21 dB
  - Bandpass Filter: BFCN-2435+
    - Center Frequency: 2.45 GHz
    - Bandwidth: 100 MHz
    - Insertion Loss: 1.5 dB
  - TR Switch: SKY13330-374LF
    - Isolation: > 30 dB
    - Insertion Loss: < 0.4 dB

- **Transceiver**:
  - AD9361 Integrated RF Agile Transceiver
    - Frequency Range: 70 MHz to 6 GHz
    - Channel Bandwidth: < 200 kHz to 56 MHz
    - RX Noise Figure: 2 dB
    - TX EVM: -40 dB
    - Integrated 12-bit ADCs and DACs

- **LO Generator**:
  - MAX2871 Synthesizer/VCO
    - Frequency Range: 23.5 to 6000 MHz
    - Phase Noise: -137 dBc/Hz
    - Integrated VCO and Dividers
    - SPI Control Interface

- **Power Amplifier**:
  - RFPA3800 S-band Power Amplifier
    - Output Power: +33 dBm (2W)
    - Power-Added Efficiency: 45%
    - Gain: 30 dB
    - MMIC Technology for Size Reduction

#### 1.3 RF Switching & Routing

- **Diplexer**:
  - MACOM MAPD-011027 
    - Passbands: 135-155 MHz and 400-450 MHz
    - Isolation: > 50 dB
    - Insertion Loss: < 0.5 dB

- **Main RF Switch**:
  - SKY13330-374LF SPDT Switch
    - Frequency Range: DC-6 GHz
    - Isolation: > 30 dB
    - Insertion Loss: < 0.45 dB
    - Control: GPIO lines from FPGA

- **Antenna Interfaces**:
  - UHF/VHF: U.FL connector to deployable dipole antenna
  - S-band: 4x U.FL connectors to patch antenna array
  - Impedance: 50Ω
  - VSWR Protection: Directional coupler with power monitor

### 2. Signal Processing

#### 2.1 Analog-to-Digital & Digital-to-Analog Conversion

For additional baseband processing outside the integrated transceivers:

- **ADC System**:
  - AD9629 ADC
    - Resolution: 12-bit
    - Sampling Rate: 80 MSPS
    - SNR: 70.6 dBFS
    - Power: 211 mW
  - Anti-aliasing Filter: 5th order Butterworth
  - Clock: 40 MHz reference with jitter cleaner

- **DAC System**:
  - AD9744 DAC
    - Resolution: 14-bit
    - Update Rate: 210 MSPS
    - SFDR: 84 dB
    - Power: 380 mW
  - Reconstruction Filter: 4th order elliptic
  - Clock: Synchronized with ADC

#### 2.2 Digital Signal Processing

- **FPGA Processing System**:
  - Microsemi SmartFusion2 M2S025
    - 25K Logic Elements
    - 64 Math

