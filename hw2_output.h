#ifndef HW2_OUTPUT_H_
#define HW2_OUTPUT_H_

enum hw2_actions {
    // First part
    GATHERER_CREATED,
    GATHERER_ARRIVED,
    GATHERER_GATHERED,
    GATHERER_CLEARED,
    GATHERER_EXITED,

    // Second part
    GATHERER_TOOK_BREAK,
    GATHERER_STOPPED,
    GATHERER_CONTINUED,
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


#endif
