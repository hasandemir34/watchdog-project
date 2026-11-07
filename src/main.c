#include <stdio.h>
#include <unistd.h>
#include "../include/signal_manager.h"

int main(void)
{
    sinyalleriKur();

    printf("Watchdog baslatildi!\n");

    while (!kapatmaIstendiMi())
    {
        printf("Watchdog calisiyor...\n");
        sleep(2);
    }

    printf("Watchdog guvenli sekilde kapatildi.\n");

    return 0;
}