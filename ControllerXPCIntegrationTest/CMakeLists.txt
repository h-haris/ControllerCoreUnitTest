cmake_minimum_required(VERSION 3.14)
project(ControllerXPCIntegrationTest LANGUAGES CXX OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_OBJCXX_STANDARD 17)

# ============================================================================
# Find Quesa Framework (Framework-XPC, Debug preferred)
# ============================================================================

find_library(QUESA_FRAMEWORK
    NAMES Quesa
    PATHS
        ${CMAKE_SOURCE_DIR}/../Quesa/SDK/Libraries/Mac/Framework_Modern/Debug
        ${CMAKE_SOURCE_DIR}/../Quesa/SDK/Libraries/Mac/Framework_Modern/Release
    NO_DEFAULT_PATH
)

if(DEFINED ENV{QUESA_FRAMEWORK_PATH})
    set(QUESA_FRAMEWORK "$ENV{QUESA_FRAMEWORK_PATH}")
endif()

if(NOT QUESA_FRAMEWORK)
    message(FATAL_ERROR
        "Quesa.framework not found. Build the Framework-XPC target first:\n"
        "  xcodebuild -project Quesa/Development/Projects/Mac/Quesa.xcodeproj "
        "-scheme Framework-XPC -configuration Debug build\n"
        "Or set QUESA_FRAMEWORK_PATH.")
endif()
message(STATUS "Using Quesa: ${QUESA_FRAMEWORK}")

set(QUESA_INCLUDE_DIR "${QUESA_FRAMEWORK}/Headers")

# ============================================================================
# Find Google Test
# ============================================================================

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

enable_testing()
include(GoogleTest)

# ============================================================================
# Helper macro: fix Quesa framework install name
# ============================================================================

macro(fix_quesa_install_name target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND install_name_tool -change
            "/Users/ole/Library/Frameworks/Quesa.framework/Versions/A/Quesa"
            "${QUESA_FRAMEWORK}/Versions/A/Quesa"
            $<TARGET_FILE:${target}>
        COMMENT "Fixing Quesa framework install name in ${target}"
    )
endmacro()

# ============================================================================
# quesa_xpc_driver — driver helper process (OBJCXX)
# ============================================================================

add_executable(quesa_xpc_driver XPCDriverHelper.mm)

target_compile_definitions(quesa_xpc_driver PRIVATE QUESA_USE_XPC=1)

target_include_directories(quesa_xpc_driver PRIVATE ${QUESA_INCLUDE_DIR})

target_link_libraries(quesa_xpc_driver PRIVATE
    "${QUESA_FRAMEWORK}"
    "-framework Foundation"
)

fix_quesa_install_name(quesa_xpc_driver)

# ============================================================================
# controller_xpc_integration_gtest — client test binary (CXX)
# ============================================================================

add_executable(controller_xpc_integration_gtest XPCIntegrationTest.cpp)

target_include_directories(controller_xpc_integration_gtest PRIVATE
    ${QUESA_INCLUDE_DIR}
)

target_link_libraries(controller_xpc_integration_gtest PRIVATE
    "${QUESA_FRAMEWORK}"
    gtest
    gtest_main
    "-framework Foundation"
)

fix_quesa_install_name(controller_xpc_integration_gtest)

gtest_discover_tests(controller_xpc_integration_gtest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS "xpc;integration"
)

# ============================================================================
# Summary
# ============================================================================

message(STATUS "")
message(STATUS "ControllerXPCIntegrationTest targets:")
message(STATUS "  quesa_xpc_driver               - driver helper process")
message(STATUS "  controller_xpc_integration_gtest - cross-process client tests")
message(STATUS "")
message(STATUS "Run with:  ./run_xpc_integration.sh")
message(STATUS "")
