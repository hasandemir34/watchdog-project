#ifndef LOGGER_H
#define LOGGER_H

#include <stddef.h>
#include <sys/types.h>

typedef enum {
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_STATUS
} LogLevel;

typedef struct {
    pid_t pid;
    const char *name;
    const char *state;
    int restart_count;
} ProcessStatusSnapshot;

int logger_init(const char *log_file_path);
void logger_close(void);
int logger_log(LogLevel level, const char *fmt, ...);
int logger_log_status_report(const ProcessStatusSnapshot *procs, size_t count);

#endif