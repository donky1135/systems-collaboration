CC = gcc
CFLAGS = -std=c99 -Wall -Wvla -fsanitize=address,undefined -g -fno-strict-aliasing
TARGETS = memtest memgrind errorcheck

memtest: memtest.o mymalloc.o
	$(CC) $(CFLAGS) memtest.o mymalloc.o -o memtest
memtest.o: memtest.c
	$(CC) $(CFLAGS) -c memtest.c
mymalloc.o: mymalloc.c
	$(CC) $(CFLAGS) -c mymalloc.c

all: $(TARGETS)

memtest: memtest.o mymalloc.o
	$(CC) $(CFLAGS) -o $@ $^
memgrind: memgrind.o mymalloc.o
	$(CC) $(CFLAGS) -o $@ $^
errorcheck: errorcheck.o mymalloc.o
	$(CC) $(CFLAGS) -o $@ $^
%.o: %.c
	$(CC) $(CFLAGS) -c $<
doublefree: memtest.o mymalloc.o mymalloc.h
	$(CC) $(CFLAGS) -o $@ $^
badpointer: mymalloc.o mymalloc.h
	$(CC) $(CFLAGS) -o $@ $^
toolarge: mymalloc.o mymalloc.h
	$(CC) $(CFLAGS) -o $@ $^
nothead: mymalloc.o mymalloc.h
	$(CC) $(CFLAGS) -o $@ $^


leakymemtest.o: memtest.o mymalloc.h
	$(CC) -c $(CFLAGS) -DLEAK $< -o $@
leakytest: leakymemtest.o mymalloc.o
	$(CC) $(CFLAGS) $^ -o $@
mymalloc.o memtest.o memgrind.o errorcheck.o: mymalloc.h
clean:
	rm -f *.o $(TARGETS)
