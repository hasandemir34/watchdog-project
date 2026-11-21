# POSIX Signal-Based Process Monitoring and Management System (`watchdog`)

**Sakarya University — Software Engineering Department**  
**2026–2027 System Programming Term Project 1**

## Overview
`watchdog` is a POSIX-compliant process supervision system written in C. It spawns and monitors multiple child worker processes, detects normal and unexpected terminations, automatically restarts crashed workers up to a bounded limit (maximum 3 restarts per worker), logs lifecycle events with timestamps, and responds safely to runtime signals.

## Key Features
- **Child Process Creation:** Spawns at least two worker processes using `fork()` and `exec()`.
- **Termination Monitoring:** Reaps terminated children via `SIGCHLD` and `waitpid()`, distinguishing normal exits from signal/error terminations.
- **Bounded Automatic Restart:** Automatically restarts unexpectedly terminated workers up to **3 times per worker**, preventing infinite restart loops and blocking restarts once shutdown is requested.
- **Periodic Monitoring:** Tracks active child processes using periodic timer checks (`SIGALRM` / `setitimer()`).
- **Async-Signal-Safe Handling:** Uses `sigaction()` with `volatile sig_atomic_t` flags; all I/O and process operations execute safely inside the main control loop.
- **Status Reporting (`SIGUSR1`):** Writes the PID, state, and restart count of every tracked worker to `watchdog.log`.
- **Graceful Shutdown (`SIGINT` / `SIGTERM`):** Stops new launches, sends `SIGTERM` to all active workers, waits for clean termination, and falls back to `SIGKILL` if a worker does not exit within the timeout—leaving zero zombie or orphan processes.
- **Timestamped Event Logging:** Uses POSIX `open()` / `write()` calls to record process creation, termination, restart, status reports, and shutdown events in `[YYYY-MM-DD HH:MM:SS]` format.

## Project Structure
```text
watchdog-project/
├── .gitignore
├── Makefile
├── README.md
├── include/
│   ├── logger.h
│   ├── process_manager.h
│   └── signal_manager.h
├── src/
│   ├── logger.c
│   ├── main.c
│   ├── process_manager.c
│   └── signal_manager.c
├── workers/
│   └── worker.c
└── tests/
    ├── test_scenarios.sh
    ├── test_child.c
    ├── test_child_exit.c
    ├── test_child_kill.c
    ├── test_manager_restart.c
    ├── test_process_manager.c
    ├── test_restart.c
    ├── test_sigchld.c
    ├── test_timer.c
    └── test_two_children.c
```

## Module Responsibilities
- **Signal Management & Main Loop (`src/main.c`, `src/signal_manager.c`, `include/signal_manager.h`):** Installs async-signal-safe handlers for `SIGINT`, `SIGTERM`, and `SIGUSR1`, runs the main supervision loop, integrates modules, and coordinates graceful shutdown.
- **Child Process Management (`src/process_manager.c`, `include/process_manager.h`, `workers/worker.c`):** Manages `fork()`/`exec()`, `SIGCHLD` + `waitpid()` reaping, normal vs. unexpected exit detection, bounded restart logic (max 3), and periodic checks.
- **Logging & Test Infrastructure (`src/logger.c`, `include/logger.h`, `Makefile`, `tests/test_scenarios.sh`, `README.md`):** Implements POSIX timestamped logging, `SIGUSR1` status reporting, build automation, and end-to-end test verification.

## Requirements
- Linux environment (or WSL)
- `gcc` (C11 / POSIX.1-2008 compliant)
- `make`
- `bash` and standard POSIX utilities (`pgrep`, `pkill`, `kill`, `grep`)

## Build & Run
Compile the `watchdog` supervisor and `workers/worker` binary:
```bash
make
```

Start the supervisor:
```bash
./watchdog
```

## Runtime Signals
From a second terminal, use the PID of the running `watchdog` or worker process:
```bash
# Write current PID, state, and restart count of all workers to watchdog.log
kill -SIGUSR1 <WATCHDOG_PID>

# Trigger graceful shutdown
kill -SIGTERM <WATCHDOG_PID>
# Alternatively, press Ctrl+C (SIGINT) in the terminal running watchdog

# Simulate an unexpected worker crash (for testing only)
kill -SIGKILL <WORKER_PID>
```

## Testing

### Automated End-to-End Test Suite
Run a clean build followed by the full automated test suite (`tests/test_scenarios.sh`):
```bash
make test
```

The automated suite verifies 7 core scenarios:
1. **Normal Operation:** Watchdog starts and spawns at least 2 worker processes.
2. **Status Reporting (`SIGUSR1`):** Both worker PIDs appear in `watchdog.log` after sending `SIGUSR1`.
3. **Bounded Restart Limit:** Tracks a single worker across 4 consecutive `SIGKILL` terminations—verifying exactly 3 restarts and no restart after the 4th termination.
4. **Graceful Shutdown (`SIGTERM`):** Watchdog and all child processes terminate cleanly without orphans/zombies.
5. **Graceful Shutdown (`SIGINT`):** Automated `SIGINT` shuts down watchdog and all workers cleanly.
6. **`SIGTERM` Timeout & `SIGKILL` Fallback:** Freezes a worker with `SIGSTOP`, sends `SIGTERM` to watchdog, and confirms the unresponsive worker is forcefully terminated via `SIGKILL`.
7. **Log Timestamp Verification:** Confirms `watchdog.log` entries follow the `[YYYY-MM-DD HH:MM:SS]` format.

### Unit Tests (`tests/*.c`)
The `tests/` directory also contains standalone C unit tests covering individual process management and signal behaviors (`test_child.c`, `test_child_exit.c`, `test_child_kill.c`, `test_manager_restart.c`, `test_process_manager.c`, `test_restart.c`, `test_sigchld.c`, `test_timer.c`, `test_two_children.c`).

## Sample Log Output (`watchdog.log`)
```text
[2026-11-21 17:30:01] [INFO] Watchdog started (PID: 12340)
[2026-11-21 17:30:01] [INFO] Spawned worker #1 (PID: 12341)
[2026-11-21 17:30:01] [INFO] Spawned worker #2 (PID: 12342)
[2026-11-21 17:30:03] [STATUS] === PROCESS STATUS REPORT (SIGUSR1) ===
[2026-11-21 17:30:03] [STATUS] Child #1 | Name: worker | PID: 12341 | State: RUNNING | Restarts: 0
[2026-11-21 17:30:03] [STATUS] Child #2 | Name: worker | PID: 12342 | State: RUNNING | Restarts: 0
[2026-11-21 17:30:03] [STATUS] === END OF STATUS REPORT ===
[2026-11-21 17:30:10] [WARN] Worker PID 12342 terminated unexpectedly; restarting (attempt 1/3)
[2026-11-21 17:30:20] [INFO] Shutdown signal received; terminating child processes gracefully
```

## Clean Build Artifacts
Remove compiled binaries, object files, and generated log files:
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


