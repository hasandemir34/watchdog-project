#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("Worker baslatildi! PID: %ld\n",
           (long)getpid());

    return 0;
}
