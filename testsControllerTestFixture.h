// ControllerTestFixture.h
// Test fixture header for Controller Core Unit Tests
// Migrated from MinUnit to Google Test
// Oracle: QD3D 1.6 MacOS Classic

#ifndef CONTROLLER_TEST_FIXTURE_H
#define CONTROLLER_TEST_FIXTURE_H

// Note: gtest/gtest.h is provided by CMake via FetchContent
// Run: ./build.sh or cmake to configure and download Google Test
#include <gtest/gtest.h>
#include <cmath>
#include <cstring>
#include <iostream>

// Include appropriate headers based on platform
// This test suite is for QUESA ONLY - not legacy QuickDraw 3D
// Legacy QD3D headers (QD3D.h) cannot be used on modern macOS
#if !defined(BenchX)
    // If BenchX is not defined, assume Quesa (CMake will define this)
    #define BenchX 1
#endif

// Always use Quesa headers (not legacy QD3D)
#ifndef QUESA_OS_MACINTOSH
#define QUESA_OS_MACINTOSH 1
#endif

#ifndef Q3_DEBUG
#define Q3_DEBUG 1
#endif

#include <Quesa/Quesa.h>
#include <Quesa/QuesaController.h>
#include <Quesa/QuesaMath.h>

// ============================================================================
// Test Fixture Class
// ============================================================================

class ControllerCoreTestFixture : public ::testing::Test {
protected:
    // Setup runs once before all tests in the suite
    static void SetUpTestSuite();
    
    // Teardown runs once after all tests in the suite
    static void TearDownTestSuite();
    
    // ========================================================================
    // Shared test state (mirrors global variables from uniBench.c)
    // ========================================================================
    
    // Controller state
    static TQ3Boolean controllerListChanged;
    static TQ3Uns32 controllerListSerialNumber;
    static TQ3ControllerData ControllerData;
    static TQ3ControllerStateObject ControllerState;
    static TQ3Status Status3D;
    static TQ3ControllerRef tempController;
    static TQ3ControllerRef ClientController;
    static TQ3ControllerRef DriverController;
    static TQ3Boolean controllerActivation;
    
    // Tracker state
    static TQ3TrackerObject ClientTracker;
    static TQ3Boolean trackerActivation;
    static float posTresh;
    static float oriTresh;
    
    // Working variables
    static char tempString[250];
    static TQ3Uns32 tempUns32;
    static TQ3Uns32 TrackerSerNum;
    static TQ3Uns32 CtrlSerNum;
    static TQ3Boolean tempBool;
    static TQ3Boolean changed;
    static TQ3Point3D position;
    static TQ3Vector3D deltaPos;
    static TQ3Quaternion orientation;
    static TQ3Quaternion deltaOri;
    static float values[2];
    static TQ3Uns32 whatSet;
    static TQ3Uns32 whatGet;
    static TQ3Uns32 whatSize;
    
    // Callback tracking
    static TQ3Uns32 lastsize;
    static TQ3Uns32 what0;
    static TQ3Uns32 what1;
    static TQ3Uns32 what_work;
    static TQ3Uns32 trackerProcCalled;
    static TQ3Uns32 doBeep;
    
    // ========================================================================
    // Helper methods (callback functions)
    // ========================================================================
    
    static TQ3Status ChannelGetMethod(TQ3ControllerRef controllerRef,
                                      TQ3Uns32 channel,
                                      void *data,
                                      TQ3Uns32 *dataSize);
    
    static TQ3Status ChannelSetMethod(TQ3ControllerRef controllerRef,
                                      TQ3Uns32 channel,
                                      const void *data,
                                      TQ3Uns32 dataSize);
    
    static TQ3Status TrackerProc(TQ3TrackerObject trackerObject,
                                 TQ3ControllerRef controllerRef);
};

#endif // CONTROLLER_TEST_FIXTURE_H
