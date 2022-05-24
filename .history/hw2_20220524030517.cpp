#include "hw2_header.h"
#include "hw2_output.c"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include "iostream"

#define NUM_THREADS 4
#define NUM_PROPERTIES 4
#define NUM_ACTIONS 4
#define NUM_SNEAKY_SMOKERS 2
#define NUM_PROPER_PRIVATES 2
#define NUM_ORDERS 2
#define NUM_BREAKS 2
#define NUM_CONTINUES 2
#define NUM_STOPS 2
#define NUM_ARRIVALS 2
#define NUM_GATHERINGS 2
#define NUM_CLEARINGS 2
#define NUM_EXITS 2

void input_parser(int argc, char *argv[]) {
  cin >> terrain->num_rows >> terrain->num_cols;
  terrain->cells = (terrain_cell **)malloc(sizeof(cell *) * terrain->num_rows);
  for (int i = 0; i < terrain->num_rows; i++) {
    terrain->cells[i] =
        (terrain_cell *)malloc(sizeof(cell) * terrain->num_cols);
    for (int j = 0; j < terrain->num_cols; j++) {
      cin >> terrain->cells[i][j].cig_count;
    }
  }
}
void semaphore(int num_threads, int num_iterations, int num_elements,
               int array_size) {
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
  int *array_copy9 = (int *)malloc(sizeof(int) * array_size);
}
void hw2_wait_for_notification(void) {
  struct timeval end_time;
  gettimeofday(&end_time, NULL);
  double elapsed_time = (end_time.tv_sec - g_start_time.tv_sec) +
                        (end_time.tv_usec - g_start_time.tv_usec) / 1000000.0;
  printf("Elapsed time: %.6f\n", elapsed_time);
}

int main(int argc, char *argv[]) {
  terrain *terrain = NULL;
  hw2_init_notifier();
  hw2_wait_for_notification();
  return 0;
}