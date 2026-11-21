#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogLevel;

int logger_init(const char *log_file_path);
void logger_close(void);
int logger_log(LogLevel level, const char *fmt, ...);

#endif