#define _POSIX_C_SOURCE 200809L

#include "logger.h"

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define LOG_BUFFER_SIZE 1024

static int g_log_fd = -1;

static const char *level_to_str(LogLevel level) {
    switch (level) {
        case LOG_INFO:   return "INFO";
        case LOG_WARN:   return "WARN";
        case LOG_ERROR:  return "ERROR";
        case LOG_STATUS: return "STATUS";
        default:         return "UNKNOWN";
    }
}

static int write_all(int fd, const char *buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t written = write(fd, buf + total, len - total);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        total += (size_t)written;
    }
    return 0;
}

static void format_timestamp(char *buf, size_t size) {
    time_t now = time(NULL);
    struct tm tm_info;
    if (localtime_r(&now, &tm_info) != NULL) {
        strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tm_info);
    } else {
        snprintf(buf, size, "0000-00-00 00:00:00");
    }
}

int logger_init(const char *log_file_path) {
    if (log_file_path == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (g_log_fd >= 0) {
        close(g_log_fd);
    }
    g_log_fd = open(log_file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (g_log_fd < 0) {
        return -1;
    }
    return 0;
}

void logger_close(void) {
    if (g_log_fd >= 0) {
        close(g_log_fd);
        g_log_fd = -1;
    }
}

int logger_log(LogLevel level, const char *fmt, ...) {
    if (g_log_fd < 0 || fmt == NULL) {
        return -1;
    }

    char timestamp[32];
    format_timestamp(timestamp, sizeof(timestamp));

    char message[LOG_BUFFER_SIZE - 64];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    char final_line[LOG_BUFFER_SIZE];
    int len = snprintf(final_line, sizeof(final_line), "[%s] [%s] %s\n",
                       timestamp, level_to_str(level), message);
    if (len < 0) {
        return -1;
    }

    size_t write_len = ((size_t)len >= sizeof(final_line))
                           ? sizeof(final_line) - 1
                           : (size_t)len;

    return write_all(g_log_fd, final_line, write_len);
}
int logger_log_status_report(const ProcessStatusSnapshot *procs, size_t count) {
    if (g_log_fd < 0) {
        return -1;
    }

    if (logger_log(LOG_STATUS, "=== PROCESS STATUS REPORT (SIGUSR1) ===") < 0) {
        return -1;
    }

    if (procs == NULL || count == 0) {
        logger_log(LOG_STATUS, "No active child processes tracked.");
    } else {
        for (size_t i = 0; i < count; i++) {
            const char *name = procs[i].name ? procs[i].name : "unnamed";
            const char *state = procs[i].state ? procs[i].state : "UNKNOWN";
            logger_log(LOG_STATUS, "Child #%zu | Name: %s | PID: %d | State: %s | Restarts: %d",
                       i + 1, name, (int)procs[i].pid, state, procs[i].restart_count);
        }
    }

    return logger_log(LOG_STATUS, "=== END OF STATUS REPORT ===");
}