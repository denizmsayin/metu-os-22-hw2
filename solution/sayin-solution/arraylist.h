#ifndef HW2_ARRAYLIST_H_
#define HW2_ARRAYLIST_H_

// Here's the disadvantage of using C: having to make your own
// data structures. 
// Two ways:
// - Generic implementation with void *'s and stuff like memcpy etc.
// - Ugly macro definition for template-like behavior

// I'm going to go with the second for performance, even though it's ugly. C-life...

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#define ARRAYLIST_DEFAULT_CAP   8

// Macro for defining arraylists. Should be careful to not re-use to prevent errors.
// Unfortunately macros can't generate other pre-processing directives,
// so it's best to add our own guards by sticking to consistent naming.
// This is how we'll do it:
// #ifndef ARRAYLIST_suff_DEFINED
// #define ARRAYLIST_suff_DEFINED
// DEFINE_ARRAYLIST(T, suff)
// #endif

#define DEFINE_ARRAYLIST(T, suff)\
    typedef struct {\
        T *data;\
        size_t len, cap;\
    } arraylist_##suff##_t;\
    \
    static void arraylist_##suff##_init(arraylist_##suff##_t *al, size_t initial_cap)\
    {\
        al->len = 0;\
        al->cap = initial_cap > 0 ? initial_cap : ARRAYLIST_DEFAULT_CAP;\
        al->data = malloc(sizeof(al->data[0]) * al->cap);\
    }\
    \
    static void arraylist_##suff##_destroy(arraylist_##suff##_t *al)\
    {\
        free(al->data);\
    }\
    \
    static void arraylist_##suff##_reserve(arraylist_##suff##_t *al, size_t new_cap)\
    {\
        if (new_cap >= al->len) {\
            al->data = realloc(al->data, sizeof(T) * new_cap);\
            al->cap = new_cap;\
        }\
    }\
    \
    static void arraylist_##suff##_insert(arraylist_##suff##_t *al, T elem)\
    {\
        if (al->len == al->cap)\
            arraylist_##suff##_reserve(al, al->cap * 2);\
        al->data[al->len++] = elem;\
    }\
    static void arraylist_##suff##_qremove(arraylist_##suff##_t *al, size_t ridx)\
    {   /* Quick O(1) removal via swap-with-last */ \
        al->data[ridx] = al->data[--al->len];\
    }\
    \
    static size_t arraylist_##suff##_itr2idx(const arraylist_##suff##_t *al, T *itr)\
    {   /* Using pointers for iterator-like behavior */ \
        /* This is slightly iffy, since error checking is actually undefined behavior...
         * (subtracting pointers not belonging to the same array is not defined)
         * I'll just assume that the pointer is valid, but still apply checks. */\
        ptrdiff_t d = itr - al->data;\
        if (d < 0 || d > al->len) {\
            fprintf(stderr, "Out of bounds pointer %p for arraylist at %p.\n",\
                    (void *) itr, (void *) al->data);\
            exit(EXIT_FAILURE);\
        }\
        return (size_t) d;\
    }

// We can also define iterators like C++ as macros since it's the same for all types.
// Can even use them with ++, -- since there's just an array in the backend!
#define ALIST_BEGIN(al) (&(al)->data[0])
#define ALIST_END(al) (&(al)->data[(al)->len])
#define ALIST_AT(al, i) (&(al)->data[(i)])

#endif
