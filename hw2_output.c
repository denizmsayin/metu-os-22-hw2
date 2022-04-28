#include "hw2_output.h"

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

static struct timeval g_start_time;

void hw2_init_notifier(void)
{
    gettimeofday(&g_start_time, NULL);
}

static long get_timestamp(void)
{
    struct timeval cur_time;
    gettimeofday(&cur_time, NULL);
    return (cur_time.tv_sec - g_start_time.tv_sec) * 1000000 
           + (cur_time.tv_usec - g_start_time.tv_usec);
}

void hw2_notify(enum hw2_actions action, unsigned id, unsigned i, unsigned j)
{
    static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

    if (g_start_time.tv_sec == 0 && g_start_time.tv_usec == 0) {
        fprintf(stderr, "You must call hw2_init_notifier() at the start of your main()!");
        exit(EXIT_FAILURE);
    }
    
    pthread_mutex_lock(&mut);

    printf("t: %9ld | ", get_timestamp());

    printf("(T%lu) ", pthread_self());

    if (GATHERER_CREATED <= action && action <= GATHERER_CONTINUED) {
        printf("G%u ", id);
    } else if (SNEAKY_SMOKER_CREATED <= action && action <= SNEAKY_SMOKER_STOPPED) {
        printf("S%u ", id);
    }

    switch (action) {
    case GATHERER_CREATED: puts("created."); break;
    case GATHERER_ARRIVED: printf("arrived at area (%u, %u).\n", i, j); break;
    case GATHERER_GATHERED: printf("gathered a cigbutt from (%u, %u).\n", i, j); break;
    case GATHERER_CLEARED: puts("cleared the current area and left."); break;
    case GATHERER_EXITED: puts("finished cleaning and exited."); break;
    case GATHERER_TOOK_BREAK: puts("took a break."); break;
    case GATHERER_STOPPED: puts("stopped as ordered."); break;
    case GATHERER_CONTINUED: puts("is continuing after a break."); break;
    case ORDER_BREAK: puts("BREAK!"); break;
    case ORDER_CONTINUE: puts("CONTINUE!"); break;
    case ORDER_STOP: puts("STOP!"); break;
    case SNEAKY_SMOKER_CREATED: puts("created."); break;
    case SNEAKY_SMOKER_ARRIVED: printf("arrived at cell (%u, %u).\n", i, j); break;
    case SNEAKY_SMOKER_FLICKED: printf("flicked a cigbutt towards (%u, %u).\n", i, j); break;
    case SNEAKY_SMOKER_LEFT: puts("had enough of the current area and left."); break;
    case SNEAKY_SMOKER_EXITED: puts("finished smoking and exited."); break;
    case SNEAKY_SMOKER_STOPPED: printf("stopped as ordered."); break;
    }

    pthread_mutex_unlock(&mut);
}
