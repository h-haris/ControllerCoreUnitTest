// testsControllerTestFixture.cpp
// Test fixture implementation for Controller Core Unit Tests
// Migrated from MinUnit to Google Test
// Oracle: QD3D 1.6 MacOS Classic

#include "testsControllerTestFixture.h"

// ============================================================================
// Static Member Initialization
// ============================================================================

// Controller state
TQ3Boolean ControllerCoreTestFixture::controllerListChanged = kQ3False;
TQ3Uns32 ControllerCoreTestFixture::controllerListSerialNumber = 0;
TQ3ControllerData ControllerCoreTestFixture::ControllerData = {};
TQ3ControllerStateObject ControllerCoreTestFixture::ControllerState = nullptr;
TQ3Status ControllerCoreTestFixture::Status3D = kQ3Failure;
TQ3ControllerRef ControllerCoreTestFixture::tempController = nullptr;
TQ3ControllerRef ControllerCoreTestFixture::ClientController = nullptr;
TQ3ControllerRef ControllerCoreTestFixture::DriverController = nullptr;
TQ3Boolean ControllerCoreTestFixture::controllerActivation = kQ3False;

// Tracker state
TQ3TrackerObject ControllerCoreTestFixture::ClientTracker = nullptr;
TQ3Boolean ControllerCoreTestFixture::trackerActivation = kQ3False;
float ControllerCoreTestFixture::posTresh = 0.0f;
float ControllerCoreTestFixture::oriTresh = 0.0f;

// Working variables
char ControllerCoreTestFixture::tempString[250] = {0};
TQ3Uns32 ControllerCoreTestFixture::tempUns32 = 0;
TQ3Uns32 ControllerCoreTestFixture::TrackerSerNum = 0;
TQ3Uns32 ControllerCoreTestFixture::CtrlSerNum = 0;
TQ3Boolean ControllerCoreTestFixture::tempBool = kQ3False;
TQ3Boolean ControllerCoreTestFixture::changed = kQ3False;
TQ3Point3D ControllerCoreTestFixture::position = {0.0f, 0.0f, 0.0f};
TQ3Vector3D ControllerCoreTestFixture::deltaPos = {0.0f, 0.0f, 0.0f};
TQ3Quaternion ControllerCoreTestFixture::orientation = {0.0f, 0.0f, 0.0f, 1.0f};
TQ3Quaternion ControllerCoreTestFixture::deltaOri = {0.0f, 0.0f, 0.0f, 1.0f};
float ControllerCoreTestFixture::values[2] = {0.0f, 0.0f};
TQ3Uns32 ControllerCoreTestFixture::whatSet = 0;
TQ3Uns32 ControllerCoreTestFixture::whatGet = 0;
TQ3Uns32 ControllerCoreTestFixture::whatSize = 0;

// Callback tracking
TQ3Uns32 ControllerCoreTestFixture::lastsize = 0;
TQ3Uns32 ControllerCoreTestFixture::what0 = 0;
TQ3Uns32 ControllerCoreTestFixture::what1 = 0;
TQ3Uns32 ControllerCoreTestFixture::what_work = 0;
TQ3Uns32 ControllerCoreTestFixture::trackerProcCalled = 0;
TQ3Uns32 ControllerCoreTestFixture::doBeep = 0;

// ============================================================================
// Setup and Teardown
// ============================================================================

void ControllerCoreTestFixture::SetUpTestSuite() {
    // Initialize all static members to default values
    controllerListChanged = kQ3False;
    controllerListSerialNumber = 0;
    std::memset(&ControllerData, 0, sizeof(ControllerData));
    ControllerState = nullptr;
    Status3D = kQ3Failure;
    tempController = nullptr;
    ClientController = nullptr;
    DriverController = nullptr;
    controllerActivation = kQ3False;
    
    ClientTracker = nullptr;
    trackerActivation = kQ3False;
    posTresh = 0.0f;
    oriTresh = 0.0f;
    
    std::memset(tempString, 0, sizeof(tempString));
    tempUns32 = 0;
    TrackerSerNum = 0;
    CtrlSerNum = 0;
    tempBool = kQ3False;
    changed = kQ3False;
    
    position = {0.0f, 0.0f, 0.0f};
    deltaPos = {0.0f, 0.0f, 0.0f};
    orientation = {0.0f, 0.0f, 0.0f, 1.0f};
    deltaOri = {0.0f, 0.0f, 0.0f, 1.0f};
    
    values[0] = 0.0f;
    values[1] = 0.0f;
    
    whatSet = 0;
    whatGet = 0;
    whatSize = 0;
    
    lastsize = 0;
    what0 = 0;
    what1 = 0;
    what_work = 0;
    trackerProcCalled = 0;
    doBeep = 0;
}

void ControllerCoreTestFixture::TearDownTestSuite() {
    // Cleanup any remaining resources
    if (ControllerState != nullptr) {
        Q3Object_Dispose(ControllerState);
        ControllerState = nullptr;
    }
    
    if (ClientTracker != nullptr) {
        Q3Object_Dispose(ClientTracker);
        ClientTracker = nullptr;
    }
    
    // Note: Controllers are references, not owned objects
    // They should not be disposed here
    ClientController = nullptr;
    DriverController = nullptr;
    tempController = nullptr;
}

// ============================================================================
// Callback Methods
// ============================================================================

TQ3Status ControllerCoreTestFixture::ChannelGetMethod(
    TQ3ControllerRef controllerRef,
    TQ3Uns32 channel,
    void *data,
    TQ3Uns32 *dataSize)
{
    (void)controllerRef; // Unused parameter
    
    // Track which channel was accessed
    what_work = channel;
    
    // Validate parameters
    if (data == nullptr || dataSize == nullptr) {
        return kQ3Failure;
    }
    
    // Handle different channel types
    switch (channel) {
        case 0: // whatGet channel
            if (*dataSize >= sizeof(TQ3Uns32)) {
                *((TQ3Uns32*)data) = whatGet;
                *dataSize = sizeof(TQ3Uns32);
                return kQ3Success;
            }
            break;
            
        case 1: // whatSize channel
            if (*dataSize >= sizeof(TQ3Uns32)) {
                *((TQ3Uns32*)data) = whatSize;
                *dataSize = sizeof(TQ3Uns32);
                return kQ3Success;
            }
            break;
            
        default:
            // Unknown channel
            return kQ3Failure;
    }
    
    return kQ3Failure;
}

TQ3Status ControllerCoreTestFixture::ChannelSetMethod(
    TQ3ControllerRef controllerRef,
    TQ3Uns32 channel,
    const void *data,
    TQ3Uns32 dataSize)
{
    (void)controllerRef; // Unused parameter
    
    // Track which channel was accessed
    what_work = channel;
    lastsize = dataSize;
    
    // Validate parameters
    if (data == nullptr) {
        return kQ3Failure;
    }
    
    // Handle different channel types
    switch (channel) {
        case 0: // whatSet channel
            if (dataSize >= sizeof(TQ3Uns32)) {
                whatSet = *((const TQ3Uns32*)data);
                what0 = whatSet;
                return kQ3Success;
            }
            break;
            
        case 1: // Additional channel
            if (dataSize >= sizeof(TQ3Uns32)) {
                what1 = *((const TQ3Uns32*)data);
                return kQ3Success;
            }
            break;
            
        default:
            // Unknown channel
            return kQ3Failure;
    }
    
    return kQ3Failure;
}

TQ3Status ControllerCoreTestFixture::TrackerProc(
    TQ3TrackerObject trackerObject,
    TQ3ControllerRef controllerRef)
{
    (void)trackerObject;  // Unused parameter
    (void)controllerRef;  // Unused parameter
    
    // Increment call counter
    trackerProcCalled++;
    
    // Simple tracker behavior - just track that we were called
    // In a real implementation, this would update tracker state
    // based on controller input
    
    return kQ3Success;
}
