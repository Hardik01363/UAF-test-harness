#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include <time.h>

//configs (didnt create a config.h file as this is a pretty small project)
#define ARR_SIZE 16 //small so as to increase thread contention
#define NUM_THREADS 16 //my machine only supports upto 8 threads, so, not going too overboard here. but, higher no. of threads would be better for testing.
#define ITERS_PER_THREAD 1000000

typedef struct Node {long value;} Node;
_Atomic(Node*) test_arr[ARR_SIZE]; //using _Atomic(Node*) instead of just Node* so as to tell the compiler that CAS atomic instructions would be valid for this array.\

//raandom number generator used by each of the threads (rand() not thread safe)
static unsigned int xorshift(unsigned int* state) {
    unsigned int x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

void* worker(void* arg) {
    long tid = (long)(size_t)arg;
    unsigned int rng = (unsigned int)(time(NULL) ^ (tid * 2654435761u));
}

int main(void) {
    for(int i = 0; i < ARR_SIZE; i++) {
        atomic_init(&test_arr[i], NULL);
    }
    
}
