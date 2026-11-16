#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "../include/process_manager.h"

int main(void)
{
    pid_t pid = childBaslat("./worker");

    if (pid == -1)
    {
        return 1;
    }

    printf("Parent: Child PID = %ld\n", (long)pid);

    int status;

    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(status))
    {
        printf("Child exit status: %d\n",
               WEXITSTATUS(status));
    }

    return 0;
}
