#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "../include/process_manager.h"

int main(void)
{
    pid_t pid = childBaslat("./worker");

    if (pid == -1)
        return 1;

    int status;

    if (kill(pid, SIGKILL) == -1)
    {
        perror("kill");
        return 1;
    }

    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        return 1;
    }

    if (WIFSIGNALED(status))
    {
        printf("Child killed by signal: %d\n",
               WTERMSIG(status));
    }

    return 0;
}
