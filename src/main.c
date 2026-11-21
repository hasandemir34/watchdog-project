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

    while (!kapatmaIstendiMi())
    {
        if (kontrolIstendiMi() && !kapatmaIstendiMi())
        {
            int oncekiRestart[WORKER_COUNT];
            int oncekiAktif[WORKER_COUNT];

            for (int i = 0; i < WORKER_COUNT; i++)
            {
                oncekiRestart[i] = children[i].restartCount;
                oncekiAktif[i] = children[i].aktif;
            }

            int sonuc = childProcessleriIzle(children, WORKER_COUNT);

            if (sonuc == CHILD_PROCESS_ERROR)
            {
                perror("Worker izleme hatasi");
                logger_log(LOG_ERROR, "Worker izleme hatasi");
                hata = 1;
                break;
            }

            for (int i = 0; i < WORKER_COUNT; i++)
            {
                if (children[i].restartCount > oncekiRestart[i])
                {
                    logger_log(LOG_WARN,
                               "Worker %d restart %d/%d PID=%ld",
                               i + 1,
                               children[i].restartCount,
                               CHILD_PROCESS_MAX_RESTARTS,
                               (long)children[i].pid);
                }

                if (oncekiAktif[i] && !children[i].aktif)
                {
                    logger_log(LOG_WARN,
                               "Worker %d durdu. Limit=%d",
                               i + 1,
                               CHILD_PROCESS_MAX_RESTARTS);
                }
            }
        }

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

        struct timespec bekleme = {0, 100000000L};
        nanosleep(&bekleme, NULL);
    }

    struct itimerval durdur = {0};
    setitimer(ITIMER_REAL, &durdur, NULL);

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