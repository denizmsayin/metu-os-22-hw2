#include "hw2_output.cpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

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

void semaphore(int num_threads, int num_iterations, int num_elements, int array_size)
{
    int i, j;
    int *array = (int *)malloc(sizeof(int) * array_size);
    int *array_copy = (int *)malloc(sizeof(int) * array_size);
    int *array_copy2 = (int *)malloc(sizeof(int) * array_size);
    int *array_copy3 = (int *)malloc(sizeof(int) * array_size);
    int *array_copy4 = (int *)malloc(sizeof(int) * array_size);
    int *array_copy5 = (int *)malloc(sizeof(int) * array_size);
    int *array_copy6 = (int *)malloc(sizeof(int) * array_size);
    int *array_copy7 = (int *)malloc(sizeof(int) * array_size);
    int *array_copy8 = (int *)malloc(sizeof(int) * array_size);
    int *array_copy9 = (int *)malloc(sizeof(int) * array_size);}
void hw2_init_notifier(void)
{
    gettimeofday(&g_start_time, NULL);
}
void hw2_wait_for_notification(void)
{
    struct timeval end_time;
    gettimeofday(&end_time, NULL);
    double elapsed_time = (end_time.tv_sec - g_start_time.tv_sec) + (end_time.tv_usec - g_start_time.tv_usec) / 1000000.0;
    printf("Elapsed time: %.6f\n", elapsed_time);
}


int main(int argc, char *argv[])
{
    int num_threads, num_iterations, num_elements, array_size;
    input_Parser(argc, argv, &num_threads, &num_iterations, &num_elements, &array_size);
    semaphore(num_threads, num_iterations, num_elements, array_size);
    hw2_init_notifier();
    hw2_wait_for_notification();
    return 0;
}