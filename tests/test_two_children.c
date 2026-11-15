#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "../include/process_manager.h"

int main(void)
{
    pid_t children[2];
    int restartCounts[2] = {0, 0};

    for (int i = 0; i < 2; i++)
    {
        children[i] = childBaslat("./worker");

        if (children[i] == -1)
        {
            perror("childBaslat");
            return 1;
        }

        printf("Child %d started. PID: %ld\n",
               i + 1, (long)children[i]);
    }

    sleep(1);

    for (int i = 0; i < 2; i++)
    {
        int result = childIzleVeYenidenBaslat(
            &children[i],
            "./worker",
            &restartCounts[i]
        );

        printf("Child %d result: %d\n", i + 1, result);
    }

    return 0;
}
