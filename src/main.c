#include <stdio.h>
#include <unistd.h>
#include "../include/signal_manager.h"

int main(void)
{
    if (sinyalleriKur() == -1)
    {
        perror("Sinyal kurulumu basarisiz");
        return 1;
    }

    printf("Watchdog baslatildi! PID: %ld\n", (long)getpid());

    while (!kapatmaIstendiMi())
    {
        if (raporIstendiMi())
            printf("SIGUSR1 alindi: Durum raporu istendi.\n");

        sleep(1);
    }

    printf("Watchdog guvenli sekilde kapatildi.\n");

    return 0;
}