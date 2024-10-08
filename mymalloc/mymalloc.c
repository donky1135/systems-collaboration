#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"
#include <unistd.h>

#define MEMLENGTH 4096
#define DEBUG 1


static union {
    char bytes[MEMLENGTH];
    double not_used;
} heap;


static int initialized;



void heap_print(){
    for(int i = 0; i < MEMLENGTH; i++){
        // if(!i) printf("0000: ");
        printf("%02X ", (heap.bytes[i])&0xff);
        if((i + 1) % 16 == 0 && i+1 != MEMLENGTH)
            printf("\n");
    }
    printf('\n');
}

void initialize_heap(){
    initialized = 1;
    ((int*)heap.bytes)[0] = -1*(MEMLENGTH - 8);
    ((int*)heap.bytes)[1] = (MEMLENGTH - 8);
    // ((int*)heap.bytes)[0] = 0xfffffffe;
    // ((int*)heap.bytes)[1] = 0xfffffffe;
    if(DEBUG) heap_print();
}

void *mymalloc(size_t size, char *file, int line){
    if (!initialized) initialize_heap();

    return NULL;
}

void myfree(void *ptr, char *file, int line){

}




