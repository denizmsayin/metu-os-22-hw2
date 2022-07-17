#ifndef HW2_SYNC_MECHANISM_H_
#define HW2_SYNC_MECHANISM_H_

#include <pthread.h>

#include "area.h"

enum order_type {
    BREAK,
    CONTINUE,
    STOP
};

// The single global order mechanism implementation will be hidden inside the 
// .c file for simplicity. hw2.c only interacts with it using these functions.

void sm_init(size_t n_initial_pps, size_t n_initial_sss);
void sm_destroy(void);

// Area locking/unlocking, will return 1 if got an order while
// waiting for the area to unlock. 0 if area is locked successfully.
// Of course, will be holding a read-lock if returning with an order.
int sm_pp_lock_area(struct area, pthread_cond_t *);
void sm_pp_unlock_area(struct area);
int sm_ss_lock_area(struct area, pthread_cond_t *);
void sm_ss_unlock_area(struct area);

// Soldiers (proper private and sneaky smoker) read-locking
void sm_soldier_lock(void);
void sm_soldier_unlock(void);
enum order_type sm_active_order(void);

// Order notifications
void sm_order_notify(enum order_type);

// For the commander to send an order, will write-lock 
// and block further soldier_lock attempts
void sm_commander_order(enum order_type);

// Proper privates should call this to wait between cigbutt gatherings.
// Make sure to be holding the soldier lock! Same for sneaky smokers.
void sm_pp_cigrest(unsigned long sleep_time);
void sm_ss_cigrest(unsigned long sleep_time);

// Proper privates should call this to wait for an order during the break
void sm_pp_await_order_in_break(void);

// To handle order completion
void sm_pp_register_leaving(void);
void sm_ss_register_leaving(void);
void sm_pp_register_order_received(void);
void sm_commander_await_orders_received(void);

#endif
