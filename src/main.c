#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "signal_manager.h"
#include "process_manager.h"
#include "logger.h"

#define WORKER_COUNT 2

int main(void)
{
    if (sinyalleriKur() == -1)
    {
        perror("Sinyal kurulumu basarisiz");
        return 1;
    }

    if (logger_init("watchdog.log") == -1)
    {
        perror("Logger baslatilamadi");
        return 1;
    }

    ChildProcess children[WORKER_COUNT] = {0};
    const char *names[WORKER_COUNT] = {"worker-1", "worker-2"};

    printf("Watchdog baslatildi! PID: %ld\n", (long)getpid());
    logger_log(LOG_INFO, "Watchdog baslatildi PID=%ld", (long)getpid());

    // Iki worker baslat
    for (int i = 0; i < WORKER_COUNT; i++)
    {
        if (childProcessBaslat(&children[i], "./workers/worker") == CHILD_PROCESS_ERROR)
        {
            perror("Worker baslatilamadi");
            logger_log(LOG_ERROR, "Worker %d baslatilamadi", i + 1);

            childProcessleriSonlandir(children, WORKER_COUNT);
            logger_close();
            return 1;
        }

        printf("Worker %d baslatildi! PID: %ld\n",
               i + 1, (long)children[i].pid);

        logger_log(LOG_INFO, "Worker %d baslatildi PID=%ld",
                   i + 1, (long)children[i].pid);
    }

    // Her saniye SIGALRM uret
    struct itimerval timer = {0};

    timer.it_value.tv_sec = 1;
    timer.it_interval.tv_sec = 1;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
    {
        perror("Timer baslatilamadi");
        logger_log(LOG_ERROR, "Timer baslatilamadi");

        childProcessleriSonlandir(children, WORKER_COUNT);
        logger_close();
        return 1;
    }

    int hata = 0;

    // Ana kontrol dongusu
    while (!kapatmaIstendiMi())
    {
        if (kontrolIstendiMi() && !kapatmaIstendiMi())
        {
            // Her worker'i ayri kontrol et
            for (int i = 0; i < WORKER_COUNT; i++)
            {
                if (kapatmaIstendiMi())
                    break;

                if (!children[i].aktif)
                    continue;

                pid_t eskiPid = children[i].pid;

                int sonuc = childProcessIzleVeYenidenBaslat(&children[i]);

                if (sonuc == CHILD_PROCESS_ERROR)
                {
                    perror("Worker izleme hatasi");
                    logger_log(LOG_ERROR,
                               "Worker %d izleme hatasi", i + 1);

                    hata = 1;
                    break;
                }

                if (sonuc == CHILD_PROCESS_EXITED)
                {
                    logger_log(LOG_INFO,
                               "Worker %d normal sonlandi PID=%ld",
                               i + 1, (long)eskiPid);
                }
                else if (sonuc == CHILD_PROCESS_RESTARTED)
                {
                    logger_log(LOG_WARN,
                               "Worker %d beklenmedik sonlandi PID=%ld",
                               i + 1, (long)eskiPid);

                    logger_log(LOG_WARN,
                               "Worker %d restart %d/%d Yeni PID=%ld",
                               i + 1,
                               children[i].restartCount,
                               CHILD_PROCESS_MAX_RESTARTS,
                               (long)children[i].pid);
                }
                else if (sonuc == CHILD_PROCESS_RESTART_LIMIT)
                {
                    logger_log(LOG_WARN,
                               "Worker %d sonlandi PID=%ld. Restart limiti doldu.",
                               i + 1, (long)eskiPid);
                }
            }

            if (hata)
                break;
        }

        // SIGUSR1 durum raporu
        if (raporIstendiMi())
        {
            ProcessStatusSnapshot snapshots[WORKER_COUNT];

            for (int i = 0; i < WORKER_COUNT; i++)
            {
                snapshots[i].pid = children[i].pid;
                snapshots[i].name = names[i];

                snapshots[i].state =
                    children[i].aktif ? "RUNNING" : "STOPPED";

                snapshots[i].restart_count = children[i].restartCount;

                printf("Worker %d - PID: %ld - Restart: %d - Durum: %s\n",
                       i + 1,
                       (long)children[i].pid,
                       children[i].restartCount,
                       snapshots[i].state);
            }

            logger_log_status_report(snapshots, WORKER_COUNT);
        }

        // Kisa araliklarla sinyalleri kontrol et
        struct timespec bekleme = {0, 100000000L};
        nanosleep(&bekleme, NULL);
    }

    // Timer'i durdur
    struct itimerval durdur = {0};
    setitimer(ITIMER_REAL, &durdur, NULL);

    // Worker'lari guvenli kapat
    if (childProcessleriSonlandir(children, WORKER_COUNT) == CHILD_PROCESS_ERROR)
    {
        perror("Worker kapatma hatasi");
        logger_log(LOG_ERROR, "Worker kapatma hatasi");
        logger_close();
        return 1;
    }

    logger_log(LOG_INFO, "Watchdog guvenli sekilde kapatildi");
    logger_close();

    printf("Watchdog guvenli sekilde kapatildi.\n");

    return hata;
}
