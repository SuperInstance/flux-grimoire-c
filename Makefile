CC ?= gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -O2
AR = ar

all: libgrimoire.a test

libgrimoire.a: grimoire.o
	$(AR) rcs $@ $^

grimoire.o: grimoire.c grimoire.h
	$(CC) $(CFLAGS) -c grimoire.c -o $@

test: test_grimoire.c grimoire.h grimoire.c
	$(CC) $(CFLAGS) -o $@ test_grimoire.c grimoire.c -lm

check: test
	./test

clean:
	rm -f *.o *.a test

.PHONY: all check clean
