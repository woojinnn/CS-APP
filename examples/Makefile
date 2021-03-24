CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Wconversion -Og

SRCS = $(filter-out s04-sum.c, $(wildcard *.c))
PROGS = $(patsubst %.c,%,$(SRCS))

all: $(PROGS) s04-sum.o s04-sum.dis.s s04-sum.s

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

s04-sum.o: s04-sum.c
	$(CC) $(CFLAGS) -c -o $@ $<

s04-sum.dis.s: s04-sum.o
	objdump -d $< > $@

s04-sum.s: s04-sum.c
	$(CC) $(CFLAGS) -S -fverbose-asm -o $@ $<

.PHONY: clean
clean:
	rm -rf $(PROGS) s04-sum.o s04-sum.dis.s s04-sum.s
