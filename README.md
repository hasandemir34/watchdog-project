# Watchdog: POSIX Signal-Based Process Monitoring System

## 1. Proje Amacı ve Mimari
Bu proje, POSIX sistem çağrılarını kullanarak child process'leri (işçi süreçler) başlatan, izleyen ve sinyaller aracılığıyla yöneten bir "Process Supervision" (Süreç Gözetim) sistemidir. Ana program (`watchdog`), `fork()` ve `exec()` kullanarak alt süreçleri başlatır, `SIGCHLD` ve `waitpid()` ile yaşam döngülerini asenkron olarak takip eder. Beklenmedik şekilde kapanan süreçler için sınırlı (bounded) otomatik yeniden başlatma (restart) mekanizması içerir.

## 2. Kurulum ve Derleme
Proje Linux/POSIX uyumlu bir ortamda çalışmak üzere tasarlanmıştır. Derlemek için ana dizinde terminal açıp şu komutu girmeniz yeterlidir:
```bash
make

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


