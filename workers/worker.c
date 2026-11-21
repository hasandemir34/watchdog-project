#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("Worker baslatildi! PID: %ld\n", (long)getpid());

    while (1)
    {
        sleep(1);
    }

    return 0;
}