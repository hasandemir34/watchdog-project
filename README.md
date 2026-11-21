# Watchdog Project

A simple process monitoring system developed in **C** for the System Programming course at Sakarya University.

## Project Features

- Creates and monitors two worker processes.
- Automatically restarts failed workers (maximum 3 times).
- Uses `SIGCHLD` and `SIGALRM` for process monitoring.
- Uses `SIGUSR1` to display process information.
- Supports safe shutdown with `SIGINT` and `SIGTERM`.
- Records events in `watchdog.log`.

## Team Members

| Member | Responsibility |
|---|---|
| Hasan Demir | Project management, signal handling and main program |
| Elif Yırtıcı| Child process management |
| Ahmet Yiğit| Logging and automated tests |

## Installation

Compile the project:

```bash
make
```

Run the program:

```bash
./watchdog
```

## Testing

Run all automated tests:

```bash
make test
```

**Test Results: 7 passed, 0 failed.**

## Useful Commands

Request process status:

```bash
kill -USR1 $(pgrep -n -x watchdog)
```

Stop the Watchdog:

```bash
kill -TERM $(pgrep -n -x watchdog)
```

Clean generated files:

```bash
make clean
```

## Technologies

C, Linux, POSIX Signals, GCC, Makefile, Git and GitHub.