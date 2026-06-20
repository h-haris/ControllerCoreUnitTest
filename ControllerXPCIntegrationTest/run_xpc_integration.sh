#!/bin/bash
# run_xpc_integration.sh
#
# Orchestrates the cross-process XPC integration test:
#   1. Loads com.quesa.devicedb LaunchAgent if not already running.
#   2. Starts quesa_xpc_driver in the background.
#   3. Waits up to 10 s for the driver to write /tmp/quesa_xpc_driver_ready.
#   4. Runs controller_xpc_integration_gtest (the client test binary).
#   5. Kills the driver.
#   6. Unloads the LaunchAgent if WE loaded it (non-destructive).
#   7. Exits with the test binary's exit code.
#
# Prerequisites — one-time build of the LaunchAgent:
#   cd ../QuesaDeviceDB && cmake -B build && cmake --build build
#
# The plist must be installed in ~/Library/LaunchAgents/:
#   ../QuesaDeviceDB/install_agent.sh
# (This script will load/unload it for the test run only.)
#
# NOTE: The in-process unit tests (controller_gtest) must NOT have the
# LaunchAgent running — they use an in-process DB. This script unloads the
# agent after the cross-process tests to leave the system clean.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

DRIVER="$BUILD_DIR/quesa_xpc_driver"
CLIENT="$BUILD_DIR/controller_xpc_integration_gtest"

READY_FILE="/tmp/quesa_xpc_driver_ready"
SELFTEST_FILE="/tmp/quesa_xpc_driver_selftest"

LABEL="com.quesa.devicedb"
PLIST="$HOME/Library/LaunchAgents/${LABEL}.plist"
GUI_DOMAIN="gui/$(id -u)"

AGENT_LOADED_BY_US=false

# ---- check binaries ----
if [[ ! -x "$DRIVER" ]]; then
    echo "Error: $DRIVER not found."
    echo "Run:  cmake -B build && cmake --build build"
    exit 1
fi
if [[ ! -x "$CLIENT" ]]; then
    echo "Error: $CLIENT not found."
    echo "Run:  cmake -B build && cmake --build build"
    exit 1
fi

# ---- ensure LaunchAgent plist is installed ----
if [[ ! -f "$PLIST" ]]; then
    echo "Error: $PLIST not found."
    echo "Install the LaunchAgent first:"
    echo "  cd ../QuesaDeviceDB && cmake -B build && cmake --build build"
    echo "  ./install_agent.sh"
    exit 1
fi

# ---- load LaunchAgent if not already running ----
if launchctl print "${GUI_DOMAIN}/${LABEL}" &>/dev/null; then
    echo "LaunchAgent ${LABEL} already loaded — using it."
else
    echo "Loading LaunchAgent ${LABEL}..."
    launchctl bootstrap "${GUI_DOMAIN}" "$PLIST"
    AGENT_LOADED_BY_US=true
    sleep 0.3   # give launchd a moment to register the Mach service
fi

cleanup() {
    # Kill the driver if still running.
    kill "$DRIVER_PID" 2>/dev/null || true
    wait "$DRIVER_PID" 2>/dev/null || true

    # Remove sentinel files.
    rm -f "$READY_FILE" "$SELFTEST_FILE"

    # Unload the agent only if this script loaded it, to leave the system clean.
    # (The in-process unit tests expect NO LaunchAgent running.)
    if [[ "$AGENT_LOADED_BY_US" == true ]]; then
        echo "Unloading LaunchAgent ${LABEL} (loaded by this script)..."
        launchctl bootout "${GUI_DOMAIN}/${LABEL}" 2>/dev/null || true
    fi
}

# ---- clean up leftover sentinel files ----
rm -f "$READY_FILE" "$SELFTEST_FILE"

# ---- start driver in background ----
"$DRIVER" &
DRIVER_PID=$!
echo "Started quesa_xpc_driver (pid $DRIVER_PID)"

# Register cleanup to run on exit.
trap cleanup EXIT

# ---- wait for ready sentinel (max 10 s) ----
WAITED=0
while [[ ! -f "$READY_FILE" ]]; do
    sleep 0.1
    WAITED=$((WAITED + 1))
    if [[ $WAITED -ge 100 ]]; then
        echo "Error: Driver did not write $READY_FILE within 10 s"
        exit 1
    fi
done
echo "Driver ready."

# ---- run client tests ----
"$CLIENT" "$@"
TEST_EXIT=$?

echo ""
if [[ $TEST_EXIT -eq 0 ]]; then
    echo "All cross-process XPC integration tests PASSED."
else
    echo "Some cross-process XPC integration tests FAILED (exit $TEST_EXIT)."
fi

exit $TEST_EXIT
