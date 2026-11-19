# Watchdog: POSIX Signal-Based Process Monitoring System

# POSIX Signal-Based Process Monitoring System (`watchdog`)

Sakarya University — Software Engineering Department  
2026–2027 System Programming Term Project 1

## Overview
`watchdog` is a C-based process supervision system built with POSIX system calls. It creates and monitors child processes, automatically restarts unexpectedly terminated workers up to a defined limit (max 3), logs lifecycle events with timestamps, and handles runtime signals safely.

## Project Structure
```text
watchdog-project/
├── Makefile
├── README.md
├── include/
│   ├── logger.h
│   ├── process_manager.h
│   └── signal_manager.h
├── src/
│   ├── main.c
│   ├── logger.c
│   ├── process_manager.c
│   └── signal_manager.c
├── workers/
│   └── worker.c
├── tests/
│   └── test_scenarios.sh
└── docs/
    └── TEST_DOCS.md
```

## Module Responsibilities
- **Signal Management & Main Loop (`main.c`, `signal_manager.c`):** Installs async-signal-safe handlers using `sigaction()` for `SIGINT`, `SIGTERM`, and `SIGUSR1`, runs the main control loop, and manages graceful shutdown.
- **Child Process Management (`process_manager.c`):** Spawns workers via `fork()` and `exec()`, reaps terminated children with `SIGCHLD` and `waitpid()`, distinguishes normal vs. unexpected exits, and enforces the 3-restart limit.
- **Logging & Test Infrastructure (`logger.c`, `Makefile`, `tests/`):** Writes timestamped event logs, handles `SIGUSR1` process status reporting, and provides automated test scenarios and documentation.

## Build & Run
Compile the `watchdog` and `worker` binaries using `make`:
```bash
make
./watchdog
```

## Runtime Signals
From another terminal, send signals using the PID of the running `watchdog` or worker process:
```bash
# Write current child process PIDs and states to watchdog.log
kill -SIGUSR1 <WATCHDOG_PID>

# Trigger graceful shutdown
kill -SIGTERM <WATCHDOG_PID>
# Or press Ctrl+C (SIGINT) in the terminal running watchdog

# Simulate unexpected worker termination (for testing only)
kill -SIGKILL <WORKER_PID>
```

## Running Tests
Run the automated test suite covering normal startup, `SIGUSR1` reporting, unexpected termination, restart limits, and graceful shutdown:
```bash
make test
```

## Clean Build Artifacts
Remove compiled binaries, object files, and logs:
```bash
make clean
```


## Team & Responsibilities

### Project Manager – Hasan
**Signal Management & Main Program**
- Implement signal handling using `sigaction()`.
- Handle `SIGINT`, `SIGTERM`, and `SIGUSR1`.
- Develop the main control loop.
- Implement graceful shutdown.
- Coordinate module integration.

### Elif – Child Process Management
- Create child processes using `fork()` and `exec()`.
- Monitor processes using `SIGCHLD` and `waitpid()`.
- Handle unexpected termination and automatic restart.
- Implement periodic process checks.

### Ahmet – Logging & Testing
- Implement event logging with timestamps.
- Implement process status reporting.
- Prepare the `Makefile` and test scenarios.
- Maintain project documentation.
- Add automated SIGINT shutdown test.
- Verify exactly 3 restart attempts.
- Test SIGTERM timeout and SIGKILL fallback.
- Verify both worker PIDs in SIGUSR1 logs.Confirm child processes terminate after shutdown.
- Improve make test coverage.



