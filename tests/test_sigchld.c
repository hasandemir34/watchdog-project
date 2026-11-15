#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/process_manager.h"

static volatile sig_atomic_t childBitti = 0;

static void childSinyali(int sig)
{
    (void)sig;
    childBitti = 1;
}

int main(void)
{
    struct sigaction sa = {0};

    sa.sa_handler = childSinyali;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }

    pid_t pid = childBaslat("./worker");

    if (pid == -1)
        return 1;

    printf("Child PID: %ld\n", (long)pid);
    fflush(stdout);

    while (!childBitti)
        pause();

    int status;
    pid_t result = waitpid(pid, &status, 0);

    if (result == -1)
    {
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(status))
        printf("Child exit status: %d\n", WEXITSTATUS(status));

    return 0;
}
