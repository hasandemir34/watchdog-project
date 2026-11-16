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
    
    const char *safe_desc = (event_description != NULL) ? event_description : "BILINMEYEN_OLAY";
    
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    
    if (time_str != NULL) {
        time_str[strlen(time_str) - 1] = '\0';
        fprintf(log_file, "[%s] EVENT: %s\n", time_str, safe_desc);
    } else {
        fprintf(log_file, "[ZAMAN_ALINAMADI] EVENT: %s\n", safe_desc);
    }
    fflush(log_file);
}

void log_process_status(pid_t pid, const char *status) {
    if (log_file == NULL) return;
    
    const char *safe_status = (status != NULL) ? status : "BILINMIYOR";
    
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    
    if (time_str != NULL) {
        time_str[strlen(time_str) - 1] = '\0';
        fprintf(log_file, "[%s] PROCESS STATUS - PID: %d, State: %s\n", time_str, pid, safe_status);
    } else {
        fprintf(log_file, "[ZAMAN_ALINAMADI] PROCESS STATUS - PID: %d, State: %s\n", pid, safe_status);
    }
    fflush(log_file);
}

void close_logger(void) {
    if (log_file != NULL) {
        log_event("Watchdog log sistemi kapatiliyor.");
        fclose(log_file);
        log_file = NULL;
    }
}
