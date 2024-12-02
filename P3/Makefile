CC = gcc
CFLAGS = -std=c99 -Wall -Wvla -fsanitize=address,undefined -g -fno-strict-aliasing
TARGETS = mysh

mysh.o: mysh.c
	$(CC) $(CFLAGS) -c mysh.c

mysh: mysh.o mysh.h
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -f *.o $(TARGETS)


