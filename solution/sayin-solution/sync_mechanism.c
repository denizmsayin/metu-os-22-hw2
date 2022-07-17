#include "sync_mechanism.h"

#include <pthread.h>
#include <errno.h>

#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "hw2_output.h"
#include "error.h"
#include "area_containers.h"
#include "time_utils.h"

struct {
    // Lock for the whole thing to allow only one
    // person to change things at a time
    pthread_mutex_t mutex;
    
    // Synchronizing the area
    area_container_t pp_active;
    area_keyed_map_t pp_waiting;
    area_container_t ss_active;
    area_keyed_map_t ss_waiting;

    // Synchronizing orders
    int commander_in;
    size_t n_soldiers;
    pthread_cond_t no_soldiers;
    pthread_cond_t order_issued;
    enum order_type active_order;

    // For orders in quick succession
    size_t n_active_pps; 
    size_t n_active_sss;
    size_t n_received_pps;
    pthread_cond_t everyone_received_order;
} sm; // <- Stands for sync_mechanism




/* ----- INITIALIZATION & LOCKING THE MUTEX ----- */

void sm_init(size_t n_initial_pps, size_t n_initial_sss)
{
    PT_CHECKED(pthread_mutex_init(&sm.mutex, NULL), "init sm mutex");
    
    ac_init(&sm.pp_active);
    akm_init(&sm.pp_waiting);
    ac_init(&sm.ss_active);
    akm_init(&sm.ss_waiting);
    
    PT_CHECKED(pthread_cond_init(&sm.no_soldiers, NULL), "init sm cv");
    PT_CHECKED(pthread_cond_init(&sm.order_issued, NULL), "init sm cv");
    sm.active_order = CONTINUE; // pp's are working initially
    sm.n_soldiers = 0;
    sm.commander_in = 0;

    sm.n_active_pps = n_initial_pps;
    sm.n_active_sss = n_initial_sss;
    sm.n_received_pps = 0;
    PT_CHECKED(pthread_cond_init(&sm.everyone_received_order, NULL), "init sm cv");
}

void sm_destroy(void)
{
    ac_destroy(&sm.pp_active);
    akm_destroy(&sm.pp_waiting);
    ac_destroy(&sm.ss_active);
    akm_destroy(&sm.ss_waiting);

    PT_CHECKED(pthread_mutex_destroy(&sm.mutex), "destroy sm mutex");
    PT_CHECKED(pthread_cond_destroy(&sm.order_issued), "destroy sm cv");
    PT_CHECKED(pthread_cond_destroy(&sm.order_issued), "destroy sm cv");
    PT_CHECKED(pthread_cond_destroy(&sm.everyone_received_order), "destroy sm cv");
}

static inline void sm_lock(void)
{
    PT_CHECKED(pthread_mutex_lock(&sm.mutex), "locking sm mutex");
}

static inline void sm_unlock(void)
{
    PT_CHECKED(pthread_mutex_unlock(&sm.mutex), "unlocking sm mutex");
}




/* ----- AREA LOCKING AND UNLOCKING ----- */

typedef struct area *(*area_checker_fn)(const area_container_t *, struct area);
typedef int (*order_checker_fn)(enum order_type);

static void soldier_lock_having_mutex(void);

// First of all, the area stuff:
// An abstract area locking function, since pp and ss are very similar
static int abstract_lock_area(
        area_checker_fn sneaky_smoker_checker, // See in definition
        order_checker_fn order_is_unimportant, // Same, see the definition comments
        area_container_t *active_container, // For inserting active areas
        area_keyed_map_t *wait_map, // For inserting wait area-cv pairs
        struct area a, // Area to lock
        pthread_cond_t *cv) // Cv to sleep on if the area can't be locked now
{
    sm_lock();

    // For both pp's and ss's, there should be no intersection with the active pp's
    // Sneaky smokers are different though. pp's should have no intersection, but
    // other ss's should not have the exact same area. We leave that as abstract.
    if (ac_find_intersection(&sm.pp_active, a) ||
        sneaky_smoker_checker(&sm.ss_active, a))
    {
        struct area *actptr;

        // Got an intersection, need to insert into the waiting list with its cv
        akm_insert(wait_map, a, cv);

        // Wait until no intersection. do-while to avoid an extra check at the start.
        // We also need to handle the reception of orders here. Decoupling orders
        // from areas is fine in part I & II, but here we may have an issue when
        // mixing in smokers. If a proper private is stuck waiting here for a sneaky smoker
        // and a break order comes in, he still needs to be able to react to the
        // order and won't be able to if we don't check for it since the sneaky
        // smoker will not care for the break order and won't unlock the area.
        do {
            PT_CHECKED(pthread_cond_wait(cv, &sm.mutex), "wait on cv for area");
        } while (order_is_unimportant(sm.active_order) && 
                 (ac_find_intersection(&sm.pp_active, a) || 
                  sneaky_smoker_checker(&sm.ss_active, a)));

        if (!order_is_unimportant(sm.active_order)) {
            // Got an order, time to leave with a read-lock
            soldier_lock_having_mutex();
            return 1;
        }

        // Alright, we woke up to no more intersections. Need to remove ourselves
        // from the waiting list. Since our position may have changed (it's a simple
        // array after all), we have to find it. Can use the CV pointer since it
        // is unique.
        actptr = akm_find_by_vptr(wait_map, cv);
        assert (actptr != NULL); // Program logic says we should find it no matter what
        akm_remove(wait_map, actptr);
    }

    // At this point, we have no more intersections and are active
    ac_insert(active_container, a);

    // No need to hold any locks
    sm_unlock();

    return 0;
}


static int order_is_continue(enum order_type order) { return order == CONTINUE; }

// a -> area to lock
// cv -> self-owned cv for thread to sleep on if it cannot currently lock
int sm_pp_lock_area(struct area a, pthread_cond_t *cv)
{
    return abstract_lock_area(ac_find_intersection,    // Intersection with sneaky smokers
                              order_is_continue,       // Ignore continue, waiting for area
                              &sm.pp_active,           // Active pp area list
                              &sm.pp_waiting,          // Waiting pp area-cv map
                              a, cv);
}


static int order_is_not_stop(enum order_type order) { return order != STOP; }

int sm_ss_lock_area(struct area a, pthread_cond_t *cv)
{
    return abstract_lock_area(ac_find_by_area,         // Exact equality with other ss
                              order_is_not_stop,       // Ignore orders that are not stop
                              &sm.ss_active,           // Active ss list
                              &sm.ss_waiting,          // Waiting ss area-cv map
                              a, cv);
}

static void wake_intersections(area_keyed_map_t *m, struct area a)
{
    arraylist_areaptr_t to_wake = akm_find_intersections(m, a);
    for (struct area **itr = ALIST_BEGIN(&to_wake); itr != ALIST_END(&to_wake); itr++) {
        pthread_cond_t *cv = akm_get_value(m, *itr);
        pthread_cond_signal(cv);
    }
    arraylist_areaptr_destroy(&to_wake);
}

void sm_pp_unlock_area(struct area a)
{
    struct area *to_rm;

    sm_lock();

    // Remove the area from the active list
    to_rm = ac_find_by_area(&sm.pp_active, a);
    assert (to_rm); // Must be already in active list if there is no bug
    ac_remove(&sm.pp_active, to_rm);

    // Now, need to wake waiters that have intersections with this active area.
    // The waiters are not guaranteed to continue though. They may have intersections
    // with other areas, or someone else that has an intersection may come in
    // the meantime. Still, it's worth waking up for!

    // We should wake-up everyone though, since a scenario where 2 wakers have
    // other intersections while one does not is possible. Could use a chaining
    // kind of approach when one waker wakes the next one up if he cannot lock,
    // that would be much more efficient. 
    // But that requires further synchronization which is painful!
    // It hopefully shouldn't be much of a problem since there won't be too many
    // intersecting with a single area.
    // Wake both proper privates and sneaky smokers.
    wake_intersections(&sm.pp_waiting, a);
    wake_intersections(&sm.ss_waiting, a);

    sm_unlock();
}

void sm_ss_unlock_area(struct area a)
{
    struct area *to_rm;
    pthread_cond_t *same_ss;

    sm_lock();

    // As with the pp's, remove the area from the active list
    to_rm = ac_find_by_area(&sm.ss_active, a);
    assert (to_rm); 
    ac_remove(&sm.ss_active, to_rm);

    // Wake all intersecting proper privates, as with pp's
    wake_intersections(&sm.pp_waiting, a);
    // But waking only one exactly same sneaky smoker is enough,
    // no need to cause a pointless thundering herd by waking all
    same_ss = akm_find_by_area(&sm.ss_waiting, a);
    if (same_ss)
        pthread_cond_signal(same_ss);

    sm_unlock();
}



// Make sure to call while holding the read-lock to avoid inconsistencies
void sm_pp_register_leaving(void)
{
    sm_lock();
    sm.n_active_pps--;
    if (sm.n_active_pps == 0 && sm.n_active_sss == 0) // No one left!
        pthread_cond_signal(&sm.everyone_received_order);

    // There is an extra sneaky case that is easy to miss here.
    // When there are many pp's, it is possible for a pp to respond
    // to an order, then do stuff and exit before everyone else has
    // responded. If it does not decrement n_responded_pps, then the
    // number of active pps will drop below responded pps and a deadlock
    // will happen. Since we're sure that the current leaving thread has
    // responded, we can safely decrement its response count.
    if (sm.n_received_pps > 0)
        sm.n_received_pps--;
    
    sm_unlock();
}

void sm_ss_register_leaving(void)
{
    sm_lock();
    sm.n_active_sss--;
    if (sm.n_active_pps == 0 && sm.n_active_sss == 0) // No one left!
        pthread_cond_signal(&sm.everyone_received_order);
    sm_unlock();
}

void sm_pp_register_order_received(void)
{
    sm_lock();
    sm.n_received_pps++;
    if (sm.n_received_pps == sm.n_active_pps)
        pthread_cond_signal(&sm.everyone_received_order);
    sm_unlock();
}

void sm_commander_await_orders_received(void)
{
    sm_lock();
    if (sm.active_order == STOP)
        while (sm.n_active_pps != 0 || sm.n_active_sss != 0)
            pthread_cond_wait(&sm.everyone_received_order, &sm.mutex);
    else
        while (sm.n_received_pps != sm.n_active_pps)
            pthread_cond_wait(&sm.everyone_received_order, &sm.mutex);
    sm.n_received_pps = 0; // reset for next order
    sm_unlock();
}

// Make sure to hold at least a read-lock when calling this
enum order_type sm_active_order(void)
{
    return sm.active_order;
}

// The lock-unlock process follows a writer-priority r/w lock.
// The commander will wait for existing soldiers to unlock,
// but any new soldier coming to lock will have to wait
// for the commander to finish issuing his order first.

static void soldier_lock_having_mutex(void)
{
    while (sm.commander_in)
        PT_CHECKED(pthread_cond_wait(&sm.order_issued, &sm.mutex), "soldier waiting");

    sm.n_soldiers++;

    sm_unlock();
}

void sm_soldier_lock(void)
{
    sm_lock();
    soldier_lock_having_mutex();
}

// Perform steps to unlock the read-lock, but still hold the mutex
static void soldier_unlock_keep_mutex(void)
{
    sm_lock();

    sm.n_soldiers--;

    if (sm.n_soldiers == 0 && sm.commander_in)
        pthread_cond_signal(&sm.no_soldiers);
}

void sm_soldier_unlock(void)
{
    soldier_unlock_keep_mutex();
    sm_unlock();
}

void sm_order_notify(enum order_type order)
{
    switch (order) {
    case BREAK:     hw2_notify(ORDER_BREAK, 0, 0, 0); break;
    case CONTINUE:  hw2_notify(ORDER_CONTINUE, 0, 0, 0); break;
    case STOP:      hw2_notify(ORDER_STOP, 0, 0, 0); break;
    default:        error_rt("Unexpected order enum value %d", (int) order);
    }
}

static void wake_cvs(area_keyed_map_t *m)
{
    // Wake all condition variables in a void* arraylist (akm.values)
    for (void **itr = AKM_VBEGIN(m); itr != AKM_VEND(m); itr++) {
        pthread_cond_t *cv = *itr;
        pthread_cond_signal(cv);
    }
}

void sm_commander_order(enum order_type order)
{
    sm_lock();

    sm.commander_in = 1; // Set flag to stop new soldiers

    while (sm.n_soldiers > 0) // Wait for existing soldiers to finish
        PT_CHECKED(pthread_cond_wait(&sm.no_soldiers, &sm.mutex), "cmdr waiting");

    sm.active_order = order; // Give the order

    // Leave and wake soldiers
    sm.commander_in = 0;

    // This one is for waking both proper privates and smokers who are cigresting
    pthread_cond_broadcast(&sm.order_issued);

    // We also need to wake those who are waiting for an area to be unlocked,
    // since it may not be unlocked in case the gatherers are the ones receiving the orders.
    wake_cvs(&sm.pp_waiting);
    if (order == STOP) // Also wake waiting smokers if the order is STOP
        wake_cvs(&sm.ss_waiting);

    sm_order_notify(order);

    sm_unlock();
}

static void cigrest(int (*order_is_unimportant)(enum order_type), unsigned long sleep_time)
{
    struct timespec wakeup_time;

    assert (sm.n_soldiers > 0); // A very weak assertion to ensure the soldier locked

    calc_delayed_abstime(sleep_time, &wakeup_time);

    // Possibly wait for an order. This part is a bit complicated!
    // Since we want to unlock & re-lock the read-lock, and not just
    // some mutex. We perform steps to unlock
    // the read-lock, but still hold the mutex to wait on the commander CV.
    soldier_unlock_keep_mutex();
    while (order_is_unimportant(sm.active_order)) { // Different for pp's and ss's
        // This loop is resistant to spurious wake-ups since the time-out is
        // specified in absolute time. If there are no orders to wake up to but 
        // the soldier still wakes up, it will sleep again until the target time. 
        int ret = pthread_cond_timedwait(&sm.order_issued, &sm.mutex, &wakeup_time);
        if (ret == ETIMEDOUT) // Break the loop if timed out
            break;
        else if (ret != 0) // Another error may have ocurred, best check
            error_wno(ret, "pthread_cond_timedwait failed in soldier_sleep");
    }

    // Whether an order has arrived or not, we should now re-acquire the read-lock
    // Remember we're already holding the mutex after waking up from cond_wait!
    soldier_lock_having_mutex();
}

// Proper privates should not wake up from their cigrest unles the order is stop || break
void sm_pp_cigrest(unsigned long sleep_time) { cigrest(order_is_continue, sleep_time); }

// Sneaky smokers should not wake up from their cigrest unless the order is stop
void sm_ss_cigrest(unsigned long sleep_time) { cigrest(order_is_not_stop, sleep_time); }

void sm_pp_await_order_in_break(void)
{
    // Similar to cigrest, wait for a useful order (not break again!)
    // while taking steps to properly handle the read-lock
    soldier_unlock_keep_mutex();
    while (sm.active_order == BREAK)
        PT_CHECKED(pthread_cond_wait(&sm.order_issued, &sm.mutex), "private@break cond_wait");
    soldier_lock_having_mutex();
}

