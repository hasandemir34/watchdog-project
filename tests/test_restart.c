#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#include "../include/process_manager.h"

#define MAX_RESTART 3

int main(void)
{
    int restartCount = 0;
    pid_t pid = childBaslat("./worker");

    if (pid == -1)
        return 1;

    while (1)
    {
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
            printf("Child %ld killed by signal %d\n",
                   (long)pid, WTERMSIG(status));

            if (restartCount >= MAX_RESTART)
            {
                printf("Restart limit reached!\n");
                break;
            }

            restartCount++;

            printf("Restarting worker... Attempt %d/%d\n",
                   restartCount, MAX_RESTART);

            pid = childBaslat("./worker");

            if (pid == -1)
                return 1;
        }
        else
        {
            printf("Child exited normally.\n");
            break;
        }
    }

    return 0;
}
