#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#include "../include/process_manager.h"

int main(void)
{
    int restartCount = 0;
    pid_t pid = childBaslat("./worker");

    if (pid == -1)
        return 1;

    while (pid > 0)
    {
        if (kill(pid, SIGKILL) == -1)
        {
            perror("kill");
            return 1;
        }

        int result = 0;

        while (result == 0)
        {
            result = childIzleVeYenidenBaslat(
                &pid, "./worker", &restartCount
            );

            if (result == -1)
                return 1;

            if (result == 0)
            {
                struct timespec delay = {0, 100000000};
                struct timespec remaining;

                while (nanosleep(&delay, &remaining) == -1)
                {
                    if (errno != EINTR)
                    {
                        perror("nanosleep");
                        return 1;
                    }

                    delay = remaining;
                }
            }
        }
    }

    printf("Total restarts: %d\n", restartCount);

    return 0;
}
