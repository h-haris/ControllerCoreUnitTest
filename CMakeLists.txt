cmake_minimum_required(VERSION 3.14)
project(ControllerCoreUnitTest VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configuration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_ORACLE_TESTS "Build original MinUnit oracle tests for comparison" ON)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_WARNINGS "Enable compiler warnings" ON)

# ============================================================================
# Find Dependencies
# ============================================================================

# Google Test - prefer system-installed, fall back to FetchContent
if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH /opt/homebrew)
endif()
find_package(GTest QUIET)
if(GTest_FOUND)
    add_library(gtest ALIAS GTest::gtest)
    add_library(gtest_main ALIAS GTest::gtest_main)
else()
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# Enable testing
enable_testing()
include(GoogleTest)

# ============================================================================
# Find Quesa Library
# ============================================================================

# On macOS, Quesa is typically built as a framework
# Support both framework and library builds
if(APPLE)
    # Try to find Quesa.framework first
    find_library(QUESA_FRAMEWORK
        NAMES Quesa
        PATHS
            # Sibling Quesa SDK (pre-built frameworks)
            ${CMAKE_SOURCE_DIR}/../Quesa/SDK/Libraries/Mac/Framework_Modern/Release
            ${CMAKE_SOURCE_DIR}/../Quesa/SDK/Libraries/Mac/Framework_Modern/Debug
            # Xcode build locations
            ${CMAKE_SOURCE_DIR}/../build/Release
            ${CMAKE_SOURCE_DIR}/../build/Debug
            ${CMAKE_SOURCE_DIR}/../Build/Release
            ${CMAKE_SOURCE_DIR}/../Build/Debug
            # DerivedData (Xcode default build location)
            ~/Library/Developer/Xcode/DerivedData/*/Build/Products/Release
            ~/Library/Developer/Xcode/DerivedData/*/Build/Products/Debug
            # Standard framework locations
            /Library/Frameworks
            ~/Library/Frameworks
            /usr/local/Frameworks
        PATH_SUFFIXES
            Frameworks
        NO_DEFAULT_PATH
    )
    
    if(QUESA_FRAMEWORK)
        set(QUESA_LIBRARY ${QUESA_FRAMEWORK})
        # Framework headers are in Quesa.framework/Headers
        get_filename_component(QUESA_FRAMEWORK_DIR ${QUESA_FRAMEWORK} DIRECTORY)
        set(QUESA_INCLUDE_DIR "${QUESA_FRAMEWORK}/Headers")
        set(QUESA_FOUND TRUE)
        message(STATUS "Found Quesa framework: ${QUESA_FRAMEWORK}")
        message(STATUS "Quesa headers: ${QUESA_INCLUDE_DIR}")
    endif()
endif()

# Fallback: Try to find Quesa as a regular library
if(NOT QUESA_FOUND)
    find_library(QUESA_LIBRARY
        NAMES Quesa libQuesa
        PATHS
            /usr/local/lib
            /usr/lib
            ${CMAKE_SOURCE_DIR}/lib
            ${CMAKE_SOURCE_DIR}/../lib
            ${CMAKE_SOURCE_DIR}/../build/Release
            ${CMAKE_SOURCE_DIR}/../build/Debug
        DOC "Quesa library"
    )

    find_path(QUESA_INCLUDE_DIR
        NAMES Quesa/Quesa.h Quesa.h QuesaController.h
        PATHS
            /usr/local/include
            /usr/include
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/../SDK/Includes/Quesa
            ${CMAKE_SOURCE_DIR}/../Development/Source/Core/Glue
            ${CMAKE_SOURCE_DIR}/../Source/Core/Glue
        PATH_SUFFIXES
            Quesa
        DOC "Quesa include directory"
    )
    
    if(QUESA_LIBRARY AND QUESA_INCLUDE_DIR)
        message(STATUS "Found Quesa library: ${QUESA_LIBRARY}")
        message(STATUS "Found Quesa includes: ${QUESA_INCLUDE_DIR}")
        set(QUESA_FOUND TRUE)
    endif()
endif()

# Allow manual override via environment or CMake variables
if(DEFINED ENV{QUESA_FRAMEWORK_PATH})
    set(QUESA_LIBRARY "$ENV{QUESA_FRAMEWORK_PATH}")
    set(QUESA_INCLUDE_DIR "$ENV{QUESA_FRAMEWORK_PATH}/Headers")
    set(QUESA_FOUND TRUE)
    message(STATUS "Using QUESA_FRAMEWORK_PATH from environment: ${QUESA_LIBRARY}")
endif()

if(NOT QUESA_FOUND)
    message(WARNING "Quesa library/framework not found - tests will not link")
    message(WARNING "  To specify Quesa.framework location:")
    message(WARNING "    cmake -DQUESA_LIBRARY=/path/to/Quesa.framework ..")
    message(WARNING "  Or set environment variable:")
    message(WARNING "    export QUESA_FRAMEWORK_PATH=/path/to/Quesa.framework")
endif()

# ============================================================================
# Compiler Flags
# ============================================================================

if(ENABLE_WARNINGS)
    if(MSVC)
        add_compile_options(/W4 /WX-)
    else()
        add_compile_options(-Wall -Wextra -Wpedantic)
    endif()
endif()

# Platform-specific definitions
if(APPLE)
    add_compile_definitions(BENCHX=1)
elseif(UNIX)
    add_compile_definitions(BENCHX=1)
elseif(WIN32)
    add_compile_definitions(BENCHX=1)
endif()

# Coverage flags
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage)
        add_link_options(--coverage)
    endif()
endif()

# ============================================================================
# Test Fixture Library
# ============================================================================

add_library(ControllerTestFixture STATIC
    testsControllerTestFixture.cpp
)

target_include_directories(ControllerTestFixture PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QUESA_INCLUDE_DIR}
)

target_link_libraries(ControllerTestFixture PUBLIC
    gtest
    "-framework Cocoa"
)

if(QUESA_FOUND)
    target_link_libraries(ControllerTestFixture PUBLIC ${QUESA_LIBRARY})
endif()

# ============================================================================
# Google Test - Controller Core Tests
# ============================================================================

add_executable(controller_gtest
    ControllerCoreTest.cpp
)

target_include_directories(controller_gtest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QUESA_INCLUDE_DIR}
)

target_link_libraries(controller_gtest PRIVATE
    ControllerTestFixture
    gtest
    gtest_main
)

if(QUESA_FOUND)
    target_link_libraries(controller_gtest PRIVATE ${QUESA_LIBRARY})
    if(APPLE)
        # Patch the embedded install name so the binary finds Quesa at its SDK location
        add_custom_command(TARGET controller_gtest POST_BUILD
            COMMAND install_name_tool -change
                "/Users/ole/Library/Frameworks/Quesa.framework/Versions/A/Quesa"
                "${QUESA_LIBRARY}/Versions/A/Quesa"
                $<TARGET_FILE:controller_gtest>
            COMMENT "Fixing Quesa framework install name in controller_gtest"
        )
    endif()
endif()

# Discover tests for CTest (PRE_TEST avoids running binary at build time)
gtest_discover_tests(controller_gtest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DISCOVERY_MODE PRE_TEST
    PROPERTIES
        LABELS "controller;gtest"
)

# ============================================================================
# Oracle Tests (MinUnit) - Optional
# ============================================================================

if(BUILD_ORACLE_TESTS)
    # Check if oracle test files exist
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/uniBench.c")
        add_executable(controller_oracle
            uniBench.c
        )
        
        target_include_directories(controller_oracle PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${QUESA_INCLUDE_DIR}
        )
        
        target_compile_definitions(controller_oracle PRIVATE
            BENCHX=1
        )
        
        if(QUESA_FOUND)
            target_link_libraries(controller_oracle PRIVATE ${QUESA_LIBRARY})
        endif()
        
        # Add as a test
        add_test(
            NAME controller_oracle_minunit
            COMMAND controller_oracle
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        
        set_tests_properties(controller_oracle_minunit PROPERTIES
            LABELS "controller;oracle;minunit"
        )
        
        message(STATUS "Oracle tests (MinUnit) will be built")
    else()
        message(STATUS "Oracle test file not found - skipping oracle tests")
    endif()
endif()

# ============================================================================
# Custom Targets
# ============================================================================

# Run all tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS controller_gtest
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all tests..."
)

# Run only Google Tests
add_custom_target(run_gtest
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L gtest
    DEPENDS controller_gtest
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running Google Tests..."
)

# Run only Oracle Tests
if(BUILD_ORACLE_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/uniBench.c")
    add_custom_target(run_oracle
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L oracle
        DEPENDS controller_oracle
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running Oracle Tests (MinUnit)..."
    )
endif()

# Run tests in verbose mode
add_custom_target(run_tests_verbose
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS controller_gtest
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all tests (verbose)..."
)

# Generate test report
add_custom_target(test_report
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --output-junit test_results.xml
    DEPENDS controller_gtest
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating test report..."
)

# ============================================================================
# Installation
# ============================================================================

install(TARGETS controller_gtest
    RUNTIME DESTINATION bin
)

if(BUILD_ORACLE_TESTS AND TARGET controller_oracle)
    install(TARGETS controller_oracle
        RUNTIME DESTINATION bin
    )
endif()

# Install documentation
install(FILES
    MIGRATION_STATUS.md
    TEST_030_MIGRATION.md
    TEST_031-040_MIGRATION.md
    MIGRATION_SUMMARY.md
    COMPLETION_REPORT.md
    DESTINATION share/doc/ControllerCoreUnitTest
)

# ============================================================================
# Print Configuration Summary
# ============================================================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "ControllerCoreUnitTest Configuration")
message(STATUS "========================================")
message(STATUS "  CMake version:        ${CMAKE_VERSION}")
message(STATUS "  C++ standard:         C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Build type:           ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler:             ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "")
message(STATUS "Dependencies:")
if(GTest_FOUND)
message(STATUS "  Google Test:          Found (system: ${GTest_DIR})")
else()
message(STATUS "  Google Test:          Found (via FetchContent)")
endif()
message(STATUS "  Quesa library:        ${QUESA_FOUND}")
if(QUESA_FOUND)
message(STATUS "    Library path:       ${QUESA_LIBRARY}")
message(STATUS "    Include path:       ${QUESA_INCLUDE_DIR}")
endif()
message(STATUS "")
message(STATUS "Build options:")
message(STATUS "  Oracle tests:         ${BUILD_ORACLE_TESTS}")
message(STATUS "  Compiler warnings:    ${ENABLE_WARNINGS}")
message(STATUS "  Code coverage:        ${ENABLE_COVERAGE}")
message(STATUS "")
message(STATUS "Targets:")
message(STATUS "  controller_gtest      - Google Test test suite")
if(BUILD_ORACLE_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/uniBench.c")
message(STATUS "  controller_oracle     - MinUnit oracle tests")
endif()
message(STATUS "  run_tests             - Run all tests")
message(STATUS "  run_gtest             - Run Google Tests only")
if(BUILD_ORACLE_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/uniBench.c")
message(STATUS "  run_oracle            - Run Oracle tests only")
endif()
message(STATUS "  test_report           - Generate test report (JUnit XML)")
message(STATUS "========================================")
message(STATUS "")
