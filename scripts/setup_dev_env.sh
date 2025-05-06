#!/bin/bash
# Development environment setup script for SkyMesh

set -e  # Exit immediately if a command exits with a non-zero status

# Color codes for output formatting
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Log utilities
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Detect operating system
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if grep -q Microsoft /proc/version 2>/dev/null; then
            echo "wsl"
        else
            echo "linux"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    else
        log_error "Unsupported operating system: $OSTYPE"
        exit 1
    fi
}

# Create directory structure if it doesn't exist
setup_directory_structure() {
    log_info "Setting up directory structure..."
    
    mkdir -p src/{orbital,protocol,ground}
    mkdir -p tests
    mkdir -p docs
    mkdir -p build
    mkdir -p scripts/hooks
    mkdir -p tools/simulators
    
    log_success "Directory structure created"
}

# Check and install common dependencies
install_common_dependencies() {
    log_info "Checking common dependencies..."
    
    # Check for Git
    if ! command -v git &> /dev/null; then
        log_error "Git is not installed. Please install Git and try again."
        exit 1
    fi
    
    # Check for Python
    if ! command -v python3 &> /dev/null; then
        log_error "Python 3 is not installed. Please install Python 3 and try again."
        exit 1
    fi
    
    # Install basic Python packages
    log_info "Installing required Python packages..."
    python3 -m pip install --upgrade pip
    python3 -m pip install numpy matplotlib pytest pylint black
    
    # Check for CMake
    if ! command -v cmake &> /dev/null; then
        log_warning "CMake is not installed. It's recommended for building C/C++ components."
        read -p "Install CMake? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            install_cmake
        fi
    fi
    
    log_success "Common dependencies checked"
}

# Platform-specific dependency installation
install_platform_dependencies() {
    OS=$(detect_os)
    log_info "Detected OS: $OS"
    
    case $OS in
        linux)
            log_info "Installing Linux dependencies..."
            if command -v apt-get &> /dev/null; then
                sudo apt-get update
                sudo apt-get install -y build-essential libssl-dev libffi-dev python3-dev
            elif command -v dnf &> /dev/null; then
                sudo dnf install -y gcc gcc-c++ openssl-devel libffi-devel python3-devel
            else
                log_warning "Unsupported Linux distribution. Please install the equivalent of build-essential, libssl-dev, libffi-dev, python3-dev manually."
            fi
            ;;
        
        macos)
            log_info "Installing macOS dependencies..."
            if ! command -v brew &> /dev/null; then
                log_info "Installing Homebrew..."
                /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
            fi
            brew update
            brew install openssl readline sqlite3 xz zlib
            ;;
        
        wsl)
            log_info "Installing WSL dependencies..."
            sudo apt-get update
            sudo apt-get install -y build-essential libssl-dev libffi-dev python3-dev dos2unix
            ;;
    esac
    
    log_success "Platform-specific dependencies installed"
}

# Install CMake based on platform
install_cmake() {
    OS=$(detect_os)
    
    case $OS in
        linux|wsl)
            if command -v apt-get &> /dev/null; then
                sudo apt-get install -y cmake
            elif command -v dnf &> /dev/null; then
                sudo dnf install -y cmake
            else
                log_warning "Please install CMake manually for your distribution."
            fi
            ;;
        
        macos)
            brew install cmake
            ;;
    esac
}

# Set up build environment
setup_build_environment() {
    log_info "Setting up build environment..."
    
    # Create basic build scripts
    cat > build/build.sh << 'EOF'
#!/bin/bash
set -e

# Simple build script for SkyMesh
mkdir -p build/output
cd build/output
cmake ../..
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
EOF
    chmod +x build/build.sh
    
    # Create CMakeLists.txt template
    cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.10)
project(SkyMesh VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Add subdirectories
add_subdirectory(src)

# Testing
enable_testing()
add_subdirectory(tests)

# Output settings
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
EOF
    
    # Create src/CMakeLists.txt
    cat > src/CMakeLists.txt << 'EOF'
# Add subdirectories
add_subdirectory(orbital)
add_subdirectory(protocol)
add_subdirectory(ground)
EOF
    
    # Create placeholder CMakeLists.txt files in subdirectories
    for dir in src/orbital src/protocol src/ground; do
        cat > $dir/CMakeLists.txt << EOF
# ${dir} components
# TODO: Add actual targets
EOF
    done
    
    # Create tests/CMakeLists.txt
    cat > tests/CMakeLists.txt << 'EOF'
# Test configuration
# TODO: Add actual tests
EOF
    
    log_success "Build environment set up"
}

# Configure development tools
configure_dev_tools() {
    log_info "Configuring development tools..."
    
    # Set up .vscode directory with basic configuration
    mkdir -p .vscode
    
    # settings.json for VSCode
    cat > .vscode/settings.json << 'EOF'
{
    "editor.formatOnSave": true,
    "python.linting.enabled": true,
    "python.linting.pylintEnabled": true,
    "python.formatting.provider": "black",
    "C_Cpp.clang_format_style": "{ BasedOnStyle: Google, IndentWidth: 4, ColumnLimit: 100 }"
}
EOF
    
    # tasks.json for VSCode
    cat > .vscode/tasks.json << 'EOF'
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build SkyMesh",
            "type": "shell",
            "command": "./build/build.sh",
            "group": {
                "kind": "build",
                "isDefault": true
            }
        },
        {
            "label": "Run Tests",
            "type": "shell",
            "command": "cd build/output && ctest",
            "group": {
                "kind": "test",
                "isDefault": true
            }
        }
    ]
}
EOF
    
    # Create a basic .editorconfig file
    cat > .editorconfig << 'EOF'
# EditorConfig is awesome: https://EditorConfig.org

root = true

[*]
end_of_line = lf
insert_final_newline = true
charset = utf-8
trim_trailing_whitespace = true
indent_style = space
indent_size = 4

[*.{json,yml,yaml}]
indent_size = 2

[Makefile]
indent_style = tab
EOF
    
    log_success "Development tools configured"
}

# Initialize test environment
init_test_environment() {
    log_info "Initializing test environment..."
    
    # Create basic test framework
    cat > tests/README.md << 'EOF'
# SkyMesh Tests

This directory contains tests for the SkyMesh project.

## Structure

- `unit/`: Unit tests for individual components
- `integration/`: Integration tests for subsystems
- `e2e/`: End-to-end tests for the complete system
- `performance/`: Performance benchmarks

## Running Tests

Run all tests:
```
cd build/output && ctest
```

Run specific test category:
```
cd build/output && ctest -R unit
```
EOF
    
    # Create test directory structure
    mkdir -p tests/{unit,integration,e2e,performance}
    
    # Create sample test file
    cat > tests/unit/test_sample.py << 'EOF'
#!/usr/bin/env python3

def test_sample():
    """Sample test function."""
    assert 1 + 1 == 2, "Basic addition should work"

if __name__ == "__main__":
    test_sample()
    print("All tests passed!")
EOF
    chmod +x tests/unit/test_sample.py
    
    log_success "Test environment initialized"
}

# Set up pre-commit hooks
setup_precommit_hooks() {
    log_info "Setting up pre-commit hooks..."
    
    # Create pre-commit hook directory if it doesn't exist
    mkdir -p .git/hooks
    
    # Create pre-commit hook
    cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash

# Pre-commit hook for SkyMesh project

# Exit on error
set -e

# Store the files that are about to be committed
files=$(git diff --cached --name-only --diff-filter=ACM)

# Skip if no files are changed
if [ -z "$files" ]; then
    exit 0
fi

# Check Python files with pylint
python_files=$(echo "$files" | grep -E '\.py$' || true)
if [ -n "$python_files" ]; then
    echo "Running pylint on Python files..."
    echo "$python_files" | xargs pylint --disable=C0111,C0103,C0303 || {
        echo "Python linting failed. Please fix the issues before committing."
        exit 1
    }
    
    echo "Running black on Python files..."
    echo "$python_files" | xargs black || {
        echo "Python formatting failed. Please run 'black' on your Python files."
        exit 1
    }
fi

# Check C/C++ files with clang-format
cpp_files=$(echo "$files" | grep -E '\.(c|cpp|h|hpp)$' || true)
if [ -n "$cpp_files" ] && command -v clang-format >/dev/null; then
    echo "Running clang-format on C/C++ files..."
    for file in $cpp_files; do
        clang-format -style=file -i "$file"
        git add "$file"
    done
fi

exit 0
EOF
    chmod +x .git/hooks/pre-commit
    
    # Copy the hook to the scripts directory for version control
    mkdir -p scripts/hooks
    cp .git/hooks/pre-commit scripts/hooks/
    
    log_success "Pre-commit hooks set up"
}

# Main function to run the setup process
main() {
    log_info "Starting SkyMesh development environment setup..."
    
    # Create scripts directory if it doesn't exist
    mkdir -p scripts
    
    # Make the script executable
    chmod +x "$0"
    
    # Run setup steps
    setup_directory_structure
    install_common_dependencies
    install_platform_dependencies
    setup_build_environment
    configure_dev_tools
    init_test_environment
    setup_precommit_hooks
    
    log_success "SkyMesh development environment setup complete!"
    log_info "To build the project, run: ./build/build.sh"
    log_info "To run tests, execute: cd build/output && ctest"
}

# Run the main function
main

