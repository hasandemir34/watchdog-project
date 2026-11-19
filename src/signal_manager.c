#include <signal.h>
#include <stddef.h>
#include "../include/signal_manager.h"

static volatile sig_atomic_t kapat = 0;
static volatile sig_atomic_t rapor = 0;

static void sinyalYakala(int sig)
{
    if (sig == SIGINT || sig == SIGTERM)
        kapat = 1;
    else if (sig == SIGUSR1)
        rapor = 1;
}

int sinyalleriKur(void)
{
    struct sigaction sa = {0};

    sa.sa_handler = sinyalYakala;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1)
        return -1;

    if (sigaction(SIGTERM, &sa, NULL) == -1)
        return -1;

    if (sigaction(SIGUSR1, &sa, NULL) == -1)
        return -1;

    return 0;
}

int kapatmaIstendiMi(void)
{
    return kapat != 0;
}

int raporIstendiMi(void)
{
    if (rapor)
    {
        rapor = 0;
        return 1;
    }

    return 0;
}