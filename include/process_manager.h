#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <stddef.h>
#include <sys/types.h>

#define CHILD_PROCESS_MAX_RESTARTS 3
#define CHILD_PROCESS_SHUTDOWN_TIMEOUT_MS 200
#define CHILD_PROCESS_SHUTDOWN_POLL_INTERVAL_MS 10

enum ChildProcessResult
{
    CHILD_PROCESS_RUNNING = 0,
    CHILD_PROCESS_EXITED = 1,
    CHILD_PROCESS_RESTART_LIMIT = 2,
    CHILD_PROCESS_RESTARTED = 3,
    CHILD_PROCESS_ERROR = -1
};

typedef struct
{
    pid_t pid;
    const char *program;
    const char *const *arguments;
    int restartCount;
    int aktif;
    int kapatmaIstendi;
}
 ChildProcess;
/* Initialize ChildProcess with {0} before first use.
   Example: ChildProcess child = {0};

   childProcessleriIzle() is intended to be called from the application's
   normal loop after a periodic SIGALRM/setitimer notification. Signal
   handlers must only set a volatile sig_atomic_t flag; they must not call
   any process-manager function. The loop may also be woken by SIGCHLD and
   should call this function again. The manager uses waitpid(..., WNOHANG)
   and retries EINTR, so polling does not block while a child is running.

   childProcessSonlandir() marks a child as shutting down before signalling
   it. It waits up to CHILD_PROCESS_SHUTDOWN_TIMEOUT_MS, polling every
   CHILD_PROCESS_SHUTDOWN_POLL_INTERVAL_MS, before escalating to SIGKILL.
   Subsequent monitoring calls leave it stopped and never restart it. */
pid_t childBaslat(const char *program);
pid_t childBaslatAvecArguments(const char *program, const char *const arguments[]);
int childIzleVeYenidenBaslat(pid_t *pid, const char *program, int *restartCount);
int childProcessBaslat(ChildProcess *child, const char *program);
int childProcessBaslatAvecArguments(ChildProcess *child, const char *program,
                                    const char *const arguments[]);
int childProcessIzleVeYenidenBaslat(ChildProcess *child);
int childProcessSonlandir(ChildProcess *child);
int childProcessleriIzle(ChildProcess *children, size_t count);
int childProcessleriSonlandir(ChildProcess *children, size_t count);

#endif
