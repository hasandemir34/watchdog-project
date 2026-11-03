# Watchdog – POSIX Process Monitoring System
#System Programming – Sakarya University  

## Project Description
A process monitoring and management system developed in C using POSIX system calls on Linux.


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


