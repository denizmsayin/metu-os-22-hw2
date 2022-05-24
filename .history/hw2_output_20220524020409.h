#ifndef HW2_OUTPUT_H_
#define HW2_OUTPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

enum hw2_actions {
  // First part
  PROPER_PRIVATE_CREATED,
  PROPER_PRIVATE_ARRIVED,
  PROPER_PRIVATE_GATHERED,
  PROPER_PRIVATE_CLEARED,
  PROPER_PRIVATE_EXITED,

  // Second part
  PROPER_PRIVATE_TOOK_BREAK,
  PROPER_PRIVATE_STOPPED,
  PROPER_PRIVATE_CONTINUED,
  ORDER_BREAK,
  ORDER_CONTINUE,
  ORDER_STOP,

  // Third part
  SNEAKY_SMOKER_CREATED,
  SNEAKY_SMOKER_ARRIVED,
  SNEAKY_SMOKER_FLICKED,
  SNEAKY_SMOKER_LEFT,
  SNEAKY_SMOKER_EXITED,
  SNEAKY_SMOKER_STOPPED,
};


typedef struct proper_private {
  int id;
  int x;
  int y;
  int z;
  int state;
} proper_private;

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

typedef struct hw2_output {
  int num_actions;
  hw2_action actions[];
} hw2_output;

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

void hw2_notify(enum hw2_actions action, unsigned id, unsigned x, unsigned y);

#ifdef __cplusplus
}
#endif

#endif
