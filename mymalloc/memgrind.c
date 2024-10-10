#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>



// Compile with -DREALMALLOC to use the real malloc() instead of mymalloc()
#ifndef REALMALLOC
#include "mymalloc.h"
#endif

// Compile with -DLEAK to leak memory
#ifndef LEAK
#define LEAK 0
#endif



void workload(){
    for(int i = 0; i < 120; i++){
        char *test1 = (char*)malloc(1);
        free(test1);
    }

    char* test2[120];
    for(int i = 0; i < 120; i++){
        test2[i] = (char*)malloc(1);
    }


    for(int i = 0; i < 120; i++){
        free(test2[i]);
    }

    char *test3[120];
    char test3Allocated[120];
    int allocations = 120;
    int numAllocs = 0;
    while(allocations){
        int choice = rand() % 2;
        if(choice){
            for(int i = 0; i < 120; i++){
                if(!test3Allocated[i]){
                    test3[i] = (char *)malloc(1);
                    test3Allocated[i]++;
                    allocations--;
                    numAllocs++;
                    break;
                }
            }
        } else {
            if(numAllocs){
                int temp = 0;
                int choice = rand() % numAllocs;
                for(int i = 0; i < 120; i++){
                    if(test3Allocated[i]){
                        if(temp == choice){
                            free(test3[i]);
                            test3Allocated[i]--;
                            numAllocs--;
                            break;
                        }
                        temp++;
                    }
                }
            }
        }
    }
    
    for(int i = 0; i < 120; i++){
        if(test3Allocated[i])
            free(test3[i]);
    }
}


int main(){
    struct timeval start;
    gettimeofday(&start, NULL);

    for(int i = 0; i < 50; i++){
        printf("Workload %02d started\n", i+1);
        workload();
        printf("Workload %02d ended\n", i+1);
    }
    struct timeval end;

    gettimeofday(&end, NULL);

    printf("Average workload time: %f\n", (end.tv_usec - start.tv_usec)/50000.0 );
}