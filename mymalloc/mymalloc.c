#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

#define MEMLENGTH 4096
#define MIMCHUNKSIZE 8
#define ALLOCATED 1
#define FREE 0

static uniont{
	char bytes[MEMLENGTH];
	doublt not_used;
} heap;

static int is_init = 0;

void init() {
	int *head_chunk_size = heap.bytes;
	int* head_is_allocated = (heap.bytes + 4);
	*head_chunk_size = MEMLENGTH - MIMCHUNKSIZE;
	*head_is_allocated = FREE;
	is_init = 1;
	atexit(leak_detector);
}

void leak_detector() {
	int size = 0;
	int obj = 0;

	/*
	* detection here
	*/

	if (size) {
		fprintf(stderr, "mymalloc: %d bytes leaked in %d objects.", size, obj);
	}
}

void *mymalloc(size_t size, char* file, int line) {
	if (!(is_init)) init();
	if (/* cannot allocate */) {
		fprintf(stderr, "malloc: Unable to allocate %d bytes (%s:%d)", size, file, line);
		return NULL;
	}
}

void myfree(void* ptr, char* file, int line) {
	if (/* called with an address */) {
		fprintf(stderr, "free: Inappropriate pointer (%s:%d)", file, line);
		exit(2);
	}
	if (/* called with an address not at the start of a chunk */) {
		fprintf(stderr, "free: Inappropriate pointer (%s:%d)", file, line);
		exit(2);
	}
	if (/* second call */) {
		fprintf(stderr, "free: Inappropriate pointer (%s:%d)", file, line);
		exit(2);
	}
}