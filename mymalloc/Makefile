CC = gcc
CFLAGS = -std=c99 -Wall -Wvla -fsanitize=address,undefined
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
leakymemtest.o: memtest.o mymalloc.h
	$(CC) -c $(CFLAGS) -DLEAK $< -o $@
leakytest: leakymemtest.o mymalloc.o
	$(CC) $(CFLAGS) $^ -o $@
mymalloc.o memtest.o memgrind.o errorcheck.o: mymalloc.h
clean:
	rm -f *.o $(TARGETS)