/*
 * Practical Session 1: Mutex Lock Demonstration
 * CSC 308 - Operating Systems | Week 6
 *
 * Objective: Demonstrate mutual exclusion using pthread mutex locks.
 *
 * Usage:
 *   ./mutex_demo            -> runs WITH mutex protection
 *   ./mutex_demo --nomutex  -> runs WITHOUT mutex protection (shows race condition)
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define NUM_THREADS 4
#define INCREMENTS_PER_THREAD 100000

long shared_counter = 0;
int use_mutex = 1; /* toggled by command-line flag */

pthread_mutex_t counter_mutex;

void *increment_counter(void *arg) {
    int thread_id = *(int *)arg;

    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        if (use_mutex) {
            pthread_mutex_lock(&counter_mutex);
            shared_counter++;
            pthread_mutex_unlock(&counter_mutex);
        } else {
            /* Unprotected increment: read-modify-write race condition */
            shared_counter++;
        }
    }

    printf("Thread %d finished incrementing.\n", thread_id);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "--nomutex") == 0) {
        use_mutex = 0;
    }

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    if (use_mutex) {
        pthread_mutex_init(&counter_mutex, NULL);
        printf("=== Running WITH mutex protection ===\n");
    } else {
        printf("=== Running WITHOUT mutex protection ===\n");
    }

    long expected = (long)NUM_THREADS * INCREMENTS_PER_THREAD;
    printf("Threads: %d | Increments per thread: %d | Expected final value: %ld\n\n",
           NUM_THREADS, INCREMENTS_PER_THREAD, expected);

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, increment_counter, &thread_ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nFinal counter value: %ld\n", shared_counter);
    printf("Expected value:      %ld\n", expected);

    if (shared_counter == expected) {
        printf("Result: CORRECT (no lost updates)\n");
    } else {
        printf("Result: INCORRECT (%ld updates lost due to race condition)\n",
               expected - shared_counter);
    }

    if (use_mutex) {
        pthread_mutex_destroy(&counter_mutex);
    }

    return 0;
}
