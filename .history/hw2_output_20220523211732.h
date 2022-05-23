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

// Call this at the start of your main function!
void hw2_init_notifier(void);

// The notifier you should use *literally* everywhere.
void hw2_notify(enum hw2_actions action, unsigned id, unsigned x, unsigned y);

#ifdef __cplusplus
}
#endif

#endif
