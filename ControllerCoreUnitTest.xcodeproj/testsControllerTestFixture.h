// ControllerTestFixture.h
// Test fixture for Controller Core Unit Tests
// Migrated from MinUnit to Google Test
// Oracle: MacOS classic QD3D 1.6 (uniBench.c)

#ifndef CONTROLLER_TEST_FIXTURE_H
#define CONTROLLER_TEST_FIXTURE_H

#include <gtest/gtest.h>
#include <cmath>

// Include Quesa headers
#ifdef BenchX
#include <Quesa/Quesa.h>
#include <Quesa/QuesaController.h>
#include <Quesa/QuesaMath.h>
#else
#include <QD3D.h>
#include <QD3DController.h>
#include <QD3DMath.h>
#endif

// Floating point comparison epsilon
#define FL_EPSILON (0.00001)

// Floating point equality check
#define FL_EQUAL(result, expected) \
    (std::fabs((result) - (expected)) < FL_EPSILON)

// Success/Failure checks with better messages
#define EXPECT_Q3_SUCCESS(status) \
    EXPECT_EQ(kQ3Success, (status)) << "Expected kQ3Success but got kQ3Failure"

#define EXPECT_Q3_FAILURE(status) \
    EXPECT_EQ(kQ3Failure, (status)) << "Expected kQ3Failure but got kQ3Success"

// Platform-specific expectations for oracle compatibility
#ifdef __MWERKS__
    #define EXPECT_PLATFORM_CONTROLLER_EQUALITY(client, driver) \
        EXPECT_EQ(client, driver) << "Controllers should be equal on CodeWarrior"
#else
    #define EXPECT_PLATFORM_CONTROLLER_EQUALITY(client, driver) \
        EXPECT_EQ(kCFCompareEqualTo, \
                  CFStringCompare((CFStringRef)client, (CFStringRef)driver, \
                                  kCFCompareCaseInsensitive)) \
            << "Controllers should be equal on modern platforms"
#endif

/**
 * @brief Test fixture for Controller Core Unit Tests
 * 
 * This fixture maintains shared state across all tests, which must run
 * sequentially in order. The tests are based on the oracle implementation
 * from QD3D 1.6 on MacOS Classic.
 * 
 * IMPORTANT: Tests must run in order! Use TEST_F in declaration order.
 */
class ControllerCoreTestFixture : public ::testing::Test {
protected:
    // SetUpTestSuite runs once before all tests in this fixture
    static void SetUpTestSuite();
    
    // TearDownTestSuite runs once after all tests in this fixture
    static void TearDownTestSuite();
    
    // ========================================================================
    // Shared test state (static members persist across all tests)
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
    // Controller callback methods (must be static for C API compatibility)
    // ========================================================================
    
    /**
     * @brief Channel get method for controller
     * Used by Q3Controller_GetChannel to retrieve channel data
     */
    static TQ3Status ChannelGetMethod(TQ3ControllerRef controllerRef,
                                      TQ3Uns32 channel,
                                      void *data,
                                      TQ3Uns32 *dataSize);
    
    /**
     * @brief Channel set method for controller
     * Used by Q3Controller_SetChannel to set channel data
     */
    static TQ3Status ChannelSetMethod(TQ3ControllerRef controllerRef,
                                      TQ3Uns32 channel,
                                      const void *data,
                                      TQ3Uns32 dataSize);
    
    /**
     * @brief Tracker notification callback
     * Called when tracker state changes
     */
    static TQ3Status TrackerProc(TQ3TrackerObject trackerObject,
                                 TQ3ControllerRef controllerRef);
};

#endif // CONTROLLER_TEST_FIXTURE_H
