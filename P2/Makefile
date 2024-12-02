CC = gcc
CFLAGS = -std=c99 -Wall -Wvla -fsanitize=address,undefined -g -fno-strict-aliasing
TARGETS = words


words: words.c
	$(CC) $(CFLAGS) -o words words.c 
clean:
	rm -f *.o $(TARGETS)


