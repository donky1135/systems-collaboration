#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

#define DEBUG 1






#define MEMLENGTH 4096
#define HEADERSIZE 8
#define HEADERINT 4
#define ALLOCATED 1
#define FREE 0

static union{
	char bytes[MEMLENGTH];
	double not_used;
} heap;


void heap_print(){
    for(int i = 0; i < MEMLENGTH; i++){
        // if(!i) printf("0000: ");
        printf("%02X ", (heap.bytes[i])&0xff);
        if((i + 1) % 16 == 0 && i+1 != MEMLENGTH)
            printf("\n");
    }
    printf('\n');
}


static int is_init = 0;

void init() {
	int *head_chunk_size = (int *)heap.bytes;
	int *head_is_allocated = (int *)(heap.bytes + HEADERINT);
	*head_chunk_size = MEMLENGTH - HEADERSIZE;
	*head_is_allocated = FREE;
	is_init = 1;
	atexit(leak_detector);
        if(DEBUG) heap_print();
}

void leak_detector() {
	int size = 0;
	int obj = 0;

	char *crr = heap.bytes;

	while (crr != (heap.bytes + MEMLENGTH)) {
		int *chunk_size = (int *)crr;
		int *is_allocated = (int *)(crr + HEADERINT);
		if (*is_allocated == ALLOCATED) {
			size += *chunk_size;
			obj++;
		}
		crr += (HEADERSIZE + *chunk_size);
	}

	if (size) {
		fprintf(stderr, "mymalloc: %d bytes leaked in %d objects.", size, obj);
	}
}

void *mymalloc(size_t size, char *file, int line) {
	int size_8_mul = (size + 7) & (~7);
	char *crr;

	if (!(is_init)) init();

	crr = heap.bytes;

	while (crr != (heap.bytes + MEMLENGTH)) {
		int *chunk_size = (int *)crr;
		int *is_allocated = (int *)(crr + HEADERINT);

		if ((*is_allocated == FREE) && (*chunk_size >= size_8_mul)) {
			char *nxt = crr + (HEADERSIZE + size_8_mul);
			if (nxt < (heap.bytes + MEMLENGTH)) {
				int* nxt_size = (int*)nxt;
				int* nxt_is_all = (int*)(nxt + HEADERINT);

				*nxt_size = *chunk_size - (HEADERSIZE + size_8_mul);
				*nxt_is_all = FREE;
			}

			*chunk_size = size_8_mul;
			*is_allocated = ALLOCATED;

			return (void *)(crr + HEADERSIZE);
		}
		crr += (HEADERSIZE + *chunk_size);
	}

	fprintf(stderr, "malloc: Unable to allocate %d bytes (%s:%d)", size, file, line);
	return NULL;

}

void myfree(void *ptr, char *file, int line) {
	char *crr = heap.bytes;
	char *obj = ((char *)(ptr)-HEADERSIZE);

	if ((obj < heap.bytes) || ((heap.bytes + MEMLENGTH) <= obj)) { // Calling free() with an address not obtained from malloc()
		fprintf(stderr, "free: Inappropriate pointer (%s:%d)", file, line);
		exit(2);
	}
	
	while(crr != (heap.bytes + MEMLENGTH)){
		int *chunk_size = (int *)crr;
		int *is_allocated = (int *)(crr + HEADERINT);

		if (is_adjacent(crr, obj) && (*is_allocated == FREE)) { // if the previous chunk free, connect it to the object
			int *obj_size = (int *)obj;
			int *obj_is_all = (int *)(obj + HEADERINT);

			if (*obj_is_all == FREE) { // Calling free() a second time on the same pointer
				fprintf(stderr, "free: Inappropriate pointer (%s:%d)", file, line);
				exit(2);
			}

			*chunk_size += (HEADERSIZE + *obj_size);
			*is_allocated = ALLOCATED; // for the purpose of detecting second call in the next if section
			obj = crr;
		}

		if (crr == obj) {
			char *nxt = crr + (HEADERSIZE + chunk_size);
			int *nxt_size = (int *)nxt;
			int *nxt_is_all = (int *)(nxt + HEADERINT);

			if (*is_allocated == FREE) { // Calling free() a second time on the same pointer.
				fprintf(stderr, "free: Inappropriate pointer (%s:%d)", file, line);
				exit(2);
			}

			*is_allocated = FREE;

			if ((nxt < (heap.bytes + MEMLENGTH)) && (*nxt_is_all == FREE)) chunk_size += (HEADERSIZE + *nxt_size);

			return;
		}

		if (crr > obj) { // Calling free() with an address not at the start of a chunk
			fprintf(stderr, "free: Inappropriate pointer (%s:%d)", file, line);
			exit(2);
		}
	}
}

int is_adjacent(char *crr, char *nxt) {
	int *chunk_size = (int *)crr;
	return ((crr + HEADERSIZE + *chunk_size) == nxt);
}
