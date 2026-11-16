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

    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(status))
    {
        printf("Normal exit. Code: %d\n",
               WEXITSTATUS(status));
    }
    else if (WIFSIGNALED(status))
    {
        printf("Killed by signal: %d\n",
               WTERMSIG(status));
    }

    return 0;
}
