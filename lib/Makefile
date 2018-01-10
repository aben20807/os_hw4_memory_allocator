TARGETS = hw_malloc.o
CC ?= gcc
CFLAGS += -std=gnu99 -Wall # -g3
OBJS = $(TARGETS)

all: $(TARGETS)

$(OBJS): %.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o
