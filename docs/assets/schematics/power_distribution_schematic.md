# SkyMesh Power Distribution Board - Technical Schematic

## Overview

This document describes the power distribution board schematic for the SkyMesh satellite. The power subsystem manages energy collection from solar panels, battery charging and protection, power regulation for various subsystems, and load switching with fault protection.

## Key Specifications

- Input: 4x Solar panels (2-5V per panel)
- Battery: 4S2P Li-ion configuration (14.8V nominal, 60Wh capacity)
- Output rails: 3.3V@3A, 5V@2A, 12V@1A
- Subsystem isolation with independent current monitoring
- Radiation-tolerant design with redundant critical circuits
- I2C telemetry and control interface
- Microcontroller: STM32L4 series (low power)
- PCB: 6-layer with radiation shielding

## Block Diagram

```
                 ┌───────────────────────────────────────────┐
                 │              SOLAR PANEL INPUTS           │
                 │    ┌─────────┐    ┌─────────┐            │
                 │    │         │    │         │            │
Solar Panel 1 ───┼───►│ MPPT #1 │    │ MPPT #2 │◄───────────┼─── Solar Panel 2
                 │    │ SPV1040 │    │ SPV1040 │            │
                 │    │         │    │         │            │
                 │    └────┬────┘    └────┬────┘            │
                 │         │              │                 │
                 │    ┌────▼────┐    ┌────▼────┐            │
                 │    │         │    │         │            │
Solar Panel 3 ───┼───►│ MPPT #3 │    │ MPPT #4 │◄───────────┼─── Solar Panel 4
                 │    │ SPV1040 │    │ SPV1040 │            │
                 │    │         │    │         │            │
                 │    └────┬────┘    └────┬────┘            │
                 └─────────┼──────────────┼─────────────────┘
                           │              │
                           ▼              ▼
                 ┌───────────────────────────────────────────┐
                 │        BATTERY MANAGEMENT SYSTEM          │
                 │                                           │
                 │    ┌─────────────────────────┐            │
                 │    │    BQ76940 Battery      │            │
                 │    │    Management IC        │            │
                 │    │                         │            │
          ┌──────┼───►│                         │            │
          │      │    └─────────────────────────┘            │
┌─────────▼─────┐│                                           │
│  4S2P Li-ion  ││    ┌─────────────────────────┐            │
│  Battery Pack ││    │  Temperature Sensors &  │◄─┐         │
│  (60Wh)       ││    │  Protection Circuits    │  │         │
└─────────┬─────┘│    └─────────────────────────┘  │         │
          │      │                                  │         │
          └──────┼──────────────────────────────────┘         │
                 │                │                           │
                 └────────────────┼───────────────────────────┘
                                  │
                                  ▼
                 ┌───────────────────────────────────────────┐
                 │          POWER REGULATION                 │
                 │                                           │
                 │    ┌─────────────┐   ┌─────────────┐      │
                 │    │ TPS62142    │   │ TPS62110    │      │
                 │    │ 3.3V/3A     │   │ 5V/2A       │      │
                 │    │ Buck        │   │ Buck        │      │
                 │    └──────┬──────┘   └──────┬──────┘      │
                 │           │                 │             │
                 │    ┌──────▼──────┐                        │
                 │    │ TPS61088    │                        │
                 │    │ 12V/1A      │                        │
                 │    │ Boost       │                        │
                 │    └──────┬──────┘                        │
                 │           │                               │
                 │    ┌──────▼──────┐                        │
                 │    │ Fault       │                        │
                 │    │ Detection   │                        │
                 │    └──────┬──────┘                        │
                 └───────────┼────────────────────────────────┘
                             │
                             ▼
                 ┌───────────────────────────────────────────┐
                 │          DISTRIBUTION CONTROL             │
                 │                                           │
                 │    ┌─────────────────────────────┐        │
                 │    │   STM32L4 Microcontroller   │        │
                 │    └───────────┬─────────────────┘        │
                 │                │                          │
                 │    ┌───────────▼─────────────────┐        │
                 │    │  I2C Control Interface      │        │
                 │    └───────────┬─────────────────┘        │
                 │                │                          │
                 │                ├─────────┬─────────┐      │
                 │                │         │         │      │
                 │    ┌───────────▼─┐ ┌─────▼───────┐ ┌─────▼───────┐  │
                 │    │Load Switch #1│ │Load Switch #2│ │Load Switch #n│  │
                 │    │INA226 Monitor│ │INA226 Monitor│ │INA226 Monitor│  │
                 │    └───────┬─────┘ └─────┬───────┘ └─────┬───────┘  │
                 └────────────┼───────────────────────────────────────┘
                              │           │           │
                              ▼           ▼           ▼
                           To OBC      To COMMS     To other
                                                   subsystems
```

## Detailed Circuit Design

### 1. Solar Panel Input Section

Each solar panel input circuit consists of:

- SPV1040 MPPT controller for maximum power extraction
- Schottky diode (MBRS360T3G) for reverse polarity protection
- 10μF input capacitor for filtering
- 22μH inductor for boost operation
- 100μF output capacitor for smoothing
- INA219 current/voltage monitor IC for telemetry

The solar panel section outputs are connected in parallel to a common power bus that feeds the battery management system.

**Key component: SPV1040**
- Input voltage range: 0.3V to 5.5V
- Maximum input current: 1.8A
- Adjustable output voltage
- Efficiency up to 95%
- MPPT algorithm for optimal power extraction

### 2. Battery Management Section

The battery management section uses a BQ76940 battery monitoring IC with the following features:

- 4S (four cell series) battery monitoring
- Cell balancing circuits with 100Ω resistors and 2N7002 MOSFETs
- Temperature monitoring using 10K NTC thermistors at three points
- Overcurrent protection using ACS712 current sensor
- Overvoltage/undervoltage protection with automatic disconnection
- Secondary protection using fuses and MOSFETs

**Key component: BQ76940**
- Monitors up to 10 series cells
- Integrated 14-bit ADC
- Cell balancing control
- Temperature monitoring
- Overcurrent/short circuit protection
- Communication via I2C

### 3. Power Regulation Section

The power regulation section converts the battery voltage to the required levels for different subsystems:

#### 3.3V Rail (Digital Logic)
- TPS62142 synchronous buck converter
- Input: 14.8V nominal
- Output: 3.3V at up to 3A
- Efficiency > 90%
- Components:
  - 10μH inductor
  - 22μF input capacitor
  - 2x 22μF output capacitors
  - Feedback resistors: 30.1K and 10K

#### 5V Rail (Peripherals)
- TPS62110 synchronous buck converter
- Input: 14.8V nominal
- Output: 5V at up to 2A
- Efficiency > 90%
- Components:
  - 10μH inductor
  - 22μF input capacitor
  - 2x 22μF output capacitors
  - Feedback resistors: 52.3K and 10K

#### 12V Rail (RF Amplifiers)
- TPS61088 boost converter
- Input: 14.8V nominal (can operate from 3V to 12V)
- Output: 12V at up to 1A
- Efficiency > 92%
- Components:
  - 4.7μH inductor
  - 22μF input capacitor
  - 2x 22μF output capacitors
  - Feedback resistors: 110K and 10K

### 4. Distribution Control Section

The distribution control section manages power distribution to each subsystem with:

- STM32L432KC microcontroller for control logic
- TPS22918 load switches for each subsystem
- INA226 current/voltage monitors for each power rail
- I2C communication for telemetry and control
- Radiation-tolerant design with watchdog timer
- Redundant power path for critical systems
- LED indicators for visual status

**Key Components:**
- STM32L432KC: Low-power ARM Cortex-M4 microcontroller
- TPS22918: 5A load switch with controlled slew rate
- INA226: High-side current and voltage monitor with 0.1% accuracy
- ISO1540: I2C isolator for protection

## PCB Layout Considerations

1. 6-layer PCB with dedicated power and ground planes
2. Separate analog and digital grounds, connecting at a single point
3. Thermal considerations for power components (copper pours, thermal vias)
4. EMI mitigation with proper component placement and routing
5. Radiation shielding for sensitive components
6. Conformal coating for vacuum operation

## Testing Procedures

1. Solar panel input testing with solar simulator
2. Battery charging/discharging cycle tests
3. Load regulation testing at minimum and maximum loads
4. Thermal vacuum testing for space qualification
5. Radiation testing for single event effects
6. Full system integration testing with actual subsystems

## Bill of Materials (Key Components)

| Component | Description | Quantity | Package |
|-----------|-------------|----------|---------|
| SPV1040 | MPPT Controller | 4 | 8-SOIC |
| BQ76940 | Battery Monitoring IC | 1 | 30-TSSOP |
| TPS62142 | 3.3V Buck Converter | 1 | 10-QFN |
| TPS62110 | 5V Buck Converter | 1 | 16-VQFN |
| TPS61088 | 12V Boost Converter | 1 | 16-VQFN |
| STM32L432KC | Microcontroller | 1 | 32-QFN |
| INA226 | Current/Voltage Monitor | 6 | 10-MSOP |
| TPS22918 | Load Switch | 6 | 6-SON |
| ISO1540 | I2C Isolator | 2 | 8-SOIC |
| ACS712 | Current Sensor | 2 | 8-SOIC |

## Appendix: Pin Connections

### STM32L432KC Microcontroller Pin Assignments

| Pin | Function | Connection |
|-----|----------|------------|
| PA0 | ADC_IN0 | Battery Voltage Monitor |
| PA1 | ADC_IN1 | Temperature Sensor 1 |
| PA2 | ADC_IN2 | Temperature Sensor 2 |
| PA3 | ADC_IN3 | Temperature Sensor 3 |
| PA5 | FAULT_DET | Fault Detection Input |
| PA7 | LED1 | Status LED 1 |
| PB0 | LED2 | Status LED 2 |
| PB1 | EN_3V3 | 3.3V Rail Enable |
| PB3 | EN_5V | 5V Rail Enable |
| PB4 | EN_12V | 12V Rail Enable |
| PB6 | I2C1_SCL | I2C Control Bus Clock |
| PB7 | I2C1_SDA | I2C Control Bus Data |
| PA9 | UART1_TX | Debug UART TX |
| PA10 | UART1_RX | Debug UART RX |
| PA11 | EN_SW1 | Load Switch 1 Enable |
| PA12 | EN_SW2 | Load Switch 2 Enable |
| PB5 | EN_SW3 | Load Switch 3 Enable |
| PA8 | EN_SW4 | Load Switch 4 Enable |
| PA4 | EN_SW5 | Load Switch 5 Enable |
| PA6 | EN_SW6 | Load Switch 6 Enable |

## Safety Features

1. Overcurrent protection on all power rails
2. Thermal shutdown protection in regulators
3. Undervoltage lockout to prevent battery damage
4. Watchdog timer for microcontroller monitoring
5. Secondary protection with fuses
6. EMI filtering on all inputs and outputs
7. Isolation between control and power circuitry

---

*Note: This schematic represents a design at the block diagram and component selection level. Final implementation would require detailed schematic capture in KiCad or similar EDA software.*

