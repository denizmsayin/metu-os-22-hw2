#include "hw2_output.h"

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

static struct timeval g_start_time;

void input_Parser(int argc, char *argv[], int *p_num_threads, int *p_num_iterations, int *p_num_elements, int *p_array_size)
{
    if (argc != 6)
    {
        fprintf(stderr, "Usage: %s <# threads> <# iterations> <# elements> <array size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    *p_num_threads = atoi(argv[1]);
    *p_num_iterations = atoi(argv[2]);
    *p_num_elements = atoi(argv[3]);
    *p_array_size = atoi(argv[4]);
}


void hw2_init_notifier(void)
{
    gettimeofday(&g_start_time, NULL);
}   

// This function is called by each thread to wait for the other threads to finish.
void hw2_wait_for_notification(void)
{
    struct timeval end_time;
    gettimeofday(&end_time, NULL);
    double elapsed_time = (end_time.tv_sec - g_start_time.tv_sec) + (end_time.tv_usec - g_start_time.tv_usec) / 1000000.0;
    printf("Elapsed time: %.6f\n", elapsed_time);
}
// This function is called by each thread to notify the other threads that it is done.
void hw2_notify_all(void)
{
    // this function is intentionally left blank
}

// This function is called by each thread to wait for the other threads to finish.
void hw2_wait_for_notification_mt(void)
{
    // TODO: Implement this function.
}
// This function is called by each thread to notify the other threads that it is done.
void hw2_notify_all_mt(void)
{
    // TODO: Implement this function.
}
// This function is called by each thread to wait for the other threads to finish.
void hw2_wait_for_notification_st(void)
{
    // TODO: Implement this function.
}
// This function is called by each thread to notify the other threads that it is done.
void hw2_notify_all_st(void)
{
    // TODO: Implement this function.
}
// This function is called by each thread to wait for the other threads to finish.
void hw2_wait_for_notification_mt_st(void)
{
    // TODO: Implement this function.
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

    if (PROPER_PRIVATE_CREATED <= action && action <= PROPER_PRIVATE_CONTINUED) {
        printf("G%u ", id);
    } else if (SNEAKY_SMOKER_CREATED <= action && action <= SNEAKY_SMOKER_STOPPED) {
        printf("S%u ", id);
    }

    switch (action) {
    case PROPER_PRIVATE_CREATED: puts("created."); break;
    case PROPER_PRIVATE_ARRIVED: printf("arrived at area (%u, %u).\n", i, j); break;
    case PROPER_PRIVATE_GATHERED: printf("gathered a cigbutt from (%u, %u).\n", i, j); break;
    case PROPER_PRIVATE_CLEARED: puts("cleared the current area and left."); break;
    case PROPER_PRIVATE_EXITED: puts("finished cleaning and exited."); break;
    case PROPER_PRIVATE_TOOK_BREAK: puts("took a break."); break;
    case PROPER_PRIVATE_STOPPED: puts("stopped as ordered."); break;
    case PROPER_PRIVATE_CONTINUED: puts("is continuing after a break."); break;
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
