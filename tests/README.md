# SkyMesh Tests

This directory contains test suites and testing infrastructure for the SkyMesh project.

## Overview

Testing in SkyMesh covers multiple levels, from unit tests for individual components to integration tests for subsystems and end-to-end tests that simulate the full orbital and ground network. The testing framework provides continuous integration capabilities and specialized tools for simulating the space environment.

## Test Categories

- Unit tests for individual components
- Integration tests for subsystems
- End-to-end tests for complete system operation
- Performance tests for benchmarking
- Radiation tolerance tests
- Power efficiency tests
- Networking resilience tests

## Directory Structure

- `/unit` - Unit tests for individual components
- `/integration` - Integration tests for subsystems
- `/e2e` - End-to-end system tests
- `/performance` - Performance benchmarks
- `/simulation` - Simulation tools and environments
- `/ci` - Continuous integration configuration

## Running Tests

```bash
# Run all tests
make test

# Run specific test suite
make test-satellite-os
make test-mesh-network
make test-ground-station

# Run performance benchmarks
make benchmark
```

## Development

See the main project README.md for development setup instructions.

