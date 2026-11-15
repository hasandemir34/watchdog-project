#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include "../include/process_manager.h"

static int reap_nonblocking(pid_t pid, int *status)
{
    pid_t result;
    do { result = waitpid(pid, status, WNOHANG); } while (result == -1 && errno == EINTR);
    if (result == 0) return CHILD_PROCESS_RUNNING;
    if (result == -1) return CHILD_PROCESS_ERROR;
    return CHILD_PROCESS_EXITED;
}

static int expected_exit(int status) { return WIFEXITED(status) && WEXITSTATUS(status) == 0; }

pid_t childBaslat(const char *program)
{
    if (program == NULL || *program == '\0') { errno = EINVAL; return -1; }
    pid_t pid = fork();
    if (pid == -1) { perror("fork"); return -1; }
    if (pid == 0) { execl(program, program, (char *)NULL); perror("execl"); _exit(127); }
    return pid;
}

int childIzleVeYenidenBaslat(pid_t *pid, const char *program, int *restartCount)
{
    if (pid == NULL || program == NULL || restartCount == NULL || *pid <= 0 || *restartCount < 0)
    { errno = EINVAL; return CHILD_PROCESS_ERROR; }

    int status;
    int reap_result = reap_nonblocking(*pid, &status);
    if (reap_result == CHILD_PROCESS_RUNNING || reap_result == CHILD_PROCESS_ERROR) return reap_result;

    pid_t old_pid = *pid;
    *pid = -1;
    if (expected_exit(status)) { printf("Child %ld normally exited.\n", (long)old_pid); return CHILD_PROCESS_EXITED; }

    if (WIFSIGNALED(status))
        printf("Child %ld killed by signal %d.\n", (long)old_pid, WTERMSIG(status));
    else if (WIFEXITED(status))
        printf("Child %ld exited with code %d.\n", (long)old_pid, WEXITSTATUS(status));

    if (*restartCount >= CHILD_PROCESS_MAX_RESTARTS)
    { printf("Restart limit reached.\n"); return CHILD_PROCESS_RESTART_LIMIT; }

    ++*restartCount;
    printf("Restarting worker... Attempt %d/%d\n", *restartCount, CHILD_PROCESS_MAX_RESTARTS);
    *pid = childBaslat(program);
    return *pid == -1 ? CHILD_PROCESS_ERROR : CHILD_PROCESS_RESTARTED;
}

int childProcessBaslat(ChildProcess *child, const char *program)
{
    if (child == NULL || program == NULL || *program == '\0') { errno = EINVAL; return CHILD_PROCESS_ERROR; }
    if (child->aktif && child->pid > 0) { errno = EALREADY; return CHILD_PROCESS_ERROR; }
    child->pid = -1; child->program = program; child->restartCount = 0; child->kapatmaIstendi = 0;
    child->pid = childBaslat(program); child->aktif = child->pid > 0;
    return child->aktif ? 0 : CHILD_PROCESS_ERROR;
}

int childProcessIzleVeYenidenBaslat(ChildProcess *child)
{
    if (child == NULL || child->program == NULL || child->pid <= 0) { errno = EINVAL; return CHILD_PROCESS_ERROR; }
    if (child->kapatmaIstendi) return CHILD_PROCESS_RUNNING;
    int result = childIzleVeYenidenBaslat(&child->pid, child->program, &child->restartCount);
    if (result == CHILD_PROCESS_EXITED || result == CHILD_PROCESS_RESTART_LIMIT) child->aktif = 0;
    else if (result == CHILD_PROCESS_RESTARTED) child->aktif = 1;
    return result;
}

int childProcessSonlandir(ChildProcess *child)
{
    if (child == NULL) { errno = EINVAL; return CHILD_PROCESS_ERROR; }
    child->kapatmaIstendi = 1;
    if (!child->aktif || child->pid <= 0) { child->pid = -1; child->aktif = 0; return 0; }

    pid_t pid = child->pid;
    if (kill(pid, SIGTERM) == -1 && errno != ESRCH) return CHILD_PROCESS_ERROR;
    int status; pid_t result;
    for (int attempt = 0; attempt < 20; ++attempt)
    {
        do { result = waitpid(pid, &status, WNOHANG); } while (result == -1 && errno == EINTR);
        if (result == pid || (result == -1 && errno == ECHILD)) goto reaped;
        if (result == -1) return CHILD_PROCESS_ERROR;
        struct timespec delay = {0, 10000000};
        nanosleep(&delay, NULL);
    }
    if (kill(pid, SIGKILL) == -1 && errno != ESRCH) return CHILD_PROCESS_ERROR;
    do { result = waitpid(pid, &status, 0); } while (result == -1 && errno == EINTR);
    if (result == -1 && errno != ECHILD) return CHILD_PROCESS_ERROR;
reaped:
    child->pid = -1; child->aktif = 0; return 0;
}

int childProcessleriIzle(ChildProcess *children, size_t count)
{
    if (children == NULL && count != 0) { errno = EINVAL; return CHILD_PROCESS_ERROR; }
    int changed = 0;
    for (size_t i = 0; i < count; ++i)
    {
        if (!children[i].aktif) continue;
        int result = childProcessIzleVeYenidenBaslat(&children[i]);
        if (result == CHILD_PROCESS_ERROR) return CHILD_PROCESS_ERROR;
        if (result != CHILD_PROCESS_RUNNING) ++changed;
    }
    return changed;
}

int childProcessleriSonlandir(ChildProcess *children, size_t count)
{
    if (children == NULL && count != 0) { errno = EINVAL; return CHILD_PROCESS_ERROR; }
    int result = 0;
    for (size_t i = 0; i < count; ++i)
        if (childProcessSonlandir(&children[i]) == CHILD_PROCESS_ERROR) result = CHILD_PROCESS_ERROR;
    return result;
}
