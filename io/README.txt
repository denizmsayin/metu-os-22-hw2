Example outputs are from my own implementation compiled with -O0. Feel free to use -O3 in
your makefiles for extra speed! Also remember that the example output is just an example,
it only matters that it conforms to the rules in the homework & complex_scenarios.pdfs,
such as no notifications after orders, waiting in between orders etc.

io
|---part1
|   |---one_pp.in:               A very simple case with a single gatherer.
|   |---some_nointersect_pps.in: A simple case with a few gatherers whose areas do not intersect.
|   |---some_pps.in:             Similar, but with a bit of intersection between gatherers.
|   |---cc_three_pp_case.in:     The 3 proper private case explained in complex_scenarios.pdf
|                                0 and 2 should be able to lock at the same time if you have 
|                                g full part 3 implementation.
|---part2
|   |---break_continue.in:       A basic case with break & continue one second later.
|   |---stop.in:                 Another basic case with a stop.
|   |---leave_and_stop.in:       Same, but with a proper private leaving before the stop order.
|   |---leave_and_stop_extra.in: Similar. Have some pointless orders after the stop.
|   |---breakbreakbreak.in:      A bunch of redundant break orders before continuing.
|   |---break_continue_fast.in:  An extreme case with 400 independent pps and a continue
|   |                            following break 1 ms later. Observe that continue is
|   |                            delayed by around ~20ms which is fine. Up to ~100ms is fine.
|   |---bcf_contention.in:       Same as before, but all 400 pps want the same area now.
|                                Crazy! This time the order is delayed by ~5ms. Anything
|                                up to 100ms should be fine again. Do note that 100ms is not
|                                general, I'll tailor the acceptable delay to the specific case.
|---part3
    |---nearby_smokers.in:       Two ss next to each other that are able to smoke in parallel.
    |---same_smokers.in:         A bunch of smokers wanting the same cell. They should wait
    |                            for each other.
    |---4_smokers_1_pp.in:       Maximum intersection between 4 extremely fast smokers.
    |                            A pp first wastes time in the upper part of the grid,
    |                            and then comes to gather what the smokers smoked below.
    |---hard_stop.in:            A stop containing both stuck pp's and ss's stuck waiting
    |                            for an area. Can you stop properly?
    |---hard_bcs.in:             Similar, but with a break-continue before the stop too.
    |                            Oh boy!
    |---ss_and_pp.in:            Another mixed case containing orders, proper privates and 
                                 sneaky smokers. Should be similar to hard_bcs.in.

TIP FOR DEBUGGING DEADLOCKS:
Alright, I hope you already learned about this through your own research during the homework,
but I'm spoiling it along with these example cases:
* Suspect a deadlock. i.e. your program seems to get stuck indefinitely.
* Compile your program with -O0 and -g.
* Run it with gdb, gdb ./hw2
* Start it with your input file inside, r < input_file.in
* Program stuck? Press Ctrl + C.
* See which thread is where by switching between threads with "t 0", "t 1", "t 2" etc. and
  checking their stacks with "bt" (short for backtrace). Print local variables with "p ...".
  Should give you a very good idea of what is stuck waiting for what.
