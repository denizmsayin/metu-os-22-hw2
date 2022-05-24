#include "hw2_header.h"
#include "hw2_output.c"
#include "iostream"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

// #define NUM_THREADS 4
// #define NUM_PROPERTIES 4
// #define NUM_ACTIONS 4
// #define NUM_SNEAKY_SMOKERS 2
// #define NUM_PROPER_PRIVATES 2
// #define NUM_ORDERS 2
// #define NUM_BREAKS 2
// #define NUM_CONTINUES 2
// #define NUM_STOPS 2
// #define NUM_ARRIVALS 2
// #define NUM_GATHERINGS 2
// #define NUM_CLEARINGS 2
// #define NUM_EXITS 2
using namespace std;

void input_parser(terrain *t) {
    // Initialize the terrain
    cin >> t->num_rows >> t->num_cols;
    // Initialize the cells
    t->cells = *new terrain_cell*[t->num_rows];
    for (int i = 0; i < t->num_rows; i++) {
        t->cells[i] = *new terrain_cell[t->num_cols];
        for (int j = 0; j < t->num_cols; j++) {
            cin >> t->cells[i][j].cig_count;
            t->cells[i][j].smoker = NULL;
            t->cells[i][j].proper_private = NULL;
            t->cells[i][j].sneaky_smoker = NULL;
        }
    }
    // Initialize the proper privates
    cin >> t->num_proper_privates;
    t->proper_privates = *new proper_private*[t->num_proper_privates];
    for (int i = 0; i < t->num_proper_privates; i++) {
        t->proper_privates[i] = *new proper_private;
        cin >> t->proper_privates[i].id >> t->proper_privates[i].area_i >> t->proper_privates[i].area_j >> t->proper_privates[i].time_to_gather >> t->proper_privates[i].areas_to_gather;
        for(int j = 0; j < t->proper_privates[i].areas_to_gather; j++) {
            cin >> t->proper_privates[i].areas[j].i >> t->proper_privates[i].areas[j].j;
        }
     

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
