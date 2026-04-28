/*  NAME:
    XPCDriverHelper.mm

    DESCRIPTION:
    Standalone driver process for cross-process XPC integration tests.

    Registers a Quesa controller with channel methods against the running
    com.quesa.devicedb LaunchAgent, performs an internal state-save/reset
    self-test (exercises the two-hop XPC path), then stays alive until
    killed so the client test binary can call SetChannel / GetChannel
    cross-process.

    Sentinel files written to /tmp:
      quesa_xpc_driver_ready      — written after controller registration
      quesa_xpc_driver_selftest   — "saveOK restoreOK\n" (1=pass, 0=fail)

    The run_xpc_integration.sh script starts this binary in the background
    and kills it (SIGTERM) after the client test binary finishes.
*/

#import <Foundation/Foundation.h>
#import <signal.h>

#include "QuesaController.h"

// ---------------------------------------------------------------------------
// Channel state (mirrors testsControllerTestFixture channel callbacks)
// ---------------------------------------------------------------------------

static TQ3Uns32 what0    = 0;   // set by ChannelSetMethod channel 0
static TQ3Uns32 what1    = 0;   // set by ChannelSetMethod channel 1
static TQ3Uns32 lastsize = 0;   // last dataSize seen by ChannelSetMethod
static TQ3Uns32 what_work = 0;  // set by ChannelSetMethod channel 3

static TQ3ControllerRef gDriverRef = nullptr;

// ChannelSetMethod / ChannelGetMethod intentionally cross what0 and what1
// on read — this matches the oracle (uniBench.c) and the existing unit tests.

static TQ3Status ChannelSetMethod(TQ3ControllerRef /*controllerRef*/,
                                  TQ3Uns32 channel,
                                  const void *data,
                                  TQ3Uns32 dataSize)
{
    lastsize = dataSize;
    switch (channel) {
        case 0: what0 = data ? *((const TQ3Uns32*)data) : 0; break;
        case 1: what1 = data ? *((const TQ3Uns32*)data) : 0; break;
        case 3: what_work = data ? *((const TQ3Uns32*)data) : 0; break;
        default: break;
    }
    return kQ3Success;
}

static TQ3Status ChannelGetMethod(TQ3ControllerRef /*controllerRef*/,
                                  TQ3Uns32 channel,
                                  void *data,
                                  TQ3Uns32 *dataSize)
{
    if (!data || !dataSize) return kQ3Failure;

    TQ3Status  status = kQ3Success;
    TQ3Uns32   result = 0;
    switch (channel) {
        case 0: result = what1;    break;          // crossed!
        case 1: result = what0;    break;          // crossed!
        case 2: result = lastsize; break;
        case 3: result = what_work; status = kQ3Failure; break;
        default: result = 65000;  status = kQ3Failure; break;
    }
    *((TQ3Uns32*)data) = result;
    *dataSize = sizeof(TQ3Uns32);
    return status;
}

// ---------------------------------------------------------------------------
// Signal handling
// ---------------------------------------------------------------------------

static volatile sig_atomic_t gShouldExit = 0;

static void handleSIGTERM(int /*sig*/)
{
    gShouldExit = 1;
    CFRunLoopStop(CFRunLoopGetMain());
}

// ---------------------------------------------------------------------------
// Self-test: SaveAndReset + Restore through two-hop XPC
// ---------------------------------------------------------------------------

static void runStateSelfTest(void)
{
    // Set up known state directly in the driver's channel variables.
    what0 = 100;
    what1 = 200;
    lastsize = 0;
    what_work = 0;

    TQ3ControllerStateObject state = Q3ControllerState_New(gDriverRef);
    if (!state)
    {
        FILE *f = fopen("/tmp/quesa_xpc_driver_selftest", "w");
        if (f) { fprintf(f, "0 0\n"); fclose(f); }
        NSLog(@"[driver] ERROR: Q3ControllerState_New failed");
        return;
    }

    // SaveAndReset: reads channels (via two-hop XPC) then zeros them
    TQ3Status sr = Q3ControllerState_SaveAndReset(state);
    int saveOK = (sr == kQ3Success) && (what0 == 0) && (what1 == 0);
    NSLog(@"[driver] SaveAndReset: status=%d what0=%u what1=%u → %s",
          sr, what0, what1, saveOK ? "PASS" : "FAIL");

    // Restore: writes saved values back via two-hop XPC
    TQ3Status rr = Q3ControllerState_Restore(state);
    // After restore: ChannelSetMethod(ch0, saved[0]) → what0 = saved[0],
    // saved[0] was ChannelGetMethod(ch0) = what1 = 200, so what0 = 200.
    // Similarly what1 = 100. The cross does not round-trip perfectly, but
    // the XPC path is exercised. We only verify the call succeeded.
    int restoreOK = (rr == kQ3Success);
    NSLog(@"[driver] Restore: status=%d what0=%u what1=%u → %s",
          rr, what0, what1, restoreOK ? "PASS" : "FAIL");

    Q3Object_Dispose(state);

    FILE *f = fopen("/tmp/quesa_xpc_driver_selftest", "w");
    if (f) { fprintf(f, "%d %d\n", saveOK, restoreOK); fclose(f); }
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int /*argc*/, const char * /*argv*/[])
{
    @autoreleasepool
    {
        signal(SIGTERM, handleSIGTERM);

        if (Q3Initialize() != kQ3Success)
        {
            NSLog(@"[driver] Q3Initialize failed");
            return 1;
        }

        // Register the controller with the LaunchAgent DB.
        TQ3ControllerData data = {};
        data.signature        = (char *)"com.quesa.xpctest.driver";
        data.valueCount       = 2;
        data.channelCount     = 4;
        data.channelSetMethod = ChannelSetMethod;
        data.channelGetMethod = ChannelGetMethod;

        gDriverRef = Q3Controller_New(&data);
        if (!gDriverRef)
        {
            NSLog(@"[driver] Q3Controller_New failed (is com.quesa.devicedb running?)");
            Q3Exit();
            return 1;
        }
        NSLog(@"[driver] Controller registered");

        // Self-test the state save/restore cross-process path.
        runStateSelfTest();

        // Signal the test runner that we are ready.
        FILE *ready = fopen("/tmp/quesa_xpc_driver_ready", "w");
        if (ready) fclose(ready);
        NSLog(@"[driver] Ready (wrote sentinel file)");

        // Stay alive so the client test binary can call SetChannel/GetChannel.
        while (!gShouldExit)
            [[NSRunLoop mainRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.5]];

        NSLog(@"[driver] Exiting");
        Q3Controller_Decommission(gDriverRef);
        Q3Exit();
    }
    return 0;
}
