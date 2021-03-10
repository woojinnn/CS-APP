CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Wconversion

SRCS = $(wildcard *.c)
PROGS = $(patsubst %.c,%,$(SRCS))

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -rf $(PROGS)
