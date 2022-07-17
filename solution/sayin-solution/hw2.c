#include "hw2_output.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include "generic_rwa.h"
#include "area_containers.h"
#include "error.h"
#include "sync_mechanism.h"
#include "time_utils.h"

// 
// x y -- grid size
// 1 0 1 2 -- butt counts
// 3 2 1 0 
// n -- number of proper_privates
// id xdim ydim tpause ngathers
// gx gy
// ...
// id xdim ydim tpause ngathers
// gx gy
// ...
// ...

// Global grid
typedef struct {
    size_t rows, cols;
    long **cells; // using atomic longs would be better IRL if possible
    pthread_mutex_t **locks;
} grid_t;

static grid_t ggrid;

static void read_grid(grid_t *g)
{
    size_t rows, cols;
    scanf(" %lu %lu", &rows, &cols);
    g->rows = rows;
    g->cols = cols;
    g->cells = alloc_2d(rows, cols, sizeof(g->cells[0][0]));
    g->locks = alloc_2d(rows, cols, sizeof(g->locks[0][0]));
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            scanf (" %ld", &g->cells[i][j]);
            PT_CHECKED(pthread_mutex_init(&g->locks[i][j], NULL), "init cell lock");
        }
    }
}

static void free_grid(grid_t *g)
{
    for (size_t i = 0; i < g->rows; i++)
        for (size_t j = 0; j < g->cols; j++)
            PT_CHECKED(pthread_mutex_destroy(&g->locks[i][j]), "destroy cell lock");
    free_2d(g->locks, g->rows, g->cols);
    free_2d(g->cells, g->rows, g->cols);
}

// PROPER PRIVATE STUFF

struct pair {
    int i, j;
};

struct proper_private {
    unsigned long span_i, span_j, id, pause_msecs;
    size_t n;
    struct pair *corners;
    pthread_cond_t cond;
};

static void read_pair(void *pp)
{
    struct pair *p = pp;
    scanf(" %d %d", &p->i, &p->j);
}

static void read_proper_private(void *pp)
{
    struct proper_private *p = pp;
    scanf(" %lu %lu %lu %lu", &p->id, &p->span_i, &p->span_j, &p->pause_msecs);
    p->corners = read_array_wsize(read_pair, sizeof(p->corners[0]), &p->n);
    PT_CHECKED(pthread_cond_init(&p->cond, NULL), "init proper private cond");
}

static void free_proper_private(struct proper_private *g)
{
    free(g->corners);
}

static void soldier_relock(void)
{
    // Why the seemingly pointless lock-unlock? Since the order mechanism
    // has commander priority, this gives the commander a chance to give
    // an order in case he is waiting.
    sm_soldier_unlock();
    sm_soldier_lock();
}

static void proper_private_stop(struct proper_private *p, const struct area *current_area)
{
    hw2_notify(PROPER_PRIVATE_STOPPED, p->id, 0, 0);
    if (current_area)
        sm_pp_unlock_area(*current_area); // Unlock after notifying to prevent smokers at first
    sm_pp_register_leaving();
    sm_soldier_unlock();
    pthread_exit(NULL);
}

static void proper_private_break(struct proper_private *p, const struct area *current_area)
{
    hw2_notify(PROPER_PRIVATE_TOOK_BREAK, p->id, 0, 0);
    if (current_area)
        sm_pp_unlock_area(*current_area); // Unlock the area
    sm_pp_register_order_received();
    sm_pp_await_order_in_break();
    switch (sm_active_order()) {
        case CONTINUE:
            hw2_notify(PROPER_PRIVATE_CONTINUED, p->id, 0, 0);
            sm_pp_register_order_received();
            break;
        case STOP:
            proper_private_stop(p, NULL); // don't pass the area, already unlocked it
        case BREAK: // Should be impossible
            error_rt("Unexpected break after break wake-up");
    } 
}

// Very nifty function for checking orders:
// * Goes into the break and returns 1 upon continuing if getting break order.
// * Exits if getting a stop order.
// * Returns zero immediately if there is no break/stop order.
// The area is passed as pointer to allow for NULL in case the private holds no area.
int proper_private_react(struct proper_private *p, const struct area *current_area)
{
    enum order_type order = sm_active_order();
    if (order != CONTINUE) {
        if (order == BREAK) {
            proper_private_break(p, current_area); // Take a break, return 1 when done
            return 1;
        } else {
            proper_private_stop(p, current_area); // Stop the private
        }
    }
    return 0;
}

static void *proper_private_routine(void *proper_private)
{
    struct proper_private *p = proper_private;
    int k;

    hw2_notify(PROPER_PRIVATE_CREATED, p->id, 0, 0);

    k = 0;
    sm_soldier_lock(); // Locking here to improve code flow in the loop. No logic!
    while (k < p->n) {
        struct area area;
        area = area_from_span(p->corners[k].i, p->corners[k].j, 
                                          p->span_i, p->span_j);
        
        sm_soldier_unlock(); // Unlock read-lock before the next area locking attempt

        // Try to lock the area, sleep may be interrupted by an order
        if (sm_pp_lock_area(area, &p->cond)) {
            // Interrupted wait by order, holding read-lock, no area locked
            if (proper_private_react(p, NULL))
                continue;
        } else {
            // Got an area, but we can still check for an order
            sm_soldier_lock(); // acquire read-lock
            if (proper_private_react(p, &area))
                continue;
        }

        // I had goto in a previous version of the loop, and had decided to explain
        // where it could be useful. The goto is gone, but the explanation remains!
        //
        // While goto may seem unrefined to some, it's standard practice in kernel
        // code when you have a bunch of things to clean up manually. It's helpful
        // to be able to go to where you call those things before exiting/restarting
        // or whatever. Here's one standard non-goto alternative when you have
        // multiple failure points in your function: 
        // 
        // ...
        // if (fail1) {
        //   cleanup1();
        //   return 1;
        // }
        // ...
        // if (fail2) {
        //   cleanup1();
        //   cleanup2();
        //   return 1;
        // }
        // ...
        // if (fail3) {
        //   cleanup1();
        //   cleanup2();
        //   cleanup3();
        //   return 1;
        // }
        // ...
        // return 0;
        // 
        // I'd rather use:
        // 
        // ...
        // if (fail1)
        //   goto CLEANUP1;
        // ...
        // if (fail2)
        //   goto CLEANUP2;
        // ...
        // if (fail3)
        //   goto CLEANUP3;
        // ...
        // return 0; // No errors
        // // Error cases
        // CLEANUP3: cleanup3();
        // CLEANUP2: cleanup2();
        // CLEANUP1: cleanup1();
        // return 1;
        //
        // Ingenious if you ask me. Here we have a similar case for unlocking the read-lock.
        // But we can handle it by unlocking at the start of the loop and just using continue.

        hw2_notify(PROPER_PRIVATE_ARRIVED, p->id, area.tli, area.tlj);

        // No need to get grid mutexes, since no one else will touch for sure
        // (intersection is only between sneaky smokers)

        // Start gathering cigs and sleeping
        for (int i = area.tli; i <= area.bri; i++) {
            for (int j = area.tlj; j <= area.brj; j++) {
                while (ggrid.cells[i][j] > 0) {
                    sm_pp_cigrest(p->pause_msecs); // Releases & re-acquires order lock
                    if (proper_private_react(p, &area)) // Got an order?
                        goto LOOP_RESTART; // Easy way to break free of nested loops
                    ggrid.cells[i][j]--;
                    hw2_notify(PROPER_PRIVATE_GATHERED, p->id, i, j);
                }
            }
        }

        hw2_notify(PROPER_PRIVATE_CLEARED, p->id, 0, 0);
        k++;
       
        soldier_relock();
        if (proper_private_react(p, &area))
            continue;

        sm_pp_unlock_area(area);

LOOP_RESTART:
        ;
    }
  
    // Register leaving & notify before unlocking the order lock to prevent incons.
    sm_pp_register_leaving();
    hw2_notify(PROPER_PRIVATE_EXITED, p->id, 0, 0);
    sm_soldier_unlock();

    return NULL;
}


// COMMANDER STUFF

struct order {
    enum order_type type;
    struct timespec delivery_abstime;
};

struct commander {
    struct order *orders;
    size_t num_orders;
};

static struct timespec g_prog_start_time;

static void read_order(void *oo)
{
    struct order *o = oo;
    unsigned long delivery_time_msec;
    char buf[256];
    scanf(" %lu %s", &delivery_time_msec, buf);
    
    if (!strcmp(buf, "break"))
        o->type = BREAK;
    else if (!strcmp(buf, "continue"))
        o->type = CONTINUE;
    else if (!strcmp(buf, "stop"))
        o->type = STOP;
    else
        error_rt("Unknown order '%s'", buf);

    // Pre-calculate absolute delivery time based on start time
    assert (g_prog_start_time.tv_sec != 0);
    memcpy(&o->delivery_abstime, &g_prog_start_time, sizeof(g_prog_start_time));
    add_ms_to_timespec(delivery_time_msec, &o->delivery_abstime);
}

static void read_commander(void *cc)
{
    struct commander *c = cc;
    c->orders = read_array_wsize(read_order, sizeof(c->orders[0]), &c->num_orders);
}

static void free_commander(struct commander *c)
{
    free(c->orders);
}

static void *commander_routine(void *commander)
{
    struct commander *c = commander;
    enum order_type last_order = CONTINUE; // privates are currently working

    for (size_t i = 0; i < c->num_orders; i++) {
        enum order_type current_order = c->orders[i].type;

        // Call clock nanosleep with abstime till it returns zero to resist interruptions
        // (not really necessary, I don't expect it to be interrupted)
        while (clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, 
                               &c->orders[i].delivery_abstime, NULL)); 

        // In case the last sent-received order is the same
        // e.g. break after break, continue after continue
        // No need to do anything except for notifying.
        if (last_order == current_order) {
            sm_order_notify(current_order);
        } else {
            // Send the order
            sm_commander_order(current_order);
            
            // Wait for everyone to receive the order
            sm_commander_await_orders_received();
        }

        last_order = current_order;
    }

    return NULL;
}

// SMOKER STUFF

struct smoke_area {
    struct pair; // Using tagged nested structs is an ms extension, but very cool!
    // ^ this allows stuff like smoke_area.i, smoke_area.j without going through
    // another layer of struct like smoke_area.pair.i
    int n_cigs;
};

struct sneaky_smoker {
    unsigned long id, pause_msecs;
    size_t n;
    struct smoke_area *centers;
    pthread_cond_t cond;
};

static void read_smoke_area(void *sap)
{
    struct smoke_area *sa = sap;
    scanf (" %d %d %d", &sa->i, &sa->j, &sa->n_cigs);
}

static void read_sneaky_smoker(void *ss)
{
    struct sneaky_smoker *s = ss;
    scanf(" %lu %lu", &s->id, &s->pause_msecs);
    s->centers = read_array_wsize(read_smoke_area, sizeof(s->centers[0]), &s->n);
    PT_CHECKED(pthread_cond_init(&s->cond, NULL), "init sneaky smoker cond");

}

static void free_sneaky_smoker(struct sneaky_smoker *s)
{
    free(s->centers);
}

void sneaky_smoker_react(struct sneaky_smoker *s, const struct area *current_area)
{
    if (sm_active_order() == STOP) {
        hw2_notify(SNEAKY_SMOKER_STOPPED, s->id, 0, 0);
        if (current_area)
            sm_ss_unlock_area(*current_area);
        sm_ss_register_leaving();
        sm_soldier_unlock();
        pthread_exit(NULL);
    }
}

static void *sneaky_smoker_routine(void *sneaky_smoker)
{
    static const struct pair OFFSETS[] = { // Offsets for rotating flicks
        {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, 
        {1, 1}, {1, 0}, {1, -1}, {0, -1}
    };
    struct sneaky_smoker *s = sneaky_smoker;
    hw2_notify(SNEAKY_SMOKER_CREATED, s->id, 0, 0);

    sm_soldier_lock(); // See pp routine for explanation
    for (size_t k = 0; k < s->n; k++) {
        struct smoke_area center = s->centers[k];
        struct area area = area_3x3_from_center(center.i, center.j);

        sm_soldier_unlock();

        // Same logic as pp's apply for being interrupted by an order
        // No continue; since only stop orders are reacted to.
        if (sm_ss_lock_area(area, &s->cond)) {
            sneaky_smoker_react(s, NULL); // Holding read-lock and area not locked
        } else {
            sm_soldier_lock(); // acquire read-lock 
            sneaky_smoker_react(s, &area);
        }


        hw2_notify(SNEAKY_SMOKER_ARRIVED, s->id, center.i, center.j);

        // Start flicking ciggies
        for (size_t i = 0; i < center.n_cigs; i++) {
            // Calculate target cell
            size_t off_i = i % 8; // Could use & 7
            int target_i = center.i + OFFSETS[off_i].i;
            int target_j = center.j + OFFSETS[off_i].j;
        
            sm_ss_cigrest(s->pause_msecs); // Cigarette rest, possibly wake-up to stop
            sneaky_smoker_react(s, &area);

            // Since multiple smokers (up to 4) can litter a cell,
            // ideally we need to protect it. Mutexes are up!
            pthread_mutex_lock(&ggrid.locks[target_i][target_j]);
            ggrid.cells[target_i][target_j]++;
            pthread_mutex_unlock(&ggrid.locks[target_i][target_j]);
            hw2_notify(SNEAKY_SMOKER_FLICKED, s->id, target_i, target_j);
        }

        hw2_notify(SNEAKY_SMOKER_LEFT, s->id, 0, 0);

        soldier_relock(); // Give the commander a chance to give an order
        sneaky_smoker_react(s, &area); 

        sm_ss_unlock_area(area);
    }

    sm_ss_register_leaving();
    hw2_notify(SNEAKY_SMOKER_EXITED, s->id, 0, 0);
    sm_soldier_unlock();
    
    return NULL;
}

static void time_init(void)
{
    hw2_init_notifier();
    clock_gettime(CLOCK_REALTIME, &g_prog_start_time);
}

static void check_boundaries(const struct proper_private *pps, size_t n)
{
    // A little check to make sure the input is correct
    for (size_t i = 0; i < n; i++) {
        const struct proper_private *p = &pps[i];
        for (size_t j = 0; j < p->n; j++) {
            struct pair c = p->corners[j];
            if (c.i < 0 || c.i + p->span_i > ggrid.rows || 
                c.j < 0 || c.j + p->span_j > ggrid.cols)
            {
                error_rt("Area %lu is out of bounds in proper private with ID %lu", j, p->id);
            }
        }
    }
}

int main(void)
{
    size_t num_proper_privates, num_sneaky_smokers;
    pthread_t *proper_private_threads, *sneaky_smoker_threads;
    struct proper_private *proper_privates;
    struct commander commander;
    struct sneaky_smoker *sneaky_smokers;

    time_init();

    read_grid(&ggrid);
    proper_privates = read_array_wsize(read_proper_private, sizeof(proper_privates[0]), 
                                       &num_proper_privates);
    read_commander(&commander);
    sneaky_smokers = read_array_wsize(read_sneaky_smoker, sizeof(sneaky_smokers[0]),
                                      &num_sneaky_smokers);

    sm_init(num_proper_privates, num_sneaky_smokers);

    check_boundaries(proper_privates, num_proper_privates);

    proper_private_threads = malloc(num_proper_privates * sizeof(proper_private_threads[0])); 
    for (size_t i = 0; i < num_proper_privates; i++)
        PT_CHECKED(pthread_create(&proper_private_threads[i], NULL, 
                                  proper_private_routine, &proper_privates[i]),
                   "create pp thread");

    sneaky_smoker_threads = malloc(num_sneaky_smokers * sizeof(sneaky_smoker_threads[0]));
    for (size_t i = 0; i < num_sneaky_smokers; i++)
        PT_CHECKED(pthread_create(&sneaky_smoker_threads[i], NULL, 
                                  sneaky_smoker_routine, &sneaky_smokers[i]),
                   "create ss thread");

    // The main thread will be the commander
    commander_routine(&commander);

    // Time for deletion and joins
    free_commander(&commander);
    
    for (size_t i = 0; i < num_proper_privates; i++) {
        PT_CHECKED(pthread_join(proper_private_threads[i], NULL), "join pp thread");
        free_proper_private(&proper_privates[i]);
    }
    free(proper_private_threads);
    free(proper_privates);

    for (size_t i = 0; i < num_sneaky_smokers; i++) {
        PT_CHECKED(pthread_join(sneaky_smoker_threads[i], NULL), "join ss thread");
        free_sneaky_smoker(&sneaky_smokers[i]);
    }
    free(sneaky_smoker_threads);
    free(sneaky_smokers);
    
    free_grid(&ggrid);

    sm_destroy();

    return 0;
}

