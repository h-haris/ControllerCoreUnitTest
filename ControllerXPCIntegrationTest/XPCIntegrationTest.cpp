/*  NAME:
    XPCIntegrationTest.cpp

    DESCRIPTION:
    Cross-process XPC integration tests for the Quesa controller API.

    Requires:
      1. com.quesa.devicedb LaunchAgent registered and running.
      2. quesa_xpc_driver helper binary running in the background
         (started by run_xpc_integration.sh before this binary is invoked).

    Tests verify that Q3Controller_SetChannel, GetChannel, and
    Q3ControllerState_SaveAndReset/Restore work correctly across process
    boundaries via the two-hop XPC path:
        client → LaunchAgent (Q3DcontrollerXPC) → driver (Q3DriverStateXPC)
*/

#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>
#include <unistd.h>

#include "QuesaController.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static bool waitForFile(const char *path, int maxTenths)
{
    for (int i = 0; i < maxTenths; ++i)
    {
        if (access(path, F_OK) == 0) return true;
        usleep(100000); // 100 ms
    }
    return false;
}

static TQ3ControllerRef findControllerBySignature(const char *sig)
{
    TQ3ControllerRef ref = nullptr;
    TQ3ControllerRef next = nullptr;
    char buf[256];

    while (Q3Controller_Next(ref, &next) == kQ3Success && next != nullptr)
    {
        ref = next;
        buf[0] = '\0';
        Q3Controller_GetSignature(ref, buf, sizeof(buf));
        if (strcmp(buf, sig) == 0)
            return ref;
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------

class XPCIntegrationTestFixture : public ::testing::Test
{
protected:
    static TQ3ControllerRef DriverRef;

    static void SetUpTestSuite()
    {
        ASSERT_EQ(Q3Initialize(), kQ3Success) << "Q3Initialize failed";

        // Wait for the driver helper to become ready (up to 5 s).
        bool ready = waitForFile("/tmp/quesa_xpc_driver_ready", 50);
        ASSERT_TRUE(ready)
            << "Driver helper did not write /tmp/quesa_xpc_driver_ready within 5 s.\n"
               "Start it with: ./build/quesa_xpc_driver &";

        // Locate the cross-process controller by signature.
        DriverRef = findControllerBySignature("com.quesa.xpctest.driver");
        ASSERT_NE(DriverRef, nullptr)
            << "Could not find controller with signature 'com.quesa.xpctest.driver'.\n"
               "Ensure com.quesa.devicedb LaunchAgent is running and the driver is registered.";
    }

    static void TearDownTestSuite()
    {
        Q3Exit();
    }
};

TQ3ControllerRef XPCIntegrationTestFixture::DriverRef = nullptr;

// ---------------------------------------------------------------------------
// Test_XPC_001 — controller is visible cross-process
// ---------------------------------------------------------------------------

TEST_F(XPCIntegrationTestFixture, Test_XPC_001_FindController)
{
    ASSERT_NE(DriverRef, nullptr);

    char sig[256] = {};
    TQ3Status st = Q3Controller_GetSignature(DriverRef, sig, sizeof(sig));
    EXPECT_EQ(st, kQ3Success);
    EXPECT_STREQ(sig, "com.quesa.xpctest.driver");

    TQ3Uns32 valueCount = 0;
    st = Q3Controller_GetValueCount(DriverRef, &valueCount);
    EXPECT_EQ(st, kQ3Success);
    EXPECT_EQ(valueCount, 2u);
}

// ---------------------------------------------------------------------------
// Test_XPC_002 — SetChannel cross-process (channel 0 ← 150)
// ---------------------------------------------------------------------------

TEST_F(XPCIntegrationTestFixture, Test_XPC_002_SetChannel_Ch0)
{
    TQ3Uns32 val = 150;
    TQ3Status st = Q3Controller_SetChannel(DriverRef, 0, &val, sizeof(val));
    EXPECT_EQ(st, kQ3Success)
        << "Q3Controller_SetChannel(ch0=150) failed cross-process";
}

// ---------------------------------------------------------------------------
// Test_XPC_003 — SetChannel cross-process (channel 1 ← 250)
// ---------------------------------------------------------------------------

TEST_F(XPCIntegrationTestFixture, Test_XPC_003_SetChannel_Ch1)
{
    TQ3Uns32 val = 250;
    TQ3Status st = Q3Controller_SetChannel(DriverRef, 1, &val, sizeof(val));
    EXPECT_EQ(st, kQ3Success)
        << "Q3Controller_SetChannel(ch1=250) failed cross-process";
}

// ---------------------------------------------------------------------------
// Test_XPC_004 — GetChannel cross-process verifies crossed read-back
//
// ChannelSetMethod stores:  ch0 → what0=150,  ch1 → what1=250
// ChannelGetMethod crosses: ch0 → what1=250,  ch1 → what0=150
//
// This mirrors the in-process oracle behaviour verified by Test_032.
// ---------------------------------------------------------------------------

TEST_F(XPCIntegrationTestFixture, Test_XPC_004_GetChannel_CrossedReadback)
{
    TQ3Uns32 val  = 0;
    TQ3Uns32 size = sizeof(val);

    // GetChannel(0) should return what1 = 250 (crossed).
    TQ3Status st = Q3Controller_GetChannel(DriverRef, 0, &val, &size);
    EXPECT_EQ(st, kQ3Success);
    EXPECT_EQ(val,  250u) << "GetChannel(0) should return what1=250";
    EXPECT_EQ(size, 4u);

    // GetChannel(1) should return what0 = 150 (crossed).
    val  = 0;
    size = sizeof(val);
    st = Q3Controller_GetChannel(DriverRef, 1, &val, &size);
    EXPECT_EQ(st, kQ3Success);
    EXPECT_EQ(val,  150u) << "GetChannel(1) should return what0=150";
    EXPECT_EQ(size, 4u);
}

// ---------------------------------------------------------------------------
// Test_XPC_005 — SaveAndReset cross-process path (driver self-test result)
//
// The driver helper calls Q3ControllerState_SaveAndReset on its own
// controller at startup. That call goes through the two-hop XPC path:
//   driver → LaunchAgent → driver's Q3DriverStateXPC → ChannelGetMethod/SetMethod
//
// The driver writes the pass/fail outcome to /tmp/quesa_xpc_driver_selftest.
// We read and assert it here.
// ---------------------------------------------------------------------------

TEST_F(XPCIntegrationTestFixture, Test_XPC_005_ControllerState_SaveAndReset)
{
    FILE *f = fopen("/tmp/quesa_xpc_driver_selftest", "r");
    ASSERT_NE(f, nullptr)
        << "/tmp/quesa_xpc_driver_selftest not found — driver self-test did not run";
    int saveOK = 0, restoreOK = 0;
    int n = fscanf(f, "%d %d", &saveOK, &restoreOK);
    fclose(f);
    ASSERT_EQ(n, 2) << "Could not parse driver self-test results";
    EXPECT_EQ(saveOK, 1)
        << "Driver Q3ControllerState_SaveAndReset cross-process failed.\n"
           "Check /tmp/com.quesa.devicedb.log for details.";
}

// ---------------------------------------------------------------------------
// Test_XPC_006 — Restore cross-process path (driver self-test result)
// ---------------------------------------------------------------------------

TEST_F(XPCIntegrationTestFixture, Test_XPC_006_ControllerState_Restore)
{
    FILE *f = fopen("/tmp/quesa_xpc_driver_selftest", "r");
    ASSERT_NE(f, nullptr)
        << "/tmp/quesa_xpc_driver_selftest not found — driver self-test did not run";
    int saveOK = 0, restoreOK = 0;
    int n = fscanf(f, "%d %d", &saveOK, &restoreOK);
    fclose(f);
    ASSERT_EQ(n, 2) << "Could not parse driver self-test results";
    EXPECT_EQ(restoreOK, 1)
        << "Driver Q3ControllerState_Restore cross-process failed.\n"
           "Check /tmp/com.quesa.devicedb.log for details.";
}
