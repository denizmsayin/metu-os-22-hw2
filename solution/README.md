
Maybe you'll want to come back to this one day!

I tried to heavily comment my solution and like to think that it is readable. A modest
1074 lines of C for a solid 100% solution, according to `sloccount`.

Since reading code is not easy, I gave a general outline of the solution and some pointers
in `solution_explanation.pdf`. One important piece of content is the answer to the elusive
question: **How to use condition variables properly?**.

#### Why C instead of C++?

##### Pros

* Easier to focus on the details of the synchronization mechanisms, rather than get bogged down in classes.
* Goes well with the primitive system calls/pthread functions.
* Can show off some fancy C tricks.

##### Cons

* Writing your own data structures. Disgusting!
* No generics. Hate it!
* Having to free everything manually. I'm fine with it, but code bloat nonetheless.

#### A General Outline of the Files

* `Makefile`: I think it's a nice example of a short makefile. Please check it!
* `arraylist.h`: Generic arraylist implementation with a macro, similar to templates. 
* `area.h`: A bunch of basic functions to deal with areas.
* `area_containers.{h,c}`: Active and waiting soldier lists. One is just a list
    of areas, the other is an area-value style map using two lists.
* `error.{h,c}`: A bunch of utility functions for throwing errors.
* `time_utils.h`: Two small utilities for manipulating time.
* `generic_rwa.{h,c}`: A few generic functions for allocating/freeing 2D arrays
    as well as getting arrays from the input.
* `hw2.c`: The core file containing the main program, PP, SS, commander routines.
* `sync_mechanism.{h,c}`: An interface to all the synchronization variables and functions used 
    in the program. Orders, areas, all the good stuff.
* `hw2_output.{h,c}`: You're probably familiar with these :)
* `grader-results`: The folder containing results of grading on `inek99` for the solution code.
