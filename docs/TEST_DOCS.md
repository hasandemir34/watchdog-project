# Test Documentation

This document describes the test scenarios, commands, and verification criteria for the `watchdog` process monitoring system.

## Test Scenarios
1. **Normal Operation:** Start `watchdog` and verify that at least two child processes are created and tracked.
2. **Normal Termination:** A child process exits normally (`exit(0)`) and is not restarted.
3. **Unexpected Termination:** A child process is terminated unexpectedly (`SIGKILL`) and automatically restarted by `watchdog`.
4. **Restart Limit:** After 3 unexpected terminations, the worker is no longer restarted.
5. **Status Reporting (`SIGUSR1`):** Sending `SIGUSR1` writes the PID, state, and restart count of each child to `watchdog.log`.
6. **Graceful Shutdown (`SIGTERM` & `SIGINT`):** Sending `SIGTERM` or pressing `Ctrl+C` stops new launches, terminates children cleanly, and leaves no zombie processes.
7. **Log Verification:** Confirm that all significant events are recorded in `watchdog.log` with valid timestamps.

## Test Commands
```bash
# Build and run automated tests
make test

# Manual signal tests (from another terminal)
kill -SIGUSR1 <WATCHDOG_PID>
kill -SIGKILL <WORKER_PID>
kill -SIGTERM <WATCHDOG_PID>
```

## Verification Table

| # | Scenario | Action / Command | Expected Result | Status |
|---|---|---|---|---|
| 1 | Normal Operation | `./watchdog` | At least 2 child processes start and are logged | Pending Integration |
| 2 | Normal Termination | Worker exits with `exit(0)` | Detected via `waitpid()`, no restart triggered | Pending Integration |
| 3 | Unexpected Termination | `kill -SIGKILL <WORKER_PID>` | Detected via `SIGCHLD`, worker is restarted | Pending Integration |
| 4 | Restart Limit | Kill worker 4 times | Restart stops after 3 attempts | Pending Integration |
| 5 | Status Reporting | `kill -SIGUSR1 <WATCHDOG_PID>` | All child PIDs and states written to `watchdog.log` | Pending Integration |
| 6 | Graceful Shutdown (`SIGTERM`) | `kill -SIGTERM <WATCHDOG_PID>` | Children receive `SIGTERM`, reaped cleanly, no zombies | Pending Integration |
| 7 | Graceful Shutdown (`SIGINT`) | Press `Ctrl+C` | Children terminated and reaped cleanly | Pending Integration |
| 8 | Log Verification | Inspect `watchdog.log` | All events appear with `[YYYY-MM-DD HH:MM:SS]` timestamps | Pending Integration |