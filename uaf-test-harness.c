#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include <time.h>

//configs (didnt create a config.h file as this is a pretty small project)
#define ARR_SIZE 16 //small so as to increase thread contention
#define NUM_THREADS 16 //my machine only supports upto 8 threads, so, not going too overboard here. but, higher no. of threads would be better for testing.
#define ITERS_PER_THREAD 1000000
#define INNER_ITERS 4

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

    for(long i = 0; i < ITERS_PER_THREAD; i++) {
        int s = xorshift(&rng) % ARR_SIZE;
        if(s > ARR_SIZE - INNER_ITERS) {s = ARR_SIZE - INNER_ITERS;}
        for(int i = s; i < s + INNER_ITERS; i++) {
            Node* cur = atomic_load_explicit(&test_arr[i], memory_order_relaxed);

            if(cur == NULL) {
                Node* n = malloc(sizeof(Node));
                n->value = tid;
                Node* expected = NULL;
                if(!atomic_compare_exchange_strong_explicit(&test_arr[i], &expected, n, memory_order_release, memory_order_relaxed)) {
                    free(n);
                }
            }
            else {
                Node* expected = cur;
                if(atomic_compare_exchange_strong_explicit(&test_arr[i], &expected, NULL, memory_order_acq_rel, memory_order_relaxed)) {
                    free(cur);
                }
            }
        }

        int s2 = xorshift(&rng) % ARR_SIZE;
        if(s2 > ARR_SIZE - INNER_ITERS) {s2 = ARR_SIZE - INNER_ITERS;}
        for(int i = s2; i < s2 + INNER_ITERS; i++) {
            Node* p = atomic_load_explicit(&test_arr[i], memory_order_relaxed);
            if(p != NULL) {
                //race window, if node freed before next line, UAF triggered
                volatile long v = p->value;
                (void)v; //no use of v except referencing the node p
            }
        }
    }
    return NULL;
}

int main(void) {
    for(int i = 0; i < ARR_SIZE; i++) {
        atomic_init(&test_arr[i], NULL);
    }
    
    pthread_t th[NUM_THREADS];
    for(long i = 0; i < NUM_THREADS; i++) {
        pthread_create(&th[i], NULL, worker, (void*)i);
    }
    for(long i = 0; i < NUM_THREADS; i++) {
        pthread_join(th[i], NULL);
    }

    printf("Run completed without triggering a UAF this time\n");
    return 0;
}
