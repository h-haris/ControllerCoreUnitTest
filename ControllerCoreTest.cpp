// ControllerCoreTest.cpp
// Controller Core Unit Tests - Migrated from MinUnit to Google Test
// Oracle: MacOS classic QD3D 1.6 (uniBench.c)
//
// IMPORTANT: Tests must run in declaration order as they share state!
// Each test builds upon the state from previous tests.

#include "testsControllerTestFixture.h"
#include "testsControllerTestHelpers.h"
#include <gtest/gtest.h>
#include <cstring>
#include <iostream>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

// ============================================================================
// Test Suite: Controller Core Tests
// Oracle: test_000 through test_043 from uniBench.c
// ============================================================================

/**
 * @brief test_000: Initialize Quesa/QD3D
 *
 * Oracle test validates that Q3Initialize succeeds.
 * This is the foundation for all subsequent tests.
 */
TEST_F(ControllerCoreTestFixture, Test_000_Initialize) {
    Status3D = Q3Initialize();
    EXPECT_Q3_SUCCESS(Status3D)
        << "Q3Initialize should succeed - required for all tests";
}

/**
 * @brief test_001: Q3Controller_Next with NULL should return NULL controller
 *
 * Oracle behavior: When no controllers are registered, calling Q3Controller_Next
 * with NULL should succeed but return NULL in ClientController.
 */
TEST_F(ControllerCoreTestFixture, Test_001_ControllerNext_Null) {
    Status3D = Q3Controller_Next(NULL, &ClientController);

    EXPECT_Q3_SUCCESS(Status3D)
        << "Q3Controller_Next should succeed even with no controllers";

    EXPECT_EQ(nullptr, ClientController)
        << "ClientController should be NULL when no controllers are registered";
}

/**
 * @brief test_002: Q3Controller_GetListChanged - Initial state check
 *
 * Oracle behavior:
 * - CodeWarrior (__MWERKS__): controllerListChanged == kQ3True, serialNumber == 1
 * - Modern (Quesa): controllerListChanged == kQ3False, serialNumber == 0
 *
 * This platform difference reflects initialization differences between QD3D and Quesa.
 */
TEST_F(ControllerCoreTestFixture, Test_002_GetListChanged_Initial) {
    Status3D = Q3Controller_GetListChanged(&controllerListChanged,
                                          &controllerListSerialNumber);

    EXPECT_Q3_SUCCESS(Status3D)
        << "Q3Controller_GetListChanged should succeed";

#ifdef __MWERKS__
    // Oracle: QD3D on CodeWarrior
    EXPECT_TRUE(controllerListChanged)
        << "On CodeWarrior, controllerListChanged should be kQ3True initially";

    EXPECT_EQ(1u, controllerListSerialNumber)
        << "On CodeWarrior, controllerListSerialNumber should be 1";
#else
    // Oracle: Quesa on modern systems
    EXPECT_FALSE(controllerListChanged)
        << "On modern systems, controllerListChanged should be kQ3False initially";

    EXPECT_EQ(0u, controllerListSerialNumber)
        << "On modern systems, controllerListSerialNumber should be 0";
#endif
}

/**
 * @brief test_003: Q3Controller_Next with tempController
 *
 * Oracle behavior: Calling Q3Controller_Next with tempController (which is NULL
 * or -1 cast to TQ3ControllerRef) should succeed but return NULL, as there are
 * still no controllers registered.
 *
 * Note: The original test set tempController to (TQ3ControllerRef)-1 initially,
 * but SetUpTestSuite() sets it to nullptr.
 */
TEST_F(ControllerCoreTestFixture, Test_003_ControllerNext_TempController) {
    Status3D = Q3Controller_Next(tempController, &ClientController);

    EXPECT_Q3_SUCCESS(Status3D)
        << "Q3Controller_Next should succeed";

    EXPECT_EQ(nullptr, ClientController)
        << "ClientController should still be NULL - no controllers available yet "
        << "(should be NULL in OSX too, per oracle comment)";
}

/**
 * @brief test_004: Create a new controller
 *
 * Oracle behavior: Fill TQ3ControllerData structure and create a new controller.
 * - signature: "com.HMDP.BenchController"
 * - valueCount: 16 (max is unknown for QD3D, seems unlimited - 4096 worked)
 * - channelCount: 4
 * - Assigns channel get/set methods
 */
TEST_F(ControllerCoreTestFixture, Test_004_CreateController) {
    // Fill struct TQ3ControllerData for Controller Creation
    ControllerData.signature = "com.HMDP.BenchController\0";
    ControllerData.valueCount = 16; // max valueCount unknown for QD3D! seems unlimited (4096 worked)
    ControllerData.channelCount = 4;
    ControllerData.channelGetMethod = ChannelGetMethod;
    ControllerData.channelSetMethod = ChannelSetMethod;

    DriverController = Q3Controller_New(&ControllerData);

    EXPECT_NE(nullptr, DriverController)
        << "Q3Controller_New should return a valid controller reference";
}

/**
 * @brief test_005: Controller list changed after controller creation
 *
 * Oracle behavior: After creating a controller, the list should have changed.
 * - controllerListChanged should be kQ3True
 * - Serial number differs by platform:
 *   - QD3D (CodeWarrior): serialNumber == 2
 *   - Quesa (modern): serialNumber == 1
 */
TEST_F(ControllerCoreTestFixture, Test_005_GetListChanged_AfterCreate) {
    Status3D = Q3Controller_GetListChanged(&controllerListChanged,
                                          &controllerListSerialNumber);

    EXPECT_Q3_SUCCESS(Status3D);

    EXPECT_TRUE(controllerListChanged)
        << "controllerListChanged should be kQ3True after controller creation";

    // controllerListSerialNumber = (QD3D: 2; Quesa: 1)
#ifdef __MWERKS__
    EXPECT_EQ(2u, controllerListSerialNumber)
        << "On QD3D, controllerListSerialNumber should be 2";
#else
    EXPECT_EQ(1u, controllerListSerialNumber)
        << "On Quesa, controllerListSerialNumber should be 1";
#endif
}

/**
 * @brief test_006: Enumerate and find the new controller
 *
 * Oracle behavior: Q3Controller_Next(NULL) should now return our controller.
 * Platform-specific comparison:
 * - CodeWarrior: Direct pointer comparison
 * - Modern: CFString comparison
 */
TEST_F(ControllerCoreTestFixture, Test_006_ControllerNext_FindController) {
    Status3D = Q3Controller_Next(NULL, &ClientController);

    EXPECT_Q3_SUCCESS(Status3D);

    EXPECT_PLATFORM_CONTROLLER_EQUALITY(ClientController, DriverController);
}

/**
 * @brief test_007: Create controller state object
 *
 * Oracle behavior: Q3ControllerState_New should create a valid state object
 * for saving/restoring controller state.
 */
TEST_F(ControllerCoreTestFixture, Test_007_CreateControllerState) {
    ControllerState = Q3ControllerState_New(ClientController);

    EXPECT_NE(nullptr, ControllerState)
        << "Q3ControllerState_New should return a valid state object";
}

/**
 * @brief test_008: Check for additional controllers
 *
 * Oracle behavior: Calling Q3Controller_Next with ClientController should
 * check if there are more controllers.
 * - QD3D: tempController != NULL (system cursor controller exists)
 * - Quesa: tempController == NULL (no system cursor in OSX)
 */
TEST_F(ControllerCoreTestFixture, Test_008_ControllerNext_NoMoreControllers) {
    Status3D = Q3Controller_Next(ClientController, &tempController);

    EXPECT_Q3_SUCCESS(Status3D);

    /*
     * tempController:
     * QD3D:  != NULL;
     * Quesa: Should be NULL in OSX - no System Cursor?!
     */
    EXPECT_EQ(nullptr, tempController)
        << "No additional controllers should be available (Quesa behavior)";
}

/**
 * @brief test_009: Create a new tracker
 *
 * Oracle behavior: Q3Tracker_New creates a tracker with the TrackerProc callback.
 * This tracker will be associated with the controller later.
 */
TEST_F(ControllerCoreTestFixture, Test_009_CreateTracker) {
    // New Tracker
    ClientTracker = Q3Tracker_New(TrackerProc);

    EXPECT_NE(nullptr, ClientTracker)
        << "Q3Tracker_New should return a valid tracker object";
}

/**
 * @brief test_010: Tracker notification thresholds
 *
 * Oracle behavior: Test getting/setting tracker notify thresholds.
 * - Initial thresholds should be 0.0
 * - Set to 0.1, verify they're set correctly
 * - Reset to 0.0
 */
TEST_F(ControllerCoreTestFixture, Test_010_TrackerNotifyThresholds) {
    // Get initial thresholds (should be 0.0)
    Status3D = Q3Tracker_GetNotifyThresholds(ClientTracker, &posTresh, &oriTresh);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FLOAT_EQ(0.0f, posTresh) << "Initial posTresh should be 0.0";
    EXPECT_FLOAT_EQ(0.0f, oriTresh) << "Initial oriTresh should be 0.0";

    // Set thresholds to 0.1
    posTresh = 0.1f;
    oriTresh = 0.1f;
    Status3D = Q3Tracker_SetNotifyThresholds(ClientTracker, posTresh, oriTresh);
    EXPECT_Q3_SUCCESS(Status3D);

    // Verify thresholds were set
    Status3D = Q3Tracker_GetNotifyThresholds(ClientTracker, &posTresh, &oriTresh);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_TRUE(FL_EQUAL(posTresh, 0.1f))
        << "posTresh should be 0.1, got " << posTresh;
    EXPECT_TRUE(FL_EQUAL(oriTresh, 0.1f))
        << "oriTresh should be 0.1, got " << oriTresh;

    // Reset thresholds to 0.0
    posTresh = 0.0f;
    oriTresh = 0.0f;
    Status3D = Q3Tracker_SetNotifyThresholds(ClientTracker, posTresh, oriTresh);
    EXPECT_Q3_SUCCESS(Status3D);
}

/**
 * @brief test_011: Deactivate tracker
 *
 * Oracle behavior: Set tracker activation to kQ3False.
 */
TEST_F(ControllerCoreTestFixture, Test_011_DeactivateTracker) {
    // Deactivate Tracker
    Status3D = Q3Tracker_SetActivation(ClientTracker, kQ3False);

    EXPECT_Q3_SUCCESS(Status3D);
}

/**
 * @brief test_012: Verify tracker is deactivated
 *
 * Oracle behavior: Q3Tracker_GetActivation should return kQ3False.
 */
TEST_F(ControllerCoreTestFixture, Test_012_GetTrackerActivation_False) {
    Status3D = Q3Tracker_GetActivation(ClientTracker, &trackerActivation);

    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FALSE(trackerActivation)
        << "trackerActivation should be kQ3False after deactivation";
}

/**
 * @brief test_013: Check if controller has tracker (before association)
 *
 * Oracle behavior: Q3Controller_HasTracker indicates both active controller
 * AND active tracker.
 * - QD3D (CodeWarrior): Returns kQ3True (unexpected behavior)
 * - Quesa: Returns kQ3False (no tracker associated yet)
 */
TEST_F(ControllerCoreTestFixture, Test_013_HasTracker_BeforeAssociation) {
    // Indicates an active controller !and! an active tracker
    Status3D = Q3Controller_HasTracker(DriverController, &tempBool);

    EXPECT_Q3_SUCCESS(Status3D);

#ifdef __MWERKS__
    EXPECT_TRUE(tempBool)
        << "On QD3D, tempBool == kQ3True (Controller has no associated tracker!)";
#else
    // Quesa: tempBool = kQ3False; no Tracker associated!
    EXPECT_FALSE(tempBool)
        << "On Quesa, tempBool == kQ3False (Controller has an associated tracker!)";
#endif
}

/**
 * @brief test_014: Associate tracker with controller
 *
 * Oracle behavior: Q3Controller_SetTracker associates ClientTracker with ClientController.
 */
TEST_F(ControllerCoreTestFixture, Test_014_SetTracker) {
    Status3D = Q3Controller_SetTracker(ClientController, ClientTracker);

    EXPECT_Q3_SUCCESS(Status3D);
}

/**
 * @brief test_015: Controller activation after tracker association
 *
 * Oracle behavior: After setting tracker, controller should be activated (kQ3True).
 */
TEST_F(ControllerCoreTestFixture, Test_015_GetControllerActivation_True) {
    Status3D = Q3Controller_GetActivation(ClientController, &controllerActivation);

    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_TRUE(controllerActivation)
        << "controllerActivation should be kQ3True after tracker association";
}

/**
 * @brief test_016: Check tracker status (inactive tracker)
 *
 * Oracle behavior: Q3Controller_HasTracker should return kQ3False because
 * tracker is inactive (deactivated in test_011).
 */
TEST_F(ControllerCoreTestFixture, Test_016_HasTracker_InactiveTracker) {
    // Inactive Tracker
    Status3D = Q3Controller_HasTracker(DriverController, &tempBool);

    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FALSE(tempBool)
        << "Controller should not have active tracker (tracker is inactive)";
}

/**
 * @brief test_017: Activate tracker and verify
 *
 * Oracle behavior:
 * - Set tracker activation to kQ3True
 * - Verify activation status
 * - Verify Q3Controller_HasTracker now returns kQ3True
 */
TEST_F(ControllerCoreTestFixture, Test_017_ActivateTracker) {
    // Active Tracker
    Status3D = Q3Tracker_SetActivation(ClientTracker, kQ3True);
    EXPECT_Q3_SUCCESS(Status3D);

    Status3D = Q3Tracker_GetActivation(ClientTracker, &trackerActivation);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_TRUE(trackerActivation)
        << "Tracker should be active (kQ3True)";

    Status3D = Q3Controller_HasTracker(DriverController, &tempBool);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_TRUE(tempBool)
        << "Controller should have active tracker now";
}

/**
 * @brief test_018: Get controller signature
 *
 * Oracle behavior: Q3Controller_GetSignature retrieves the signature string.
 * - Test with buffer size 5: should get "com."
 * - Test with buffer size 128: should get full "com.HMDP.BenchController"
 */
TEST_F(ControllerCoreTestFixture, Test_018_GetControllerSignature) {
    // Test with small buffer (5 bytes)
    Status3D = Q3Controller_GetSignature(ClientController, tempString, 5);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_STREQ("com.", tempString)
        << "Signature with buffer size 5 should return 'com.'";

    // Test with full buffer (128 bytes)
    Status3D = Q3Controller_GetSignature(ClientController, tempString, 128);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_STREQ("com.HMDP.BenchController", tempString)
        << "Full signature should be 'com.HMDP.BenchController'";
}

/**
 * @brief test_019: Get controller value count
 *
 * Oracle behavior: Q3Controller_GetValueCount should return 16 (set in test_004).
 */
TEST_F(ControllerCoreTestFixture, Test_019_GetValueCount) {
    Status3D = Q3Controller_GetValueCount(ClientController, &tempUns32);

    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(16u, tempUns32)
        << "ValueCount should be 16 (as set in controller creation)";
}

/**
 * @brief test_020: Track 2D/3D cursor
 *
 * Oracle behavior: Check if controller tracks 2D or 3D cursor.
 * Both should return kQ3False for this controller.
 */
TEST_F(ControllerCoreTestFixture, Test_020_TrackCursor) {
    Status3D = Q3Controller_Track2DCursor(DriverController, &tempBool);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FALSE(tempBool) << "Controller should not track 2D cursor";

    Status3D = Q3Controller_Track3DCursor(DriverController, &tempBool);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FALSE(tempBool) << "Controller should not track 3D cursor";
}

/**
 * @brief test_021: Set and get buttons
 *
 * Oracle behavior: Test button state management.
 * - Set buttons to 5 (should trigger TrackerProc)
 * - Verify buttons can be retrieved multiple times
 * - Oracle note: "Take a look at Q3Tracker_ChangeButtons, too"
 */
TEST_F(ControllerCoreTestFixture, Test_021_SetGetButtons) {
    TQ3Uns32 lastTrackerProcCalled = trackerProcCalled;

    // Set buttons (should trigger callback)
    Status3D = Q3Controller_SetButtons(DriverController, 5);
    // Take a look at Q3Tracker_ChangeButtons, too
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_NE(lastTrackerProcCalled, trackerProcCalled)
        << "trackerProc should have been called after SetButtons";

    // Get buttons via Tracker
    tempUns32 = 0;
    Status3D = Q3Tracker_GetButtons(ClientTracker, &tempUns32);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(5u, tempUns32) << "Buttons should be 5";

    // Get buttons via Controller
    tempUns32 = 0;
    Status3D = Q3Controller_GetButtons(ClientController, &tempUns32);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(5u, tempUns32) << "Buttons should be 5";

    // Get buttons again via Tracker
    tempUns32 = 0;
    Status3D = Q3Tracker_GetButtons(ClientTracker, &tempUns32);
    EXPECT_Q3_SUCCESS(Status3D);
#if 1
    EXPECT_EQ(5u, tempUns32) << "Buttons should still be 5";
#else
    // Alternative: buttons cleared after first read
    EXPECT_EQ(0u, tempUns32)
        << "Buttons should be cleared (returned call before, tracker set buttons to zero)";
#endif
}

/**
 * @brief test_022: Tracker position and event coordinates
 *
 * Oracle behavior: Test tracker position management and event coordinates.
 * - Get initial position (should be 0,0,0)
 * - Set event coordinates
 * - Set tracker position (should trigger TrackerProc)
 */
TEST_F(ControllerCoreTestFixture, Test_022_TrackerPosition_Initial) {
    TQ3Uns32 lastTrackerProcCalled;

    TrackerSerNum = 0;
    Status3D = Q3Tracker_GetPosition(ClientTracker, &position, NULL, NULL, &TrackerSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_POINT3D_EQ(position, 0.0f, 0.0f, 0.0f); // Initial position should be (0,0,0)
    EXPECT_EQ(1u, TrackerSerNum) << "TrackerSerNum should be 1";

    // Get event coordinates (should fail - no event set yet)
    Status3D = Q3Tracker_GetEventCoordinates(ClientTracker, 10, NULL, NULL, NULL);
    EXPECT_Q3_FAILURE(Status3D) << "GetEventCoordinates should fail with no event";

    // Set event coordinates
    tempUns32 = 5;
    Status3D = Q3Tracker_SetEventCoordinates(ClientTracker, 10, tempUns32, &position, NULL);
    EXPECT_Q3_SUCCESS(Status3D);

    // Set tracker position (should trigger callback)
    lastTrackerProcCalled = trackerProcCalled;
    position.x = 1.0f;
    position.y = 1.0f;
    position.z = 1.0f;
    Status3D = Q3Controller_SetTrackerPosition(DriverController, &position);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_NE(lastTrackerProcCalled, trackerProcCalled)
        << "trackerProc should have been called after SetTrackerPosition";
}

/**
 * @brief test_023: Get tracker position after setting
 *
 * Oracle behavior: Verify position was set correctly and deltaPos is tracked.
 * - position should be (1,1,1)
 * - deltaPos should be (0,0,0) (delta from previous read)
 * - changed should be kQ3True
 * - TrackerSerNum should be 2
 */
TEST_F(ControllerCoreTestFixture, Test_023_TrackerPosition_AfterSet) {
    Status3D = Q3Tracker_GetPosition(ClientTracker, &position, &deltaPos, &changed, &TrackerSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_POINT3D_EQ(position, 1.0f, 1.0f, 1.0f); // Position should be (1,1,1)
    EXPECT_VECTOR3D_EQ(deltaPos, 0.0f, 0.0f, 0.0f); // DeltaPos should be (0,0,0)
    EXPECT_TRUE(changed) << "changed should be kQ3True";
    EXPECT_EQ(2u, TrackerSerNum) << "TrackerSerNum should be 2";

    // Set event coordinates for time 20
    Status3D = Q3Tracker_SetEventCoordinates(ClientTracker, 20, (TQ3Uns32)0, &position, NULL);
    EXPECT_Q3_SUCCESS(Status3D);
}

/**
 * @brief test_024: Move tracker position (relative movement)
 *
 * Oracle behavior: Test Q3Controller_MoveTrackerPosition for relative positioning.
 * - Get current position (1,1,1), should not have changed
 * - Move by delta (1,1,1), resulting in (2,2,2)
 * - Verify deltaPos is (0,0,0) - why no delta? Doc? Threshold? previously cleared?
 */
TEST_F(ControllerCoreTestFixture, Test_024_MoveTrackerPosition) {
    // Get current position
    Status3D = Q3Controller_GetTrackerPosition(ClientController, &position);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_POINT3D_EQ(position, 1.0f, 1.0f, 1.0f);

    // Get via tracker - should show no change
    Status3D = Q3Tracker_GetPosition(ClientTracker, &position, &deltaPos, &changed, &TrackerSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_POINT3D_EQ(position, 1.0f, 1.0f, 1.0f);
    EXPECT_VECTOR3D_EQ(deltaPos, 0.0f, 0.0f, 0.0f);
    EXPECT_FALSE(changed) << "Position hasn't changed";
    EXPECT_EQ(2u, TrackerSerNum);

    // Move position by delta
    deltaPos.x = 1.0f;
    deltaPos.y = 1.0f;
    deltaPos.z = 1.0f;
    Status3D = Q3Controller_MoveTrackerPosition(DriverController, &deltaPos);
    EXPECT_Q3_SUCCESS(Status3D);

    // Verify new position
    Status3D = Q3Controller_GetTrackerPosition(ClientController, &position);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_POINT3D_EQ(position, 2.0f, 2.0f, 2.0f);

    // Get via tracker
    Status3D = Q3Tracker_GetPosition(ClientTracker, &position, &deltaPos, &changed, &TrackerSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_POINT3D_EQ(position, 2.0f, 2.0f, 2.0f);
    EXPECT_VECTOR3D_EQ(deltaPos, 0.0f, 0.0f, 0.0f);
    // why no delta? Doc? Threshold? previously cleared?
    EXPECT_TRUE(changed);
    EXPECT_EQ(3u, TrackerSerNum);

    // Set event coordinates for time 30
    Status3D = Q3Tracker_SetEventCoordinates(ClientTracker, 30, (TQ3Uns32)0, &position, NULL);
    EXPECT_Q3_SUCCESS(Status3D);
}

/**
 * @brief test_025: Tracker orientation - initial state
 *
 * Oracle behavior: Test tracker orientation management.
 * - Get initial orientation (identity quaternion: 1,0,0,0)
 * - Set event coordinates
 * - Set orientation to identity
 * - Verify orientation and delta
 */
TEST_F(ControllerCoreTestFixture, Test_025_TrackerOrientation_Initial) {
    TrackerSerNum = 0;
    Status3D = Q3Tracker_GetOrientation(ClientTracker, &orientation, NULL, NULL, &TrackerSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_QUATERNION_EQ(orientation, 1.0f, 0.0f, 0.0f, 0.0f);
    // orientation = 1,-0,-0,-0
    EXPECT_EQ(1u, TrackerSerNum);

    // Set event coordinates for time 40
    Status3D = Q3Tracker_SetEventCoordinates(ClientTracker, 40, (TQ3Uns32)0, NULL, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);

    // Set orientation to identity
    Q3Quaternion_SetIdentity(&orientation); // orientation = 1,0,0,0
    Status3D = Q3Controller_SetTrackerOrientation(DriverController, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);

    // Get orientation
    Status3D = Q3Controller_GetTrackerOrientation(ClientController, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_QUATERNION_EQ(orientation, 1.0f, 0.0f, 0.0f, 0.0f);
    // orientation = 1,-0,-0,-0

    // Get via tracker
    Status3D = Q3Tracker_GetOrientation(ClientTracker, &orientation, &deltaOri, &changed, &TrackerSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_QUATERNION_EQ(orientation, 1.0f, 0.0f, 0.0f, 0.0f);
    EXPECT_QUATERNION_EQ(deltaOri, 1.0f, 0.0f, 0.0f, 0.0f);
    // deltaOri = 1,-0,-0,-0
    EXPECT_TRUE(changed);
    EXPECT_EQ(2u, TrackerSerNum);
}

/**
 * @brief test_026: Move tracker orientation (rotation)
 *
 * Oracle behavior: Test Q3Controller_MoveTrackerOrientation.
 * - Rotate 45 degrees around Y axis
 * - Expected quaternion: (-0.8733047, 0, -0.4871745, 0)
 * - Verify deltaOri returns to identity (1,0,0,0)
 */
TEST_F(ControllerCoreTestFixture, Test_026_MoveTrackerOrientation) {
    Q3Quaternion_SetRotate_Y(&deltaOri, 45); // deltaOri = -0.8733047,0,-0.4871745,0
    Status3D = Q3Controller_MoveTrackerOrientation(DriverController, &deltaOri);
    EXPECT_Q3_SUCCESS(Status3D);

    // Get orientation via controller
    Status3D = Q3Controller_GetTrackerOrientation(ClientController, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_QUATERNION_NEAR(orientation, -0.8733047f, 0.0f, -0.4871745f, 0.0f);
    // orientation = -0.8733047,-0,-0.4871745,-0

    // Get via tracker
    Status3D = Q3Tracker_GetOrientation(ClientTracker, &orientation, &deltaOri, &changed, &TrackerSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_QUATERNION_NEAR(orientation, -0.8733047f, 0.0f, -0.4871745f, 0.0f);
    // orientation = -0.8733047,-0,-0.4871745,-0
    EXPECT_QUATERNION_EQ(deltaOri, 1.0f, 0.0f, 0.0f, 0.0f);
    // deltaOri = 1,0,0,0
    EXPECT_TRUE(changed);
    EXPECT_EQ(3u, TrackerSerNum);

    // Set event coordinates for time 50
    Status3D = Q3Tracker_SetEventCoordinates(ClientTracker, 50, (TQ3Uns32)0, NULL, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);
}

/**
 * @brief test_027: Controller values
 *
 * Oracle behavior: Test getting and setting controller values.
 * - Get initial values (should be 0,0)
 * - Set values to 1.0, -1.0
 * - Verify values and serial number
 */
TEST_F(ControllerCoreTestFixture, Test_027_ControllerValues) {
    // Get initial values
    Status3D = Q3Controller_GetValues(ClientController, 2, values, NULL, &CtrlSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FLOAT_EQ(0.0f, values[0]) << "Initial value[0] should be 0";
    EXPECT_FLOAT_EQ(0.0f, values[1]) << "Initial value[1] should be 0";
    EXPECT_EQ(1u, CtrlSerNum);

    // Set values
    values[0] = 1.0f;
    values[1] = -1.0f;
    Status3D = Q3Controller_SetValues(DriverController, values, 2);
    EXPECT_Q3_SUCCESS(Status3D);

    // Get values
    Status3D = Q3Controller_GetValues(ClientController, 2, values, &changed, &CtrlSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FLOAT_EQ(1.0f, values[0]);
    EXPECT_FLOAT_EQ(-1.0f, values[1]);
    EXPECT_EQ(2u, CtrlSerNum);
}

/**
 * @brief test_028: Set channel
 *
 * Oracle behavior: Test Q3Controller_SetChannel.
 * Set channel 3 to value 1500.
 */
TEST_F(ControllerCoreTestFixture, Test_028_SetChannel) {
    whatSet = 1500;
    Status3D = Q3Controller_SetChannel(ClientController, 3, &whatSet, sizeof(whatSet));
    EXPECT_Q3_SUCCESS(Status3D);
}

/**
 * @brief test_029: Prepare for state save/restore test
 *
 * Oracle behavior: Set up controller state before testing Q3ControllerState_SaveAndReset.
 * - Reset position to (0,0,0)
 * - Set orientation to 45-degree Y rotation
 * - Set buttons to 5
 * - Save and reset state
 */
TEST_F(ControllerCoreTestFixture, Test_029_PrepareStateTest) {
    // preparation for the test of Q3ControllerState_SaveAndReset

    // Reset position
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 0.0f;
    Status3D = Q3Controller_SetTrackerPosition(DriverController, &position);
    EXPECT_Q3_SUCCESS(Status3D);

    // Set orientation to 45-degree Y rotation
    Q3Quaternion_SetIdentity(&orientation); // orientation = 1,0,0,0
    Q3Quaternion_SetRotate_Y(&orientation, 45); // orientation = -0.8733, 0, -0.4871, 0
    Status3D = Q3Controller_SetTrackerOrientation(DriverController, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);

    // Set buttons
    Status3D = Q3Controller_SetButtons(DriverController, 5);
    EXPECT_Q3_SUCCESS(Status3D);

    // Save and reset state
    Status3D = Q3ControllerState_SaveAndReset(ControllerState);
    EXPECT_Q3_SUCCESS(Status3D);
}

/**
 * @brief test_030: State restore test (MOST COMPLEX TEST)
 *
 * Oracle behavior: This test verifies Q3ControllerState_Restore by:
 * 1. Overriding the saved state with new values
 * 2. Restoring the saved state
 * 3. Checking if state changes between "save and reset" and "restore" are reverted
 *
 * Key oracle notes:
 * - QD3D overrides the returned status of Q3Controller_GetChannel with kQ3Success
 * - Deactivated controller should have no effect on returned position
 * - TODO in original: check if SetActivation(kQ3False) is needed
 *
 * This is the most complex test with ~70 lines of assertions covering:
 * - Channel state
 * - Position state (with platform-specific behavior)
 * - Orientation state
 * - Button state
 * - Value state
 * - Activation state
 */
TEST_F(ControllerCoreTestFixture, Test_030_StateRestore) {
    TQ3Uns32 lastTrackerProcCalled;

    // ========================================================================
    // PART 1: Override saved state with new values
    // ========================================================================

    // Override channel
    whatSet = 0;
    Status3D = Q3Controller_SetChannel(ClientController, 3, &whatSet, sizeof(whatSet));
    EXPECT_Q3_SUCCESS(Status3D);

    // Modify tracker position (should trigger callback)
    lastTrackerProcCalled = trackerProcCalled;
    position.x = 1.0f;
    position.y = 1.0f;
    position.z = 1.0f;
    Status3D = Q3Controller_SetTrackerPosition(DriverController, &position);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_NE(lastTrackerProcCalled, trackerProcCalled)
        << "trackerProc should have been called after SetTrackerPosition";

    // Modify tracker orientation (should trigger callback)
    lastTrackerProcCalled = trackerProcCalled;
    Q3Quaternion_SetIdentity(&orientation); // orientation = 1,0,0,0
    Status3D = Q3Controller_SetTrackerOrientation(DriverController, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_NE(lastTrackerProcCalled, trackerProcCalled)
        << "trackerProc should have been called after SetTrackerOrientation";

    // Modify buttons (should trigger callback)
    lastTrackerProcCalled = trackerProcCalled;
    Status3D = Q3Controller_SetButtons(DriverController, 7);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_NE(lastTrackerProcCalled, trackerProcCalled)
        << "trackerProc should have been called after SetButtons";

    // Verify buttons were set
    Status3D = Q3Controller_GetButtons(ClientController, &tempUns32);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(7u, tempUns32) << "Buttons should be set to 7";

    // Modify values
    values[0] = 5.0f;
    values[1] = -5.0f;
    Status3D = Q3Controller_SetValues(DriverController, values, 2);
    EXPECT_Q3_SUCCESS(Status3D);

    // Verify values were set
    Status3D = Q3Controller_GetValues(ClientController, 2, values, NULL, &CtrlSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FLOAT_EQ(5.0f, values[0]) << "values[0] should be 5.0";
    EXPECT_FLOAT_EQ(-5.0f, values[1]) << "values[1] should be -5.0";
    EXPECT_EQ(3u, CtrlSerNum) << "CtrlSerNum should be 3"; // CtrlSerNum = (QD3D = 2) 3

    // Deactivate controller (TODO in oracle: check if needed)
    Status3D = Q3Controller_SetActivation(ClientController, kQ3False);
    EXPECT_Q3_SUCCESS(Status3D);

    // ========================================================================
    // PART 2: Restore saved state
    // ========================================================================

    Status3D = Q3ControllerState_Restore(ControllerState);
    EXPECT_Q3_SUCCESS(Status3D);

    // ========================================================================
    // PART 3: Check if state changes between "save and reset" and "restore"
    //         are reverted
    // ========================================================================

    // Check activation (special case!)
    Status3D = Q3Controller_GetActivation(ClientController, &controllerActivation);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FALSE(controllerActivation)
        << "controllerActivation should be kQ3False (special case!)";

    // Check channel
    whatSize = sizeof(whatGet);
    Status3D = Q3Controller_GetChannel(ClientController, 3, &whatGet, &whatSize);
    // to be fixed: Results in bad access if DeviceServer is not started!
#if 1
    // QD3D overrides the returned status of Q3Controller_GetChannel with kQ3Success
    EXPECT_Q3_SUCCESS(Status3D)
        << "QD3D overrides the returned status to kQ3Success";
#else
    EXPECT_Q3_FAILURE(Status3D)
        << "Expected kQ3Failure but QD3D overrides to kQ3Success";
#endif
    EXPECT_EQ(4u, whatSize) << "whatSize should be 4";
    EXPECT_EQ(1500u, whatGet) << "whatGet should be 1500 (restored value)";

    // Check tracker position
    Status3D = Q3Controller_GetTrackerPosition(ClientController, &position);
    EXPECT_Q3_SUCCESS(Status3D);
#if 1
    // Deactivated controller should have no effect on returned position
    EXPECT_POINT3D_EQ(position, 1.0f, 1.0f, 1.0f);
    // Position should be (1,1,1) - deactivated controller has no effect on returned position
#else
    EXPECT_POINT3D_EQ(position, 0.0f, 0.0f, 0.0f);
    // Alternative: position should be (0,0,0)
#endif

    // Check tracker orientation
    Status3D = Q3Controller_GetTrackerOrientation(ClientController, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_QUATERNION_EQ(orientation, 1.0f, 0.0f, 0.0f, 0.0f);
    // Orientation should be identity quaternion

    // Check buttons
    Status3D = Q3Controller_GetButtons(ClientController, &tempUns32);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(7u, tempUns32) << "Buttons should still be 7";

    // Check values
    Status3D = Q3Controller_GetValues(ClientController, 2, values, NULL, &CtrlSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FLOAT_EQ(5.0f, values[0]) << "values[0] should still be 5.0";
    EXPECT_FLOAT_EQ(-5.0f, values[1]) << "values[1] should still be -5.0";
    EXPECT_EQ(3u, CtrlSerNum) << "CtrlSerNum should be 3"; // TBC: QD3D - CtrlSerNum = 3
}

/**
 * @brief test_031: Get event coordinates for multiple time values
 *
 * Oracle behavior: Test Q3Tracker_GetEventCoordinates for various time values.
 * This verifies that event coordinates set in previous tests (022-026) can be retrieved.
 *
 * Event times tested:
 * - time 10: buttons=5, position=(0,0,0) - set in test_022
 * - time 20: buttons=0, position=(1,1,1) - set in test_023
 * - time 30: buttons=0, position=(2,2,2) - set in test_024
 * - time 40: buttons=0, orientation=(1,0,0,0) - set in test_025
 * - time 50: buttons=0, orientation=(-0.8733,-0.4871) - set in test_026
 *
 * After all events are retrieved, calling GetEventCoordinates again should fail.
 */
TEST_F(ControllerCoreTestFixture, Test_031_GetEventCoordinates) {
    // Reset position for testing
    position.x = -1.0f;
    position.y = -1.0f;
    position.z = -1.0f;
    tempUns32 = 0;

    // Event at time 10 (set in test_022)
    Status3D = Q3Tracker_GetEventCoordinates(ClientTracker, 10, &tempUns32, &position, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(5u, tempUns32) << "Buttons at time 10 should be 5";
    EXPECT_POINT3D_EQ(position, 0.0f, 0.0f, 0.0f); // Position at time 10 should be (0,0,0)

    // Event at time 20 (set in test_023)
    Status3D = Q3Tracker_GetEventCoordinates(ClientTracker, 20, &tempUns32, &position, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(0u, tempUns32) << "Buttons at time 20 should be 0";
    EXPECT_POINT3D_EQ(position, 1.0f, 1.0f, 1.0f); // Position at time 20 should be (1,1,1)

    // Event at time 30 (set in test_024)
    Status3D = Q3Tracker_GetEventCoordinates(ClientTracker, 30, &tempUns32, &position, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(0u, tempUns32) << "Buttons at time 30 should be 0";
    EXPECT_POINT3D_EQ(position, 2.0f, 2.0f, 2.0f); // Position at time 30 should be (2,2,2)

    // Event at time 40 (set in test_025) - orientation only
    Status3D = Q3Tracker_GetEventCoordinates(ClientTracker, 44, &tempUns32, &position, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(0u, tempUns32) << "Buttons at time 44 should be 0";
    EXPECT_QUATERNION_EQ(orientation, 1.0f, 0.0f, 0.0f, 0.0f);
    // Orientation at time 44 should be identity

    // Event at time 50 (set in test_026) - orientation with rotation
#if 1
    Status3D = Q3Tracker_GetEventCoordinates(ClientTracker, 50, NULL, NULL, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_QUATERNION_NEAR(orientation, -0.8733047f, 0.0f, -0.4871745f, 0.0f);
    // Orientation at time 50 should be 45-degree Y rotation
#else
    Status3D = Q3Tracker_GetEventCoordinates(ClientTracker, 50, &tempUns32, &position, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(0u, tempUns32) << "Buttons at time 50 should be 0";
    EXPECT_QUATERNION_NEAR(orientation, -0.8733047f, 0.0f, -0.4871745f, 0.0f);
    EXPECT_POINT3D_EQ(position, 2.0f, 2.0f, 2.0f);
#endif

    // Try to get event at time 10 again - should fail (events consumed)
    Status3D = Q3Tracker_GetEventCoordinates(ClientTracker, 10, &tempUns32, &position, &orientation);
    EXPECT_Q3_FAILURE(Status3D)
        << "GetEventCoordinates should fail - event coordinates no longer available";
}

/**
 * @brief test_032: Channel Get/Set methods
 *
 * Oracle behavior: Comprehensive test of channel communication.
 * Channels are zero-indexed (0 to channelCount-1).
 *
 * Key oracle notes:
 * - Channels count from 0 (zero)
 * - ChannelCount == 4, so valid channels are 0-3
 * - Channel 4 is "ambitious" - out of range but QD3D succeeds
 * - QD3D ignores channelCount!
 * - ChannelGetMethod returns kQ3Failure for out-of-range, but QD3D overrides to kQ3Success
 *
 * Oracle recommendation: Driver should keep local copy of channelCount and check range.
 */
TEST_F(ControllerCoreTestFixture, Test_032_ChannelMethods) {
    // Set channel 0 to 150
    whatSet = 150;
    Status3D = Q3Controller_SetChannel(ClientController, 0, &whatSet, sizeof(whatSet));
    EXPECT_Q3_SUCCESS(Status3D);

    // Set channel 1 to 250
    whatSet = 250;
    Status3D = Q3Controller_SetChannel(ClientController, 1, &whatSet, sizeof(whatSet));
    EXPECT_Q3_SUCCESS(Status3D);

    // Get channel 0 - should return 250 (last write wins due to ChannelGetMethod implementation)
    whatGet = 0;
    whatSize = 0;
    whatSize = sizeof(whatGet);
    Status3D = Q3Controller_GetChannel(ClientController, 0, &whatGet, &whatSize);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(250u, whatGet) << "Channel 0 should return 250 (last write)";
    EXPECT_EQ(4u, whatSize) << "whatSize should be 4";

    // Get channel 1 - should return 150
    whatGet = 0;
    whatSize = 0;
    Status3D = Q3Controller_GetChannel(ClientController, 1, &whatGet, &whatSize);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(150u, whatGet) << "Channel 1 should return 150";
    EXPECT_EQ(4u, whatSize) << "whatSize should be 4";

    // Get channel 2 - should return 4 (from ChannelGetMethod switch default)
    whatGet = 0;
    whatSize = 0;
    Status3D = Q3Controller_GetChannel(ClientController, 2, &whatGet, &whatSize);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(4u, whatGet) << "Channel 2 should return 4";
    EXPECT_EQ(4u, whatSize) << "whatSize should be 4";

    // Get channel 3 - should return 1500 (set in test_028)
    whatGet = 0;
    whatSize = 0;
    Status3D = Q3Controller_GetChannel(ClientController, 3, &whatGet, &whatSize);
#if 1
    EXPECT_Q3_SUCCESS(Status3D);
#else
    EXPECT_Q3_FAILURE(Status3D);
#endif
    EXPECT_EQ(1500u, whatGet) << "Channel 3 should return 1500";
    EXPECT_EQ(4u, whatSize) << "whatSize should be 4";

    // Get channel 4 - OUT OF RANGE (ChannelCount==4, channels 0 to 3!)
    // Oracle: "channel 4 is ambitious"
    whatGet = 0;
    whatSize = 0;
    Status3D = Q3Controller_GetChannel(ClientController, 4, &whatGet, &whatSize);
    // channel 4 is ambitious. remember: ChannelCount==4, channels 0 to 3!
#if 1
    EXPECT_Q3_SUCCESS(Status3D)
        << "QD3D succeeds even for out-of-range channel";
    EXPECT_EQ(65000u, whatGet)
        << "Channel 4 should return 65000 (from default case ChannelGetMethod)";
#else
    EXPECT_Q3_FAILURE(Status3D)
        << "Should fail for out-of-range channel";
    EXPECT_EQ(1500u, whatGet)
        << "TBC: same as channel 3?";
#endif
    EXPECT_EQ(4u, whatSize) << "whatSize should be 4";

    /* TBC from oracle:
     + observation:
     - shouldn't be successful, but works!
     - should be kQ3Failure by ChannelGetMethod, but QD3D overrides return value!
     - Method got called!! - QD3D-doc recommends 0<=channel<channelCount but defines no error!
     - TODO: (graceful) pass; with whatSize=4; whatGet wasn't modified

     + Conclusion: QD3D ignores channelCount!

     + Solution: SetChannel and GetChannel are methods to communicate with the device driver.
     When the driver creates a new instance of a TQ3Controller via Q3Controller_New, it should keep
     a local copy of the TQ3ControllerData.ChannelCount value. Later the driver can check against this
     copy if the channel of SetChannel or GetChannel is out of Range and should behave gracefully.
     */
}

/**
 * @brief test_033: Deactivate controller and tracker
 *
 * Oracle behavior: Set both controller and tracker to inactive.
 *
 * IMPORTANT: For all follow-up tests (034-040), controller and tracker are now inactive!
 */
TEST_F(ControllerCoreTestFixture, Test_033_DeactivateControllerAndTracker) {
    // Deactivate controller
    Status3D = Q3Controller_SetActivation(ClientController, kQ3False);
    EXPECT_Q3_SUCCESS(Status3D);

    // Deactivate tracker
    Status3D = Q3Tracker_SetActivation(ClientTracker, kQ3False);
    EXPECT_Q3_SUCCESS(Status3D);

    // Verify controller is deactivated
    Status3D = Q3Controller_GetActivation(ClientController, &controllerActivation);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FALSE(controllerActivation)
        << "controllerActivation should be kQ3False (ambitious behavior)";
}

/**
 * @brief test_034: Check signature a 2nd time (with inactive controller)
 *
 * Oracle behavior: Signature should still be retrievable even when controller is inactive.
 */
TEST_F(ControllerCoreTestFixture, Test_034_GetSignature_Inactive) {
    // Note: controller and tracker are both inactive (from test_033)

    // Get truncated signature
    Status3D = Q3Controller_GetSignature(ClientController, tempString, 5);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_STREQ("com.", tempString)
        << "Truncated signature should be 'com.'";

    // Get full signature
    Status3D = Q3Controller_GetSignature(ClientController, tempString, 128);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_STREQ("com.HMDP.BenchController", tempString)
        << "Full signature should be 'com.HMDP.BenchController'";
}

/**
 * @brief test_035: Get value count and track cursor (with inactive controller)
 *
 * Oracle behavior: These properties should still be queryable when inactive.
 */
TEST_F(ControllerCoreTestFixture, Test_035_GetValueCount_TrackCursor_Inactive) {
    // Note: controller and tracker are both inactive!

    // Get value count
    Status3D = Q3Controller_GetValueCount(ClientController, &tempUns32);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(16u, tempUns32)
        << "ValueCount should still be 16 when inactive";

    // Check 2D cursor tracking
    Status3D = Q3Controller_Track2DCursor(DriverController, &tempBool);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FALSE(tempBool)
        << "Controller should not track 2D cursor";

    // Check 3D cursor tracking
    Status3D = Q3Controller_Track3DCursor(DriverController, &tempBool);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FALSE(tempBool)
        << "Controller should not track 3D cursor";
}

/**
 * @brief test_036: Buttons with inactive controller/tracker
 *
 * Oracle behavior: When controller/tracker are inactive:
 * - SetButtons should NOT trigger TrackerProc callback
 * - GetButtons via Tracker should return 0 (not the set value)
 * - GetButtons via Controller should return last active value (7)
 * - Subsequent GetButtons via Tracker should return 0 (buttons cleared)
 */
TEST_F(ControllerCoreTestFixture, Test_036_Buttons_Inactive) {
    // Note: controller and tracker are both inactive!

    TQ3Uns32 lastTrackerProcCalled = trackerProcCalled;

    // Set buttons - should NOT trigger callback (inactive)
    Status3D = Q3Controller_SetButtons(DriverController, 5);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(lastTrackerProcCalled, trackerProcCalled)
        << "trackerProc should NOT be called - Controller is inactive";

    // Get buttons via Tracker - should return 0 (inactive tracker)
    tempUns32 = 0;
    Status3D = Q3Tracker_GetButtons(ClientTracker, &tempUns32);
    EXPECT_Q3_SUCCESS(Status3D);
#if 1
    EXPECT_EQ(0u, tempUns32)
        << "Buttons via Tracker should be 0 (ambitious - inactive tracker)";
#else
    EXPECT_EQ(7u, tempUns32)
        << "Alternative: Buttons should be 7";
#endif

    // Get buttons via Controller - should return 7 (last active value)
    tempUns32 = 0;
    Status3D = Q3Controller_GetButtons(ClientController, &tempUns32);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(7u, tempUns32)
        << "Buttons via Controller should be 7 (ambitious - last active value)";

    // Get buttons via Tracker again - should be 0 (cleared)
    tempUns32 = 0;
    Status3D = Q3Tracker_GetButtons(ClientTracker, &tempUns32);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(0u, tempUns32)
        << "Buttons not cleared. Buttons were returned the call before, "
        << "then the tracker set buttons bitfield to zero.";
}

/**
 * @brief test_037: Position with inactive tracker
 *
 * Oracle behavior: When tracker is inactive:
 * - GetPosition should return (0,0,0)
 * - TrackerSerNum should NOT be overwritten (remains 0)
 * - SetTrackerPosition should NOT trigger TrackerProc
 * - Position should remain unchanged
 *
 * Oracle note: "Q3Tracker_GetPosition shall not overwrite TrackerSerNum when deactivated"
 */
TEST_F(ControllerCoreTestFixture, Test_037_Position_Inactive) {
    TQ3Uns32 lastTrackerProcCalled;
    // Note: controller and tracker are both inactive!

    // Get initial position - should be (0,0,0) with TrackerSerNum unchanged
    TrackerSerNum = 0;
    Status3D = Q3Tracker_GetPosition(ClientTracker, &position, NULL, NULL, &TrackerSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_POINT3D_EQ(position, 0.0f, 0.0f, 0.0f);
    // Position should be (0,0,0) when inactive
    EXPECT_EQ(0u, TrackerSerNum)
        << "Q3Tracker_GetPosition shall not overwrite TrackerSerNum when deactivated";

    // Set tracker position - should NOT trigger callback
    lastTrackerProcCalled = trackerProcCalled;
    position.x = 1.0f;
    position.y = 1.0f;
    position.z = 1.0f;
    Status3D = Q3Controller_SetTrackerPosition(DriverController, &position);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(lastTrackerProcCalled, trackerProcCalled)
        << "trackerProc should NOT be called - Controller is inactive";

    // Get position again - should still be (0,0,0)
    Status3D = Q3Tracker_GetPosition(ClientTracker, &position, &deltaPos, &changed, &TrackerSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_POINT3D_EQ(position, 0.0f, 0.0f, 0.0f);
    // Position should remain (0,0,0) - set had no effect
    EXPECT_VECTOR3D_EQ(deltaPos, 0.0f, 0.0f, 0.0f);
    // deltaPos should be (0,0,0)
    EXPECT_FALSE(changed)
        << "Tracker should not have changed";
    EXPECT_EQ(0u, TrackerSerNum)
        << "Q3Tracker_GetPosition shall not overwrite TrackerSerNum when deactivated";

    // Get via controller - should also be (0,0,0)
    Status3D = Q3Controller_GetTrackerPosition(ClientController, &position);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_POINT3D_EQ(position, 0.0f, 0.0f, 0.0f);
    // Position via Controller should be (0,0,0)
}

/**
 * @brief test_038: Move position with inactive tracker
 *
 * Oracle behavior: When tracker is inactive:
 * - MoveTrackerPosition should succeed but have no effect
 * - Position should remain (0,0,0)
 * - TrackerSerNum should remain 0
 */
TEST_F(ControllerCoreTestFixture, Test_038_MovePosition_Inactive) {
    // Note: controller and tracker are both inactive!

    // Verify initial position
    Status3D = Q3Tracker_GetPosition(ClientTracker, &position, NULL, NULL, &TrackerSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_POINT3D_EQ(position, 0.0f, 0.0f, 0.0f);
    EXPECT_EQ(0u, TrackerSerNum)
        << "Q3Tracker_GetPosition shall not overwrite TrackerSerNum when deactivated";

    // Move position by (1,1,1)
    deltaPos.x = 1.0f;
    deltaPos.y = 1.0f;
    deltaPos.z = 1.0f;
    Status3D = Q3Controller_MoveTrackerPosition(DriverController, &deltaPos);
    EXPECT_Q3_SUCCESS(Status3D);

    // Get position via controller - should still be (0,0,0)
    Status3D = Q3Controller_GetTrackerPosition(ClientController, &position);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_POINT3D_EQ(position, 0.0f, 0.0f, 0.0f);
    // Position should remain (0,0,0) - move had no effect

    // Get position via tracker - should still be (0,0,0)
    Status3D = Q3Tracker_GetPosition(ClientTracker, &position, &deltaPos, &changed, &TrackerSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_POINT3D_EQ(position, 0.0f, 0.0f, 0.0f);
    EXPECT_VECTOR3D_EQ(deltaPos, 0.0f, 0.0f, 0.0f);
    EXPECT_FALSE(changed)
        << "changed should be kQ3False";
    EXPECT_EQ(0u, TrackerSerNum)
        << "Q3Tracker_GetPosition shall not overwrite TrackerSerNum when deactivated";
}

/**
 * @brief test_039: Orientation with inactive tracker
 *
 * Oracle behavior: When tracker is inactive:
 * - GetOrientation should return identity quaternion (1,0,0,0)
 * - SetTrackerOrientation should succeed but have no effect
 * - MoveTrackerOrientation should succeed but have no effect
 * - TrackerSerNum should remain 0
 */
TEST_F(ControllerCoreTestFixture, Test_039_Orientation_Inactive) {
    // Note: controller and tracker are both inactive!

    // Get initial orientation
    Status3D = Q3Tracker_GetOrientation(ClientTracker, &orientation, NULL, NULL, &TrackerSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_QUATERNION_EQ(orientation, 1.0f, 0.0f, 0.0f, 0.0f);
    // Orientation should be identity (1,0,0,0)
    EXPECT_EQ(0u, TrackerSerNum)
        << "TrackerSerNum should be 0";

    // Set orientation to identity
    Q3Quaternion_SetIdentity(&orientation);
    Status3D = Q3Controller_SetTrackerOrientation(DriverController, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);

    // Get orientation via controller
    Status3D = Q3Controller_GetTrackerOrientation(ClientController, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_QUATERNION_EQ(orientation, 1.0f, 0.0f, 0.0f, 0.0f);
    // Orientation should remain identity

    // Get orientation via tracker with delta
    Status3D = Q3Tracker_GetOrientation(ClientTracker, &orientation, &deltaOri, &changed, &TrackerSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_QUATERNION_EQ(orientation, 1.0f, 0.0f, 0.0f, 0.0f);
    EXPECT_QUATERNION_EQ(deltaOri, 1.0f, 0.0f, 0.0f, 0.0f);
    // deltaOri should be identity
    EXPECT_FALSE(changed)
        << "changed should be kQ3False";
    EXPECT_EQ(0u, TrackerSerNum);

    // Move orientation by 45-degree Y rotation
    Q3Quaternion_SetRotate_Y(&deltaOri, 45); // deltaOri = -0.8733047,0,-0.4871745,0
    Status3D = Q3Controller_MoveTrackerOrientation(DriverController, &deltaOri);
    EXPECT_Q3_SUCCESS(Status3D);

    // Get orientation via controller - should still be identity
    Status3D = Q3Controller_GetTrackerOrientation(ClientController, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_QUATERNION_EQ(orientation, 1.0f, 0.0f, 0.0f, 0.0f);
    // Orientation should remain identity - move had no effect

    // Get orientation via tracker - should still be identity
    Status3D = Q3Tracker_GetOrientation(ClientTracker, &orientation, &deltaOri, &changed, &TrackerSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_QUATERNION_EQ(orientation, 1.0f, 0.0f, 0.0f, 0.0f);
    EXPECT_QUATERNION_EQ(deltaOri, 1.0f, 0.0f, 0.0f, 0.0f);
    // deltaOri should remain identity
    EXPECT_FALSE(changed)
        << "changed should be kQ3False";
    EXPECT_EQ(0u, TrackerSerNum);
}

/**
 * @brief test_040: Controller values with inactive state
 *
 * Oracle behavior: When controller is inactive:
 * - GetValues should return last active values (5.0, -5.0)
 * - SetValues should succeed
 * - GetValues should return new values (3.0, -3.0)
 * - changed flag should be kQ3False (inactive - no change notification)
 * - CtrlSerNum should remain 3 (ambitious - not incremented when inactive)
 */
TEST_F(ControllerCoreTestFixture, Test_040_Values_Inactive) {
    // Get initial values - should be (5.0, -5.0) from test_030
    Status3D = Q3Controller_GetValues(ClientController, 2, values, NULL, &CtrlSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FLOAT_EQ(5.0f, values[0]) << "values[0] should be 5.0";
    EXPECT_FLOAT_EQ(-5.0f, values[1]) << "values[1] should be -5.0";
    EXPECT_EQ(3u, CtrlSerNum) << "CtrlSerNum should be 3";

    // Set new values
    values[0] = 3.0f;
    values[1] = -3.0f;
    Status3D = Q3Controller_SetValues(DriverController, values, 2);
    EXPECT_Q3_SUCCESS(Status3D);

    // Get values with changed flag
    changed = kQ3True;
    Status3D = Q3Controller_GetValues(ClientController, 2, values, &changed, &CtrlSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FLOAT_EQ(3.0f, values[0]) << "values[0] should be 3.0 (overwritten)";
    EXPECT_FLOAT_EQ(-3.0f, values[1]) << "values[1] should be -3.0 (overwritten)";
    EXPECT_FALSE(changed)
        << "changed flag should be kQ3False (overwritten - inactive controller)";
#if 1
    EXPECT_EQ(3u, CtrlSerNum)
        << "CtrlSerNum should remain 3 (ambitious - not incremented when inactive)";
#else
    EXPECT_EQ(4u, CtrlSerNum)
        << "Alternative: CtrlSerNum should be 4 (ambitious)";
#endif
}

/**
 * @brief test_041: Controller decommission
 *
 * Oracle behavior: Test Q3Controller_Decommission comprehensively.
 * When a controller is decommissioned:
 * - Activation should be kQ3False
 * - HasTracker should be kQ3False
 * - Signature should be empty string (QD3D) or "reasonable default value" (Quesa - ambitious)
 * - ValueCount should be 0
 * - Buttons should be 0
 * - Position should be (0,0,0)
 * - Orientation should be identity (1,0,0,0)
 * - Values should NOT be overwritten
 * - CtrlSerNum should be 0 (QD3D) or 4 (Quesa - ambitious)
 *
 * Oracle note: "TODO: what happens to assigned tracker?"
 */
TEST_F(ControllerCoreTestFixture, Test_041_Decommission) {
    // Controller decommissioned

    // Decommission controller
    Status3D = Q3Controller_Decommission(DriverController);
    // TODO: what happens to assigned tracker?
    EXPECT_Q3_SUCCESS(Status3D);

    // Check activation - should be false
    Status3D = Q3Controller_GetActivation(ClientController, &controllerActivation);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FALSE(controllerActivation)
        << "controllerActivation should be kQ3False after decommission";

    // Check tracker - should not have tracker
    Status3D = Q3Controller_HasTracker(DriverController, &tempBool);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FALSE(tempBool)
        << "Controller should not have tracker after decommission";

    // Check signature (truncated) - should be empty
    Status3D = Q3Controller_GetSignature(ClientController, tempString, 5);
    EXPECT_Q3_SUCCESS(Status3D);
#if 1
    EXPECT_EQ(0u, strlen(tempString))
        << "Signature should be empty string after decommission";
#else
    EXPECT_STREQ("com.", tempString)
        << "Ambitious pass: Quesa returns signature even when the controller is "
        << "decommissioned: 'reasonable default value'";
#endif

    // Check signature (full) - should be empty
    Status3D = Q3Controller_GetSignature(ClientController, tempString, 128);
    EXPECT_Q3_SUCCESS(Status3D);
#if 1
    EXPECT_EQ(0u, strlen(tempString))
        << "Full signature should be empty string after decommission";
#else
    EXPECT_STREQ("com.HMDP.BenchController", tempString)
        << "Ambitious pass: Quesa returns signature even when the controller is "
        << "decommissioned: 'reasonable default value'";
#endif

    // Check value count - should be 0
    Status3D = Q3Controller_GetValueCount(ClientController, &tempUns32);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(0u, tempUns32)
        << "ValueCount should be 0 after decommission";

    // Check buttons - should be 0
    Status3D = Q3Tracker_GetButtons(ClientTracker, &tempUns32);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(0u, tempUns32)
        << "Buttons should be 0 after decommission";

    // Check position - should be (0,0,0)
    Status3D = Q3Controller_GetTrackerPosition(ClientController, &position);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_POINT3D_EQ(position, 0.0f, 0.0f, 0.0f);
    // Position should be (0,0,0) after decommission

    // Check orientation - should be identity
    Status3D = Q3Controller_GetTrackerOrientation(ClientController, &orientation);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_QUATERNION_EQ(orientation, 1.0f, 0.0f, 0.0f, 0.0f);
    // Orientation should be identity (1,0,0,0) after decommission

    // Check values - should NOT be overwritten
    CtrlSerNum = 0;
    values[0] = +5.0f;
    values[1] = -5.0f;
    Status3D = Q3Controller_GetValues(ClientController, 2, values, NULL, &CtrlSerNum);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_FLOAT_EQ(+5.0f, values[0])
        << "values[0] should NOT be overwritten (should remain +5)";
    EXPECT_FLOAT_EQ(-5.0f, values[1])
        << "values[1] should NOT be overwritten (should remain -5)";
#if 1
    EXPECT_EQ(0u, CtrlSerNum)
        << "CtrlSerNum should be 0 after decommission";
#else
    EXPECT_EQ(4u, CtrlSerNum)
        << "Ambitious: CtrlSerNum should be 4";
#endif
}

/**
 * @brief test_042: Re-create controller after decommission
 *
 * Oracle behavior: After decommissioning, should be able to create a new controller.
 * Q3Controller_Next should find the new controller.
 */
TEST_F(ControllerCoreTestFixture, Test_042_RecreateController) {
    // Controller re-new
    DriverController = Q3Controller_New(&ControllerData);
    EXPECT_NE(nullptr, DriverController)
        << "DriverController should not be NULL after re-creation";

    // Check that controller is available
    Status3D = Q3Controller_Next(NULL, &tempController);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_NE(nullptr, tempController)
        << "Controller should be available after re-creation";
}

/**
 * @brief test_043: Dispose tracker and controller state
 *
 * Oracle behavior: Clean up resources.
 * Dispose tracker and controller state objects.
 *
 * Oracle notes:
 * - "done: check/review of Tracker Dispose"
 * - "done: check/review of OS X Quesa cleanup"
 */
TEST_F(ControllerCoreTestFixture, Test_043_DisposeResources) {
    // Dispose tracker
    Q3Object_Dispose(ClientTracker);
    ClientTracker = nullptr;
    // done: check/review of Tracker Dispose

    // Dispose controller state
    Q3Object_Dispose(ControllerState);
    ControllerState = nullptr;
    // done: check/review of OS X Quesa cleanup

    // No assertions - just cleanup
    SUCCEED() << "Resources disposed successfully";
}

// ============================================================================
// Repeated Tests (from original oracle)
// The oracle runs test_006, test_018, and test_019 again after test_042
// ============================================================================

/**
 * @brief Repeat test_006: Controller enumeration (2nd time)
 *
 * Oracle behavior: Same as test_006, verifying controller enumeration still works
 * after decommission and re-creation.
 */
TEST_F(ControllerCoreTestFixture, Test_044_Rerun_006_ControllerNext) {
    Status3D = Q3Controller_Next(NULL, &ClientController);

    EXPECT_Q3_SUCCESS(Status3D);

    EXPECT_PLATFORM_CONTROLLER_EQUALITY(ClientController, DriverController);
}

/**
 * @brief Repeat test_018: Get controller signature (2nd time)
 *
 * Oracle behavior: Same as test_018, verifying signature retrieval after
 * decommission and re-creation.
 */
TEST_F(ControllerCoreTestFixture, Test_045_Rerun_018_GetSignature) {
    // Test with small buffer (5 bytes)
    Status3D = Q3Controller_GetSignature(ClientController, tempString, 5);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_STREQ("com.", tempString)
        << "Signature with buffer size 5 should return 'com.'";

    // Test with full buffer (128 bytes)
    Status3D = Q3Controller_GetSignature(ClientController, tempString, 128);
    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_STREQ("com.HMDP.BenchController", tempString)
        << "Full signature should be 'com.HMDP.BenchController'";
}

/**
 * @brief Repeat test_019: Get controller value count (2nd time)
 *
 * Oracle behavior: Same as test_019, verifying value count retrieval after
 * decommission and re-creation.
 */
TEST_F(ControllerCoreTestFixture, Test_046_Rerun_019_GetValueCount) {
    Status3D = Q3Controller_GetValueCount(ClientController, &tempUns32);

    EXPECT_Q3_SUCCESS(Status3D);
    EXPECT_EQ(16u, tempUns32)
        << "ValueCount should be 16 (as set in controller re-creation)";
}

/**
 * @brief Final test: Repeat test_043 (final cleanup)
 *
 * Oracle behavior: The original oracle ends with test_043 being called again.
 * This is the final cleanup.
 */
TEST_F(ControllerCoreTestFixture, Test_047_Final_Cleanup) {
    // Note: In the original oracle, test_043 is called again at the very end
    // This appears to be a final cleanup step

    // The fixture's TearDownTestSuite will handle final cleanup
    SUCCEED() << "All tests completed - final cleanup will be handled by fixture";
}

// ============================================================================
// END OF TESTS
// All 47 test runs complete (44 unique tests + 3 repeats)
// ============================================================================

