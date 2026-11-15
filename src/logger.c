#include "logger.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

static FILE *log_file = NULL;

int init_logger(const char *log_file_path) {
    log_file = fopen(log_file_path, "a");
    if (log_file == NULL) {
        perror("Log dosyasi acilamadi");
        return -1;
    }
    return 0;
}

void log_event(const char *event_description) {
    if (log_file == NULL) return;
    
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';
    
    fprintf(log_file, "[%s] EVENT: %s\n", time_str, event_description);
    fflush(log_file); 
}

void log_process_status(pid_t pid, const char *status) {
    if (log_file == NULL) return;
    
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';
    
    fprintf(log_file, "[%s] PROCESS STATUS - PID: %d, State: %s\n", time_str, pid, status);
    fflush(log_file);
}

void close_logger(void) {
    if (log_file != NULL) {
        log_event("Watchdog log sistemi kapatiliyor.");
        fclose(log_file);
        log_file = NULL;
    }
}
