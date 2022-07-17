#include <stdio.h>
#include <stdlib.h>

// Could have a fancy scheme with just 1 malloc call, but whatever!
void *alloc_2d(size_t rows, size_t cols, size_t elem_size)
{
    void **arr = malloc(sizeof(void *) * rows);
    for (size_t i = 0; i < rows; i++)
        arr[i] = malloc(cols * elem_size);
    return arr;
}

void free_2d(void *arrp, size_t rows, size_t cols)
{
    void **arr = arrp;
    (void) cols;
    for (size_t i = 0; i < rows; i++)
        free(arr[i]);
    free(arr);
}

// Reading a 1D array given size, 0 if EOF. Useful for all inputs.
void *read_array_wsize(void (*reader)(void *), size_t elem_size, size_t *count)
{
    size_t n;
    void *elems;
    size_t ret = scanf(" %lu", &n);

    if (ret == EOF) {
        *count = 0;
        return NULL;
    }

    elems = malloc(n * elem_size);
    for (int i = 0; i < n; i++)
        reader((char *) elems + i * elem_size);
    *count = n;

    return elems;
}

