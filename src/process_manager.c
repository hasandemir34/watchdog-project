#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
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

static int wait_for_child(pid_t pid, int *status)
{
    pid_t result;

    do { result = waitpid(pid, status, 0); } while (result == -1 && errno == EINTR);
    return result == pid ? 0 : -1;
}

static int report_exec_error(int fd, int error_number)
{
    const char *data = (const char *)&error_number;
    size_t remaining = sizeof(error_number);

    while (remaining > 0)
    {
        ssize_t written = write(fd, data, remaining);
        if (written > 0)
        {
            data += written;
            remaining -= (size_t)written;
        }
        else if (written == -1 && errno == EINTR)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return remaining == 0 ? 0 : -1;
}

pid_t childBaslat(const char *program)
{
    if (program == NULL || *program == '\0')
    {
        errno = EINVAL;
        return -1;
    }

    int exec_status[2];
    if (pipe(exec_status) == -1)
    {
        perror("pipe");
        return -1;
    }

    int flags = fcntl(exec_status[1], F_GETFD);
    if (flags == -1 || fcntl(exec_status[1], F_SETFD, flags | FD_CLOEXEC) == -1)
    {
        int saved_errno = errno;
        close(exec_status[0]);
        close(exec_status[1]);
        errno = saved_errno;
        perror("fcntl");
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        int saved_errno = errno;
        close(exec_status[0]);
        close(exec_status[1]);
        errno = saved_errno;
        perror("fork");
        return -1;
    }

    if (pid == 0)
    {
        close(exec_status[0]);
        execl(program, program, (char *)NULL);

        int exec_errno = errno;
        (void)report_exec_error(exec_status[1], exec_errno);
        _exit(127);
    }

    close(exec_status[1]);

    int exec_errno = 0;
    char *data = (char *)&exec_errno;
    size_t remaining = sizeof(exec_errno);
    int read_failed = 0;
    while (remaining > 0)
    {
        ssize_t read_count = read(exec_status[0], data, remaining);
        if (read_count > 0)
        {
            data += read_count;
            remaining -= (size_t)read_count;
        }
        else if (read_count == -1 && errno == EINTR)
        {
            continue;
        }
        else
        {
            read_failed = read_count == -1;
            break;
        }
    }

    close(exec_status[0]);

    if (remaining == 0)
    {
        int status;
        (void)wait_for_child(pid, &status);
        errno = exec_errno;
        fprintf(stderr, "exec '%s' failed: %s\n", program, strerror(exec_errno));
        return -1;
    }

    if (read_failed || remaining != sizeof(exec_errno))
    {
        int status;
        if (wait_for_child(pid, &status) == -1 && errno != ECHILD)
        {
            return -1;
        }
        errno = EIO;
        perror("read");
        return -1;
    }

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
    if (WIFEXITED(status))
    {
        int exit_code = WEXITSTATUS(status);
        if (exit_code == 0)
        {
            printf("Child %ld normally exited.\n", (long)old_pid);
            return CHILD_PROCESS_EXITED;
        }

        printf("Child %ld exited with code %d.\n", (long)old_pid, exit_code);
    }
    else if (WIFSIGNALED(status))
        printf("Child %ld killed by signal %d.\n", (long)old_pid, WTERMSIG(status));
    else
    {
        errno = ECHILD;
        return CHILD_PROCESS_ERROR;
    }

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
    if (child == NULL) { errno = EINVAL; return CHILD_PROCESS_ERROR; }
    if (child->kapatmaIstendi) return CHILD_PROCESS_RUNNING;
    if (child->program == NULL || child->pid <= 0) { errno = EINVAL; return CHILD_PROCESS_ERROR; }
    int result = childIzleVeYenidenBaslat(&child->pid, child->program, &child->restartCount);
    if (result == CHILD_PROCESS_EXITED || result == CHILD_PROCESS_RESTART_LIMIT) child->aktif = 0;
    else if (result == CHILD_PROCESS_RESTARTED) child->aktif = 1;
    else if (result == CHILD_PROCESS_ERROR && child->pid <= 0) child->aktif = 0;
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
    const int poll_count = CHILD_PROCESS_SHUTDOWN_TIMEOUT_MS /
                           CHILD_PROCESS_SHUTDOWN_POLL_INTERVAL_MS;
    for (int attempt = 0; attempt < poll_count; ++attempt)
    {
        do { result = waitpid(pid, &status, WNOHANG); } while (result == -1 && errno == EINTR);
        if (result == pid || (result == -1 && errno == ECHILD)) goto reaped;
        if (result == -1) return CHILD_PROCESS_ERROR;
        struct timespec delay = {
            0,
            CHILD_PROCESS_SHUTDOWN_POLL_INTERVAL_MS * 1000000L
        };
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
