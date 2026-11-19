# POSIX Signal-Based Process Monitoring System (`watchdog`)

Sakarya University — Software Engineering Department  
2026–2027 System Programming Term Project 1

## Overview
`watchdog` is a C-based process supervision system using POSIX system calls. It spawns and monitors child processes, restarts unexpectedly terminated workers up to a defined limit, logs events with timestamps, and handles runtime signals safely.

## Build & Run
```bash
make
./watchdog