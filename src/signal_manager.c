#include <signal.h>
#include <stddef.h>
#include "../include/signal_manager.h"

static volatile sig_atomic_t kapat = 0;

static void sinyalYakala(int sig)
{
    if (sig == SIGINT)
        kapat = 1;
}

void sinyalleriKur(void)
{
    struct sigaction sa = {0};

    sa.sa_handler = sinyalYakala;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);
}

int kapatmaIstendiMi(void)
{
    return kapat != 0;
}