#ifndef HW2_HEADER_H
#define HW2_HEADER_H

#include <bits/types/FILE.h>
#include <bits/types/__locale_t.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct terrain_cell {
  unsigned i;
  unsigned j;
  unsigned cig_count;
  unsigned num_proper_privates;
  unsigned num_sneaky_smokers;
} terrain_cell;


// proper private are
typedef struct proper_private {
  int id;
  int area_i;
  int area_j;
  int time_to_gather;
  int areas_to_gather;
  terrain_cell *areas;
  int state;
} proper_private;
typedef struct {
  unsigned id;
  unsigned i;
  unsigned j;
} hw2_notification;

typedef struct {
  unsigned id;
  unsigned i;
  unsigned j;
} hw2_order;

typedef struct {
  unsigned id;
  unsigned i;
  unsigned j;
} hw2_order_break;
typedef struct {
  unsigned id;
  unsigned i;
  unsigned j;
} hw2_order_continue;
typedef struct {
  unsigned id;
  unsigned i;
  unsigned j;
} hw2_order_stop;
typedef struct {
  unsigned id;
  unsigned i;
  unsigned j;
} hw2_order_flicked;


typedef struct terrain {
  unsigned num_rows;
  unsigned num_cols;
  terrain_cell *cells;
  unsigned num_proper_privates;
  proper_private *proper_privates;
} terrain;

typedef struct hw2_state {
  unsigned num_smokers;
  unsigned num_proper_privates;
  unsigned num_sneaky_smokers;
  unsigned num_gathers;
  unsigned num_clears;
  unsigned num_exits;
  unsigned num_breaks;
  unsigned num_continues;
  unsigned num_stops;
  unsigned num_flicked;
  unsigned num_lefts;
  unsigned num_exits_sneaky;
  unsigned num_stops_sneaky;
  unsigned num_flicked_sneaky;
  unsigned num_lefts_sneaky;
} hw2_state;

typedef struct hw2_output {
  hw2_state state;
  terrain *terrain;
  hw2_notification *notifications;
  hw2_order *orders;
  hw2_order_break *order_breaks;
  hw2_order_continue *order_continues;
  hw2_order_stop *order_stops;
  hw2_order_flicked *order_flicked;
} hw2_output;

typedef struct hw2_output_writer {
  void (*write_state)(struct hw2_output_writer *self, hw2_state state);
  void (*write_terrain)(struct hw2_output_writer *self, terrain *terrain);
  void (*write_notification)(struct hw2_output_writer *self, hw2_notification notification);
  void (*write_order)(struct hw2_output_writer *self, hw2_order order);
  void (*write_order_break)(struct hw2_output_writer *self, hw2_order_break order);
  void (*write_order_continue)(struct hw2_output_writer *self, hw2_order_continue order);
  void (*write_order_stop)(struct hw2_output_writer *self, hw2_order_stop order);
  void (*write_order_flicked)(struct hw2_output_writer *self, hw2_order_flicked order);
} hw2_output_writer;

typedef struct hw2_output_writer_c {
  hw2_output_writer writer;
  FILE *file;
  __locale_t locale;
} hw2_output_writer_c;

typedef struct sneaky_smoker {
  int id;
  int x;
  int y;
  int z;
  int state;
} sneaky_smoker;

typedef struct order {
  int id;
  int x;
  int y;
  int z;
  int state;
} order;

typedef struct hw2_action {
  int action;
  union {
    proper_private proper_private;
    sneaky_smoker sneaky_smoker;
    order order;
  };
} hw2_action;

typedef struct hw2_output_iterator {
  hw2_output *output;
  int index;
} hw2_output_iterator;
// Call this at the start of your main function!
void hw2_init_notifier(void);

typedef void (*hw2_notifier_callback)(hw2_output *output);

typedef struct hw2_notifier {
  hw2_notifier_callback callback;
} hw2_notifier;

typedef struct hw2_notifier_iterator {
  hw2_notifier *notifier;
  int index;
} hw2_notifier_iterator;

typedef struct hw2_notifier_list {
  hw2_notifier_iterator begin;
  hw2_notifier_iterator end;
} hw2_notifier_list;

typedef struct hw2_notifier_list_iterator {
  hw2_notifier_list *list;
  hw2_notifier_iterator iterator;
} hw2_notifier_list_iterator;


// Call this at the end of your main function!

// Call this to register a callback.
void hw2_register_notifier(hw2_notifier *notifier);

// Call this to deregister a callback.
void hw2_deregister_notifier(hw2_notifier *notifier);
// Call this to iterate over the notifiers.
// Returns 0 when there are no more notifiers.
int hw2_notifier_list_iterator_next(hw2_notifier_list_iterator *iterator);
// Call this to iterate over the notifiers.
// Returns 0 when there are no more notifiers.
int hw2_notifier_iterator_next(hw2_notifier_iterator *iterator);
// Call this to iterate over the notifiers.
// Returns 0 when there are no more notifiers.
int hw2_notifier_list_iterator_prev(hw2_notifier_list_iterator *iterator);
// Call this to iterate over the notifiers.
// Returns 0 when there are no more notifiers.
int hw2_notifier_iterator_prev(hw2_notifier_iterator *iterator);
// Call this to iterate over the notifiers.
// Returns 0 when there are no more notifiers.
int hw2_notifier_list_iterator_begin(hw2_notifier_list_iterator *iterator);
// Call this to iterate over the notifiers.
// Returns 0 when there are no more notifiers.
int hw2_notifier_iterator_begin(hw2_notifier_iterator *iterator);
// Call this to iterate over the notifiers.
// Returns 0 when there are no more notifiers.
int hw2_notifier_list_iterator_end(hw2_notifier_list_iterator *iterator);
// Call this to iterate over the notifiers.
// Returns 0 when there are no more notifiers.
int hw2_notifier_iterator_end(hw2_notifier_iterator *iterator);
// Call this to iterate over the notifiers.
// Returns 0 when there are no more notifiers.
int hw2_notifier_list_iterator_deref(hw2_notifier_list_iterator *iterator);
// Call this to iterate over the notifiers.
// Returns 0 when there are no more notifiers.
int hw2_notifier_iterator_deref(hw2_notifier_iterator *iterator);
// Call this to iterate over the notifiers.
// Returns 0 when there are no more notifiers.
int hw2_notifier_list_iterator_inc(hw2_notifier_list_iterator *iterator);
// Call this to iterate over the notifiers.
// Returns 0 when there are no more notifiers.
int hw2_notifier_iterator_inc(hw2_notifier_iterator *iterator);
// Call this to iterate over the notifiers.
// Returns 0 when there are no more notifiers.

int hw2_notifier_list_iterator_dec(hw2_notifier_list_iterator *iterator);
// Call this to iterate over the notifiers.
// Returns 0 when there are no more notifiers.
int hw2_notifier_iterator_dec(hw2_notifier_iterator *iterator);

// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_next(hw2_output_iterator *iterator);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_prev(hw2_output_iterator *iterator);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_begin(hw2_output_iterator *iterator);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_end(hw2_output_iterator *iterator);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_deref(hw2_output_iterator *iterator);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_inc(hw2_output_iterator *iterator);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_dec(hw2_output_iterator *iterator);

// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_next_action(hw2_output_iterator *iterator, int *action);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_prev_action(hw2_output_iterator *iterator, int *action);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
// The notifier you should use *literally* everywhere.
int hw2_output_iterator_begin_action(hw2_output_iterator *iterator, int *action);
// Call this to iterate over the actions.   
// Returns 0 when there are no more actions.
int hw2_output_iterator_end_action(hw2_output_iterator *iterator, int *action);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_deref_action(hw2_output_iterator *iterator, int *action);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_inc_action(hw2_output_iterator *iterator, int *action);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_dec_action(hw2_output_iterator *iterator, int *action);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_next_notifier(hw2_output_iterator *iterator, hw2_notifier *notifier);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_prev_notifier(hw2_output_iterator *iterator, hw2_notifier *notifier);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_begin_notifier(hw2_output_iterator *iterator, hw2_notifier *notifier);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_end_notifier(hw2_output_iterator *iterator, hw2_notifier *notifier);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_deref_notifier(hw2_output_iterator *iterator, hw2_notifier *notifier);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_inc_notifier(hw2_output_iterator *iterator, hw2_notifier *notifier);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_dec_notifier(hw2_output_iterator *iterator, hw2_notifier *notifier);

// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_next_data(hw2_output_iterator *iterator, void **data);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_prev_data(hw2_output_iterator *iterator, void **data);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_begin_data(hw2_output_iterator *iterator, void **data);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_end_data(hw2_output_iterator *iterator, void **data);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_deref_data(hw2_output_iterator *iterator, void **data);
// Call this to iterate over the actions.
// Returns 0 when there are no more actions.
int hw2_output_iterator_inc_data(hw2_output_iterator *iterator, void **data);

#ifdef __cplusplus
}
#endif

#endif
