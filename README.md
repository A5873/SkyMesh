<p align="center">
  <img src="docs/assets/logos/skymesh-logo-full-color.svg" width="600" alt="SkyMesh Logo">
</p>

# SkyMesh: Decentralized Orbital Mesh Network

SkyMesh is a decentralized orbital mesh network composed of low-cost nano-satellites, each powered by radiation-tolerant single-board computers running a custom lightweight satellite OS. These CubeSat-class devices operate in low Earth orbit and act as dynamic uplink nodes, relays, and observers, forming an autonomous, reconfigurable communication layer in space.

Paired with open-source ground gateways and uplink stations, SkyMesh enables:
- Censorship-resistant data exchange
- Emergency communication networks
- Remote IoT relay capabilities
- Decentralized internet fallback infrastructure

Designed with modular hardware and a real-time-capable OS optimized for mesh networking and power efficiency, SkyMesh bridges the Earth and orbit with a peer-to-peer ethos—turning the sky into a sovereign digital space.

## Core Features

- 🛰️ **Orbital Mesh Network**: Dynamic satellite-to-satellite communication
- 🔒 **Radiation-Hardened Design**: Triple Modular Redundancy and error correction
- 📡 **Multi-Band Communication**: UHF/VHF for control, S-band for high-speed data
- 🌐 **Open Protocol**: Community-driven mesh networking standards
- 🔋 **Efficient Power Management**: Adaptive power control and optimization
- 🛡️ **Autonomous Operation**: Self-healing network with minimal ground control

## Development Status

**Current Status**: Early Development / Prototype Phase

SkyMesh is currently in the early development and prototype design phase. I am working on system architecture, simulation tools, and proof-of-concept implementations for the core components.

## Quick Start Guide

### Prerequisites

- Linux, macOS, or Windows with WSL
- CMake 3.15+
- GCC/Clang 10+ or MSVC 19+
- Python 3.8+ (for simulation and tools)
- GNU Radio 3.9+ (for RF simulation)

### Clone and Build

```bash
# Clone the repository
git clone https://github.com/A5873/SkyMesh.git
cd SkyMesh

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j$(nproc)

# Run tests
make test
```

### Running Simulations

```bash
# Start the mesh network simulation
./bin/sim/mesh_sim --nodes=12 --orbit=leo --duration=24h

# Visualize the results
python3 tools/visualize.py --data=sim_results.json
```

## Detailed Build Instructions

### Standard Build

The build process uses CMake to configure the project:

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

### Build Options

- `-DENABLE_TESTS=ON`: Build test suite (default: ON)
- `-DENABLE_SIMULATION=ON`: Build simulation tools (default: ON)
- `-DENABLE_RF_MODULES=ON`: Build RF communication modules (default: OFF, requires GNU Radio)
- `-DENABLE_DOCS=ON`: Build documentation (default: OFF, requires Doxygen)

### Cross-compilation for Target Hardware

For RISC-V hardware targets:

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/riscv.cmake ..
```

## Development Environment Setup

### Required Dependencies

```bash
# Ubuntu/Debian
sudo apt install build-essential cmake libboost-all-dev python3-dev libgtest-dev

# macOS
brew install cmake boost python gtest

# Python dependencies
pip install numpy matplotlib pycubesat orbit-predictor
```

### IDE Integration

- **VSCode**: Install C/C++ and CMake extensions
- **CLion**: Native CMake support
- **Eclipse**: Import as CMake project

## Architecture Overview

The SkyMesh architecture consists of three main components:

1. **Orbital Layer**: A constellation of CubeSat-class nanosatellites in low Earth orbit (LEO), forming a dynamic mesh network.
2. **Ground Layer**: A network of ground stations and gateways that connect to the orbital layer.
3. **Protocol Layer**: The software stSee the [full architecture documentation](docs/PROJECT.md) for detailed information.

## Key Components

### Satellite OS

The custom lightweight operating system running on each satellite node provides:
- Real-time capabilities for critical operations
- Power-efficient operation modes
- Radiation-tolerant software design with error detection and correction
- Secure communications with end-to-end encryption
- Over-the-air update mechanisms
- Autonomous constellation management

### Mesh Network

The decentralized networking layer enables:
- Dynamic routing protocols optimized for orbital mechanics
- Self-healing network topology
- Quality of service management
- Bandwidth optimization
- Secure message passing
- Store-and-forward capability for delayed delivery

### Ground Station

The ground segment consists of:
- Open-source gateway software for uplink/downlink operations
- User-deployable antenna designs
- Authentication and access control systems
- Local caching and message prioritization
- Emergency broadcast capabilities
- API interfaces for application development

### Hardware

The hardware designs focus on:
- Radiation-tolerant single-board computers
- Low-cost, CubeSat-compatible form factors
- Modular antenna systems
- Power-efficient components
- Solar panel integration
- Open standardized interfaces

## Contribution Guidelines

### Code Contributions

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/your-feature-name`
3. Commit your changes: `git commit -am 'Add some feature'`
4. Push to the branch: `git push origin feature/your-feature-name`
5. Submit a pull request

### Coding Standards

- Follow the [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- Use clang-format with the provided configuration
- Ensure all tests pass before submitting PRs
- Include documentation for new features

### Issue Reporting

Report issues using the GitHub issue tracker. Please include:

- Detailed description of the issue
- Steps to reproduce
- Expected vs. actual behavior
- Environment information


## Project Roadmap

### Phase 1: Foundation (Current)
- System architecture design
- Simulation framework
- Core OS prototype

### Phase 2: Ground Testing
- Hardware communication modules
- Radiation tolerance testing
- Full mesh protocol implementation

### Phase 3: Deployment
- Prototype satellite construction
- Launch of initial test constellation
- Ground station network setup

See our [detailed roadmap](docs/business/roadmap.md) for more information.

## Development Workflow

The project uses a branching model:
- `main` branch for stable releases
- `develop` branch for integration
- Feature branches for new development

Each component can be developed and tested independently:
- Satellite OS can be tested in simulation
- Mesh protocols can be verified in virtual networks
- Ground station software can interface with test devices
## License


