#ifndef HW2_ERROR_H_
#define HW2_ERROR_H_

// Various wrappers to simplify error-reporting

#include <stdio.h>

void warning(const char *fmt, ...);
void error_rt(const char *fmt, ...); // rt = runtime
void error_wno(int no, const char *fmt, ...); // provided errno
void error_werrno(const char *fmt, ...); // current errno

// For performing error-checked calls to pthread functions easily
#define PT_CHECKED(call, what) do {\
        int ret = call;\
        if (ret)\
            error_wno(ret, "Failed to " what);\
    } while(0)
// ^ do { } while(0) is a common macro pattern to keep
// macros inside their own block. Also, can replace this
// with empty if you want to remove error checks
// for extra performance!

#endif
