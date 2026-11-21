#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "../include/signal_manager.h"
#include "../include/process_manager.h"

#define WORKER_COUNT 2

int main(void)
{
    if (sinyalleriKur() == -1)
    {
        perror("Sinyal kurulumu basarisiz");
        return 1;
    }

    ChildProcess children[WORKER_COUNT] = {0};

    printf("Watchdog baslatildi! PID: %ld\n", (long)getpid());

    // Iki worker baslat
    for (int i = 0; i < WORKER_COUNT; i++)
    {
        if (childProcessBaslat(&children[i], "./worker") == CHILD_PROCESS_ERROR)
        {
            perror("Worker baslatilamadi");
            childProcessleriSonlandir(children, WORKER_COUNT);
            return 1;
        }

        printf("Worker %d baslatildi! PID: %ld\n",
               i + 1, (long)children[i].pid);
    }

    // Her saniye SIGALRM uret
    struct itimerval timer = {0};

    timer.it_value.tv_sec = 1;
    timer.it_interval.tv_sec = 1;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
    {
        perror("Timer baslatilamadi");
        childProcessleriSonlandir(children, WORKER_COUNT);
        return 1;
    }

    int hata = 0;

    // Ana kontrol dongusu
    while (!kapatmaIstendiMi())
    {
        // SIGCHLD veya SIGALRM geldiginde kontrol et
        if (kontrolIstendiMi() && !kapatmaIstendiMi())
        {
            int sonuc = childProcessleriIzle(children, WORKER_COUNT);

            if (sonuc == CHILD_PROCESS_ERROR)
            {
                perror("Worker izleme hatasi");
                hata = 1;
                break;
            }
        }

        // SIGUSR1 durum raporu
        if (raporIstendiMi())
        {
            for (int i = 0; i < WORKER_COUNT; i++)
            {
                printf("Worker %d - PID: %ld - Restart: %d - Durum: %s\n",
                       i + 1,
                       (long)children[i].pid,
                       children[i].restartCount,
                       children[i].aktif ? "RUNNING" : "STOPPED");
            }
        }

        // Sinyalleri kisa araliklarla kontrol et
        struct timespec bekleme = {0, 100000000L};
        nanosleep(&bekleme, NULL);
    }

    // Timer durdur
    struct itimerval durdur = {0};
    setitimer(ITIMER_REAL, &durdur, NULL);

    // Worker'lari guvenli kapat
    if (childProcessleriSonlandir(children, WORKER_COUNT) == CHILD_PROCESS_ERROR)
    {
        perror("Worker kapatma hatasi");
        return 1;
    }

    printf("Watchdog guvenli sekilde kapatildi.\n");

    return hata;
}