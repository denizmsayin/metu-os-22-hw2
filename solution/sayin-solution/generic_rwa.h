#ifndef HW2_GENERIC_RWA_H_
#define HW2_GENERIC_RWA_H_

#include <stddef.h>

void *alloc_2d(size_t rows, size_t cols, size_t elem_size);
void free_2d(void *arrp, size_t rows, size_t cols);

void *read_array_wsize(void (*reader)(void *), size_t elem_size, size_t *count);

#endif
