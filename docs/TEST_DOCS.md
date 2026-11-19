# Test Documentation

This document describes the test scenarios used to verify the `watchdog` process monitoring system.

## Test Scenarios
1. **Normal Operation:** Start `watchdog` and verify that at least two child processes are created and tracked.
2. **Normal Termination:** A child process exits normally (`exit(0)`) and is not restarted.
3. **Unexpected Termination:** A child process is killed with `SIGKILL` and automatically restarted by `watchdog`.
4. **Restart Limit:** After 3 unexpected terminations, the worker is no longer restarted.
5. **Status Reporting (`SIGUSR1`):** Sending `SIGUSR1` writes the PID, state, and restart count of each child to `watchdog.log`.
6. **Graceful Shutdown (`SIGTERM` & `SIGINT`):** Sending `SIGTERM` or pressing `Ctrl+C` stops new launches, terminates children cleanly, and leaves no zombie processes.
7. **Log Verification:** Confirm that all key events are recorded in `watchdog.log` with valid timestamps.