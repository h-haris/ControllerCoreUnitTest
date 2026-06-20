#!/bin/bash
#
# Quick Build & Test Script for ControllerCoreUnitTest
# 
# This script automates the build and test process for the Google Test migration.
#

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print colored message
print_info() {
    echo -e "${BLUE}ℹ${NC} $1"
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_header() {
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}\n"
}

# Parse command line arguments
BUILD_TYPE="Release"
RUN_TESTS=true
RUN_ORACLE=false
VERBOSE=false
CLEAN=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --no-test)
            RUN_TESTS=false
            shift
            ;;
        --oracle)
            RUN_ORACLE=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug       Build in Debug mode (default: Release)"
            echo "  --no-test     Skip running tests after build"
            echo "  --oracle      Also build and run oracle tests"
            echo "  --verbose     Enable verbose test output"
            echo "  --clean       Clean build directory before building"
            echo "  --help        Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0                    # Build and run Google Tests (Release)"
            echo "  $0 --debug            # Build and run in Debug mode"
            echo "  $0 --oracle           # Build both Google and Oracle tests"
            echo "  $0 --clean --verbose  # Clean build with verbose test output"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Print configuration
print_header "ControllerCoreUnitTest - Quick Build"
print_info "Build type: $BUILD_TYPE"
print_info "Run tests: $RUN_TESTS"
print_info "Include oracle: $RUN_ORACLE"
print_info "Verbose: $VERBOSE"
print_info "Clean build: $CLEAN"
echo ""

# Check for CMake
if ! command -v cmake &> /dev/null; then
    print_error "CMake not found. Please install CMake 3.14 or later."
    exit 1
fi

print_success "CMake found: $(cmake --version | head -n 1)"

# Clean if requested
if [ "$CLEAN" = true ]; then
    print_header "Cleaning Build Directory"
    if [ -d "build" ]; then
        rm -rf build
        print_success "Build directory removed"
    else
        print_info "Build directory doesn't exist, nothing to clean"
    fi
fi

# Create build directory
print_header "Configuring Build"
mkdir -p build
cd build

# Configure CMake
CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"

if [ "$RUN_ORACLE" = true ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DBUILD_ORACLE_TESTS=ON"
else
    CMAKE_ARGS="$CMAKE_ARGS -DBUILD_ORACLE_TESTS=OFF"
fi

print_info "Running: cmake $CMAKE_ARGS .."
if cmake $CMAKE_ARGS ..; then
    print_success "Configuration complete"
else
    print_error "Configuration failed"
    exit 1
fi

# Build
print_header "Building Tests"
if cmake --build . --config $BUILD_TYPE; then
    print_success "Build complete"
else
    print_error "Build failed"
    exit 1
fi

# Run tests if requested
if [ "$RUN_TESTS" = true ]; then
    print_header "Running Tests"
    
    # Run Google Tests
    print_info "Running Google Test suite..."
    if [ "$VERBOSE" = true ]; then
        if ctest -C $BUILD_TYPE --verbose -L gtest; then
            print_success "Google Tests PASSED"
        else
            print_error "Google Tests FAILED"
            exit 1
        fi
    else
        if ctest -C $BUILD_TYPE --output-on-failure -L gtest; then
            print_success "Google Tests PASSED"
        else
            print_error "Google Tests FAILED"
            exit 1
        fi
    fi
    
    # Run Oracle tests if requested
    if [ "$RUN_ORACLE" = true ]; then
        print_info "Running Oracle Test suite..."
        if [ "$VERBOSE" = true ]; then
            if ctest -C $BUILD_TYPE --verbose -L oracle; then
                print_success "Oracle Tests PASSED"
            else
                print_error "Oracle Tests FAILED"
                exit 1
            fi
        else
            if ctest -C $BUILD_TYPE --output-on-failure -L oracle; then
                print_success "Oracle Tests PASSED"
            else
                print_error "Oracle Tests FAILED"
                exit 1
            fi
        fi
    fi
fi

# Summary
print_header "Build Summary"
echo "Build directory: $(pwd)"
echo ""
echo "Executables:"
if [ -f "controller_gtest" ] || [ -f "controller_gtest.exe" ]; then
    print_success "controller_gtest (Google Test suite)"
fi
if [ -f "controller_oracle" ] || [ -f "controller_oracle.exe" ]; then
    print_success "controller_oracle (MinUnit oracle tests)"
fi
echo ""

if [ "$RUN_TESTS" = true ]; then
    echo "Test Results:"
    print_success "All tests completed successfully!"
    echo ""
fi

print_info "To run tests again:"
echo "  cd build"
echo "  ctest --output-on-failure"
echo ""
print_info "Or run executable directly:"
echo "  ./controller_gtest"
echo ""
print_info "For more options, see BUILD_GUIDE.md"
echo ""

print_success "Done!"
