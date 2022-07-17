LDFLAGS=-pthread
CFLAGS=-Wall -Wshadow -pedantic -Wno-unused-result -Wno-unused-function -std=gnu11 -fms-extensions -O3

SRCS=hw2.c hw2_output.c generic_rwa.c area_containers.c error.c sync_mechanism.c

all: hw2

hw2: $(SRCS:.c=.o)

# implicit rules handle the generation of .o files. LDFLAGS and CFLAGS are used by default.
# i.e. the makefile already uses something like $(CC) $(LDFLAGS) $(CFLAGS) -c <name>
# by default for object file (.o) targets.

# Automatic dependency generation. See:
# https://www.gnu.org/software/make/manual/html_node/Automatic-Prerequisites.html
%.d: %.c
	@set -e; rm -f $@; \
	$(CC) $(CFLAGS) -MM $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

include $(SRCS:.c=.d)

clean:
	rm -f *.o *.d hw2
