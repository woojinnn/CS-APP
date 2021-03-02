CC = gcc
CFLAGFS = -Wall

SRCS = $(wildcard *.c)
PROGS = $(patsubst %.c,%,$(SRCS))

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -rf $(PROGS)
