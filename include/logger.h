#ifndef LOGGER_H
#define LOGGER_H

#include <sys/types.h>

int init_logger(const char *log_file_path);

void log_event(const char *event_description);

void log_process_status(pid_t pid, const char *status);

void close_logger(void);

#endif /* LOGGER_H */
