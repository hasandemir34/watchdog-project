# POSIX Signal-Based Process Monitoring System (`watchdog`)

Sakarya University — Software Engineering Department  
2026–2027 System Programming Term Project 1

## Overview
`watchdog` is a C-based process supervision system using POSIX system calls. It spawns and monitors child processes, restarts unexpectedly terminated workers up to a defined limit, logs events with timestamps, and handles runtime signals safely.

## Build & Run
```bash
make
./watchdog

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


