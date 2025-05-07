# SkyMesh ADCS - Technical Schematic

## Overview

This document describes the Attitude Determination and Control System (ADCS) schematic for the SkyMesh satellite. The ADCS provides orientation control in orbit, enabling proper solar panel pointing, antenna directionality, and thermal management. The design uses a combination of sensors for attitude determination and electromagnetic actuators for control, with redundancy for critical functions.

## Key Specifications

- **Pointing Accuracy**: Better than 2° in all axes
- **Stabilization Time**: < 30 minutes from tumbling state
- **Angular Rate Limit**: 5°/sec maximum during normal operation
- **Power Consumption**:
  - 120mW average (determination only)
  - 1.2W peak during actuation
  - 3.3V and 5V power rails
- **Operational Modes**:
  - Detumbling
  - Sun pointing
  - Nadir pointing
  - Target tracking (for communication)
- **Size**: 90mm x 95mm PCB
- **Mass**: < 150g (electronics only, excluding reaction wheels)
- **MTBF**: > 20,000 hours

## Block Diagram

```
                                  ┌─────────────────┐
                                  │   Power Input   │
                                  │   3.3V & 5V     │
                                  └────────┬────────┘
                                           │
                                           ▼
┌──────────────────────────────────────────────────────────────────────┐
│                           POWER MANAGEMENT                           │
│                                                                      │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐              │
│  │ 3.3V LDO    │    │ 5V LDO      │    │ Power Mon   │              │
│  │ TPS7A2633   │    │ TPS7A4701   │    │ INA226      │              │
│  └──────┬──────┘    └──────┬──────┘    └──────┬──────┘              │
│         │                  │                   │                     │
└─────────┼──────────────────┼───────────────────┼─────────────────────┘
          │                  │                   │
          ▼                  ▼                   ▼
┌─────────────────────────────────────────────────────────────────────┐
│                          SENSOR SYSTEMS                             │
│                                                                     │
│  ┌─────────────────────┐   ┌─────────────────────┐                  │
│  │ Sun Sensors (x6)    │   │ Magnetometers (x2)  │                  │
│  │ ▪ OSRAM SFH 2430    │   │ ▪ HMC1053           │                  │
│  │ ▪ Photodiode Array  │   │ ▪ 3-axis            │                  │
│  └──────────┬──────────┘   └──────────┬──────────┘                  │
│             │                         │                             │
│  ┌──────────▼──────────┐   ┌──────────▼──────────┐                  │
│  │ Gyroscopes (x2)     │   │ Temperature Sensors │                  │
│  │ ▪ BMG250            │   │ ▪ TMP112A           │                  │
│  │ ▪ 3-axis            │   │ ▪ 0.5°C Accuracy    │                  │
│  └──────────┬──────────┘   └──────────┬──────────┘                  │
│             │                         │                             │
│  ┌──────────▼─────────────────────────▼──────────┐                  │
│  │              Sensor Conditioning                │                  │
│  │ ▪ Op-amps: OPA2333     ▪ ADC: ADS131M04        │                  │
│  │ ▪ Mux: ADG708          ▪ Buffers: SN74AUP1G34  │                  │
│  └──────────────────────────┬───────────────────────┘                  │
└────────────────────────────┬┼────────────────────────────────────────┘
                             ││
                             ▼▼
┌────────────────────────────┬┬────────────────────────────────────────┐
│                          PROCESSING UNIT                             │
│                                                                      │
│  ┌─────────────────────────────────────────────────────────────┐     │
│  │                  STM32H743 Microcontroller                  │     │
│  │ ▪ ARM Cortex-M7   ▪ 480 MHz   ▪ FPU   ▪ DSP Instructions   │     │
│  └───────────────────────────┬─────────────────────────────────┘     │
│                              │                                       │
│  ┌──────────────────────┐    │    ┌────────────────────────────┐     │
│  │     Fault Detection  │◄───┼───►│ Watchdog: MAX6369          │     │
│  │  ▪ Current Monitors  │    │    │ ▪ Triple Redundant         │     │
│  │  ▪ Voltage Monitors  │    │    │ ▪ Independent Clock        │     │
│  └──────────┬───────────┘    │    └────────────┬───────────────┘     │
│             │                │                 │                      │
└─────────────┼────────────────┼─────────────────┼──────────────────────┘
              │                │                 │
              │                ▼                 │
┌─────────────┼────────────────────────────────────────────────────────┐
│             │           ACTUATION SYSTEMS                            │
│             │                                                        │
│  ┌──────────▼──────────┐   ┌─────────────────────┐                   │
│  │ Magnetorquer Control│   │ Reaction Wheel      │                   │
│  │ ▪ H-Bridge: TB6612  │   │ Driver              │                   │
│  │ ▪ 3-axis Control    │   │ ▪ DRV8313 3-phase   │                   │
│  └──────────┬──────────┘   └──────────┬──────────┘                   │
│             │                         │                              │
│  ┌──────────▼──────────┐   ┌──────────▼──────────┐                   │
│  │ Coil Drivers        │   │ PWM Control         │                   │
│  │ ▪ FETs: CSD18540    │   │ ▪ TIM1/TIM8         │                   │
│  │ ▪ Current Sense     │   │ ▪ 16-bit Resolution │                   │
│  └──────────┬──────────┘   └──────────┬──────────┘                   │
│             │                         │                              │
└─────────────┼─────────────────────────┼──────────────────────────────┘
              │                         │
              ▼                         ▼
     ┌─────────────────┐       ┌─────────────────┐
     │ Magnetorquers   │       │ Reaction Wheels │
     │ (X,Y,Z axes)    │       │ (X,Y,Z axes)    │
     └─────────────────┘       └─────────────────┘

                        ┌─────────────────┐
                        │   OBC Interface │
                        │   I2C & SPI     │
                        └─────────────────┘
```

## Detailed Circuit Design

### 1. Sensor Systems

#### 1.1 Sun Sensors

Six sun sensors are placed on each face of the CubeSat to provide full sky coverage:

- **Photodiode Array**:
  - OSRAM SFH 2430 PIN Photodiodes (4 per face)
  - Spectral Range: 750-1100 nm
  - Active Area: 7 mm²
  - Angle of Half Sensitivity: ±60°

- **Signal Conditioning**:
  - Transimpedance Amplifier: OPA2333 (low offset, rail-to-rail)
  - Gain: 1 MΩ feedback resistance
  - Bandwidth: 10 kHz
  - Dynamic Range: 100 dB
  - Compensation: Temperature correction based on TMP112A readings

- **Multiplexing**:
  - ADG708 8-channel analog multiplexer
  - Fast Settling: 150ns
  - Low On-Resistance: 2.5Ω
  - Power: 15 μW

#### 1.2 Magnetometers

Dual 3-axis magnetometers provide magnetic field measurements for attitude determination and magnetorquer control:

- **HMC1053 3-Axis Magnetometer**:
  - Range: ±6 gauss
  - Resolution: 120 μgauss
  - Bandwidth: 1 kHz
  - Power Consumption: 15 mW
  - Interface: SPI

- **Signal Conditioning**:
  - Set/Reset Circuit: Periodic flipping for offset cancellation
  - Anti-aliasing Filter: RC, 200 Hz cutoff
  - Protection: Series resistors and TVS diodes
  - Calibration: Runtime compensation for hard/soft iron effects

#### 1.3 Gyroscopes

Dual gyroscopes provide angular rate measurements:

- **BMG250 3-Axis MEMS Gyroscope**:
  - Range: ±2000°/s (configurable)
  - Resolution: 0.004°/s
  - Bandwidth: 1 kHz (configurable)
  - Noise Density: 0.007°/s/√Hz
  - Zero-Rate Offset: ±1°/s
  - Interface: SPI
  - Power: 5 mW (normal mode), 850 μW (low power)

- **Signal Processing**:
  - Digital Low-Pass Filter: Configurable 5-300 Hz
  - Self-Test: Built-in functionality
  - Auto-Zero: Regular recalibration during quiet periods
  - Redundancy: Cross-validation between two sensors

#### 1.4 Temperature Sensors

Multi-point temperature sensing for thermal management and sensor calibration:

- **TMP112A Digital Temperature Sensor**:
  - Range: -40°C to +125°C
  - Accuracy: ±0.5°C
  - Resolution: 0.0625°C
  - Power: 10 μW at 1 Hz sampling
  - Interface: I²C
  - Locations: 6 sensors distributed at critical points

- **ADC Integration**:
  - ADS131M04 Delta-Sigma ADC
  - 24-bit Resolution
  - 4 Differential Channels
  - Sample Rate: 128 kSPS
  - Programmable Digital Filter
  - Interface: SPI

### 2. Actuation Systems

#### 2.1 Magnetorquer Control

Magnetic torquers for coarse attitude control in Earth's magnetic field:

- **Control Circuitry**:
  - TB6612FNG Dual H-Bridge Driver (3 ICs for 3 axes)
  - Current Capability: 1.2A per coil
  - PWM Frequency: 20 kHz
  - Direction Control: 2-pin interface per coil
  - Voltage: 5V operation

- **Coil Drivers**:
  - CSD18540Q5B N-Channel FETs
    - Vds: 40V
    - Id: 100A
    - Rds(on): 5.9mΩ
  - INA169 Current Shunt Monitor
    - Range: 0-800 mA
    - Resolution: 0.5 mA
    - Bandwidth: 800 Hz

- **Magnetorquer Specifications**:
  - Air Core Design
  - Dipole Moment: 0.2 Am² per axis
  - Coil Resistance: 30Ω
  - Current: 167 mA at 5V

#### 2.2 Reaction Wheel Drivers

Three-axis reaction wheels for fine pointing control:

- **Motor Driver**:
  - DRV8313 Three-Phase Motor Driver
  - Current: Up to 2.5A per phase
  - Voltage: 8-60V operation (12V nominal)
  - Protection: Overcurrent, thermal shutdown
  - Control: PWM inputs up to 100 kHz

- **Sensing & Feedback**:
  - Hall Effect Sensors: DRV5053 (3 per wheel)
  - Speed Measurement: 12-bit resolution
  - Current Sensing: INA226 (0.1% accuracy)
  - Acceleration Control: PI algorithm with anti-windup
  - Limiting: Software and hardware torque/speed limits

- **Reaction Wheel Specifications**:
  - Brushless DC Motors
  - Moment of Inertia: 1.3×10⁻⁵ kg·m²
  - Maximum Speed: 6000 RPM
  - Maximum Torque: 2 mNm
  - Power: 500 mW at nominal speed

#### 2.3 PWM Generation & Control

Precision pulse-width modulation for motor and actuator control:

- **PWM Generation**:
  - STM32H7 Timer Modules (TIM1/TIM8)
  - Resolution: 16-bit (65536 steps)
  - Frequency: 20 kHz base
  - Features: Dead-time insertion, complementary outputs, emergency stop
  - Synchronization: Hardware triggers between timers

- **Control Features**:
  - Graceful Spin-Up: S-curve profile
  - Zero-Crossing Detection: For sensor-less control
  - Momentum Management: Software desaturation algorithm
  - Safe Mode: Automatic shutdown on anomalies

### 3. Processing Unit

#### 3.1 Microcontroller System

Main processing platform for attitude determination and control algorithms:

- **STM32H743ZI Microcontroller**:
  - Core: ARM Cortex-M7 @ 480 MHz
  - Memory: 2MB Flash, 1MB RAM with ECC
  - FPU: Single and double precision
  - DSP: Hardware acceleration for matrix operations
  - Peripherals:
    - 2× SPI (sensors & OBC)
    - 3× I²C (sensors & system bus)
    - 22× Timers 
    - 3× ADC (16-bit)

- **Software Architecture**:
  - RTOS: FreeRTOS with priority-based scheduling
  - Task Allocation:
    - Sensor Data Acquisition: 100 Hz

