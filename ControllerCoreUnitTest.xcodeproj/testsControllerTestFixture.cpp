// ControllerTestFixture.cpp
// Implementation of test fixture for Controller Core Unit Tests
// Migrated from MinUnit to Google Test

#include "ControllerTestFixture.h"
#include <cstdio>
#include <cstdarg>

#ifndef BenchX
#include <Sound.h>
#endif

// ============================================================================
// Static member initialization
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
TQ3Quaternion ControllerCoreTestFixture::orientation = {1.0f, 0.0f, 0.0f, 0.0f};
TQ3Quaternion ControllerCoreTestFixture::deltaOri = {1.0f, 0.0f, 0.0f, 0.0f};
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
// SetUp and TearDown
// ============================================================================

void ControllerCoreTestFixture::SetUpTestSuite() {
    // Initialize globals - this mimics the initialization in do_bench()
    controllerListSerialNumber = 0;
    trackerProcCalled = 0;
    doBeep = 0;
    tempController = nullptr;
    
    std::cout << "=== Controller Core Test Suite Started ===" << std::endl;
    std::cout << "Oracle: QD3D 1.6 MacOS Classic" << std::endl;
    std::cout << "Framework: Google Test" << std::endl;
}

void ControllerCoreTestFixture::TearDownTestSuite() {
    std::cout << "=== Controller Core Test Suite Completed ===" << std::endl;
    std::cout << "TrackerProc called " << trackerProcCalled << " times" << std::endl;
}

// ============================================================================
// Controller Callback Methods
// ============================================================================

TQ3Status ControllerCoreTestFixture::ChannelGetMethod(
    TQ3ControllerRef controllerRef,
    TQ3Uns32 channel,
    void *data,
    TQ3Uns32 *dataSize)
{
    TQ3Status status = kQ3Success;
    
    // Note: what0 and what1 are crossed!!
    switch(channel) {
        case 0:
            *(TQ3Uns32*)data = what1;
            break;
        case 1:
            *(TQ3Uns32*)data = what0;
            break;
        case 2:
            *(TQ3Uns32*)data = lastsize;
            break;
        case 3:
            *(TQ3Uns32*)data = what_work;
            status = kQ3Failure;
            break;
        default:
            *(TQ3Uns32*)data = 65000;
            status = kQ3Failure;
            break;
    }
    
    *dataSize = sizeof(TQ3Uns32);
    
    return status;
}

TQ3Status ControllerCoreTestFixture::ChannelSetMethod(
    TQ3ControllerRef controllerRef,
    TQ3Uns32 channel,
    const void *data,
    TQ3Uns32 dataSize)
{
    // Store channel data
    switch(channel) {
        case 0:
            if (data == nullptr)
                what0 = (TQ3Uns32)0;
            else
                what0 = *(TQ3Uns32*)data;
            break;
        case 1:
            if (data == nullptr)
                what1 = (TQ3Uns32)0;
            else
                what1 = *(TQ3Uns32*)data;
            break;
        case 3:
            if (data == nullptr)
                what_work = (TQ3Uns32)0;
            else
                what_work = *(TQ3Uns32*)data;
            break;
    }
    
    lastsize = dataSize;
    
    return kQ3Success; // kQ3Failure not implemented
}

TQ3Status ControllerCoreTestFixture::TrackerProc(
    TQ3TrackerObject trackerObject,
    TQ3ControllerRef controllerRef)
{
    // Track callback invocations
#if 1
    doBeep++;
#else
    #ifdef BenchX
        // NSBeep() not available in C++ without Objective-C
        std::cout << '\a'; // System beep
    #else
        SysBeep(100);
    #endif
#endif
    
    trackerProcCalled++;
    
    return kQ3Success;
}
