#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "../include/process_manager.h"

int main(void)
{
    ChildProcess children[2] = {0};

    if (childProcessBaslat(&children[0], "./worker") != 0 ||
        childProcessBaslat(&children[1], "./worker") != 0)
        return 1;

    if (!children[0].aktif || !children[1].aktif)
        return 2;

    struct timespec delay = {0, 200000000};
    nanosleep(&delay, NULL);
    if (childProcessleriIzle(children, 2) < 0)
        return 3;

    if (childProcessleriSonlandir(children, 2) != 0)
        return 4;

    if (children[0].aktif || children[1].aktif ||
        children[0].pid != -1 || children[1].pid != -1)
        return 5;

    puts("process manager collection test passed");
    return 0;
}
