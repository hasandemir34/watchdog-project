# Watchdog — POSIX Process Monitoring System

A process monitoring and recovery system developed in **C** using POSIX system calls and Linux signals.

**Sakarya University | Software Engineering | System Programming | 2026–2027**

## 📌 Project Overview

Watchdog is a process supervision application that creates, monitors, and manages child processes.

The main application starts two worker processes and continuously monitors their execution. If a worker terminates unexpectedly, Watchdog automatically restarts it up to **3 times**.

The system also supports signal-based process control, status reporting, event logging, and graceful shutdown.

## ⚙️ Features

- **Process Management:** Creates two child processes using `fork()` and `exec()`.
- **Process Monitoring:** Detects terminated processes using `SIGCHLD` and `waitpid()`.
- **Automatic Restart:** Restarts unexpectedly terminated workers up to 3 times per worker.
- **Periodic Monitoring:** Uses `SIGALRM` and `setitimer()` to periodically check processes.
- **Status Reporting:** Uses `SIGUSR1` to report worker PIDs, states, and restart counts.
- **Graceful Shutdown:** Handles `SIGINT` and `SIGTERM` to terminate child processes safely.
- **Forced Termination:** Uses `SIGKILL` if a worker does not terminate within the timeout.
- **Event Logging:** Records process events with timestamps in `watchdog.log`.

## 🏗️ Project Structure

```text
watchdog-project/
├── include/
│   ├── signal_manager.h
│   ├── process_manager.h
│   └── logger.h
├── src/
│   ├── main.c
│   ├── signal_manager.c
│   ├── process_manager.c
│   └── logger.c
├── workers/
│   └── worker.c
├── tests/
│   └── test_scenarios.sh
├── Makefile
└── README.md
```

### Module Responsibilities

| Module | Description |
|---|---|
| `main.c` | Main control loop and module integration |
| `signal_manager.c` | POSIX signal handling |
| `process_manager.c` | Child creation, monitoring, restart, and termination |
| `logger.c` | Timestamped event logging and status reports |
| `worker.c` | Worker process executable |
| `test_scenarios.sh` | Automated verification of system functionality |

## 🚀 Installation & Usage

### Requirements

- Linux
- GCC Compiler
- GNU Make

### 1. Clone the Repository

```bash
git clone https://github.com/hasandemir34/watchdog-project.git
cd watchdog-project
```

### 2. Compile

```bash
make
```

### 3. Run

```bash
./watchdog
```

The application automatically starts two worker processes.

Example output:

```text
Watchdog baslatildi! PID: 12695
Worker 1 baslatildi! PID: 12696
Worker 2 baslatildi! PID: 12697
```

## 🧪 Testing the System

### Check Running Workers

Open a second terminal:

```bash
pgrep -a -P $(pgrep -n -x watchdog)
```

### Test Automatic Restart

Terminate a worker using its actual PID:

```bash
kill -SIGKILL <WORKER_PID>
```

Watchdog detects the termination and starts a replacement process.

Each worker has a maximum of **3 restart attempts**.

### Request a Status Report

```bash
kill -SIGUSR1 $(pgrep -n -x watchdog)
```

The application displays process information and records it in `watchdog.log`.

### Graceful Shutdown

Press `Ctrl+C` in the Watchdog terminal or execute:

```bash
kill -SIGTERM $(pgrep -n -x watchdog)
```

Watchdog terminates its remaining child processes before exiting.

## 📊 Automated Tests

Run the complete test suite:

```bash
make test
```

The automated test suite verifies:

| Test | Result |
|---|---|
| Creation of two worker processes | ✅ PASS |
| SIGUSR1 status reporting | ✅ PASS |
| Automatic restart and restart limit | ✅ PASS |
| SIGTERM graceful shutdown | ✅ PASS |
| SIGINT graceful shutdown | ✅ PASS |
| SIGKILL fallback | ✅ PASS |
| Log timestamp verification | ✅ PASS |

**Final Test Result: 7 Passed, 0 Failed**

Normal worker termination was also verified manually.

## 📝 Logging

The system records important events in `watchdog.log`, including:

- Process creation and termination
- Automatic restart attempts
- Restart limit enforcement
- SIGUSR1 status reports
- Application shutdown

Example:

```text
[2026-11-21 17:57:21] [INFO] Watchdog baslatildi PID=21790
[2026-11-21 17:57:21] [INFO] Worker 1 baslatildi PID=21792
[2026-11-21 17:57:24] [WARN] Worker 2 restart 1/3 PID=21869
[2026-11-21 17:57:32] [INFO] Watchdog guvenli sekilde kapatildi
```

## 👥 Team & Contributions

| Team Member | Responsibilities |
|---|---|
| **Hasan Demir** | Project management, signal handling, main program, and integration |
| **Elif** | Child process management, process monitoring, and automatic restart |
| **Ahmet** | Logging system, Makefile, and automated testing |

The project was developed collaboratively using Git and GitHub. Individual contributions can be reviewed through the repository's commit history.

## 🧹 Cleanup

To remove generated files:

```bash
make clean
```

## 📚 Technologies

**C · Linux · POSIX · GCC · GNU Make · Bash · Git · GitHub**

---

**Sakarya University — System Programming Term Project 1**