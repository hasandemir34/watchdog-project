#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <stddef.h>
#include <sys/types.h>

#define CHILD_PROCESS_MAX_RESTARTS 3

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
    int restartCount;
    int aktif;
    int kapatmaIstendi;
}
 ChildProcess;
/* Initialize ChildProcess with {0} before first use.
   Example: ChildProcess child = {0}; */
pid_t childBaslat(const char *program);
int childIzleVeYenidenBaslat(pid_t *pid, const char *program, int *restartCount);
int childProcessBaslat(ChildProcess *child, const char *program);
int childProcessIzleVeYenidenBaslat(ChildProcess *child);
int childProcessSonlandir(ChildProcess *child);
int childProcessleriIzle(ChildProcess *children, size_t count);
int childProcessleriSonlandir(ChildProcess *children, size_t count);

#endif
