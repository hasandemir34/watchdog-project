#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

static volatile sig_atomic_t kontrolZamani = 0;

static void alarmYakala(int sig)
{
    (void)sig;
    kontrolZamani = 1;
}

int main(void)
{
    struct sigaction sa = {0};

    sa.sa_handler = alarmYakala;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGALRM, &sa, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }

    struct itimerval timer = {0};

    timer.it_value.tv_sec = 2;
    timer.it_interval.tv_sec = 2;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
    {
        perror("setitimer");
        return 1;
    }

    int sayac = 0;

    while (sayac < 3)
    {
        if (kontrolZamani)
        {
            kontrolZamani = 0;
            sayac++;

            printf("Periodic child check: %d\n", sayac);
        }
        else
        {
            pause();
        }
    }

    return 0;
}
