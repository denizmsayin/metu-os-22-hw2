#include "error.h"

#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void warning(const char *fmt, ...)
{
    va_list args;
    fprintf(stderr, "Warning: ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    putc('\n', stderr);
}

void error_rt(const char *fmt, ...)
{
    va_list args;
    fprintf(stderr, "Error: ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    putc('\n', stderr);
    exit(EXIT_FAILURE);
}

void error_wno(int num, const char *fmt, ...) 
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    errno = num;
    perror(" ");
    exit(EXIT_FAILURE);
}

void error_werrno(const char *fmt, ...)
{
    int prev_errno = errno;
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    errno = prev_errno;
    perror(" ");
    exit(EXIT_FAILURE);
}

