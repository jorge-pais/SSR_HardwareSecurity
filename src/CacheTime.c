#include <emmintrin.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define DELTA 1024

uint8_t array[10*4096 + DELTA];

uint64_t accessTimes[10];

int main(int argc, const char **argv) {
    int junk=0;
    register uint64_t time1, time2;
    volatile uint8_t *addr;
    int i;
    // Initialize the array
    for(i=0; i<10; i++) array[i*4096 + DELTA]=1;
    // FLUSH the array from the CPU cache
    for(i=0; i<10; i++) _mm_clflush(&array[i*4096 + DELTA]);
    // Access some of the array items
    array[3*4096 + DELTA] = 100;
    array[7*4096 + DELTA] = 200;
    for(i=0; i<10; i++) {
        addr = &array[i*4096 + DELTA];
        time1 = __rdtscp(&junk);   junk = *addr;
        time2 = __rdtscp(&junk) - time1;
        accessTimes[i] = time2;
    }

    double cacheAvgTime = (accessTimes[3]+accessTimes[7])/2;
    double ramTime;
    for(i = 0; i < 10; i++)
        ramTime = (i == 3 || i == 7) ? ramTime : ramTime+accessTimes[i];
    
    printf("Avg. MEMORY access time: %f CPU cycles\n", ramTime/8);
    printf("Avg. CACHE access time : %f CPU cycles\n", cacheAvgTime);

    return 0; 
}
