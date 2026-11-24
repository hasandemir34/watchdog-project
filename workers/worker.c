#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int parse_exit_delay(const char *value, unsigned int *delay)
{
    char *end = NULL;
    unsigned long parsed;

    errno = 0;
    parsed = strtoul(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0' || parsed > UINT_MAX)
        return -1;

    *delay = (unsigned int)parsed;
    return 0;
}

int main(int argc, char *argv[])
{
    unsigned int exit_delay = 0;

    if (argc == 3 && strcmp(argv[1], "--exit-after") == 0)
    {
        if (parse_exit_delay(argv[2], &exit_delay) == -1)
        {
            fprintf(stderr, "Usage: %s [--exit-after SECONDS]\n", argv[0]);
            return 2;
        }
    }
    else if (argc != 1)
    {
        fprintf(stderr, "Usage: %s [--exit-after SECONDS]\n", argv[0]);
        return 2;
    }

    printf("Worker baslatildi! PID: %ld\n", (long)getpid());

    if (argc == 3)
    {
        sleep(exit_delay);
        return 0;
    }

    while (1)
    {
        sleep(1);
    }

    return 0;
}
