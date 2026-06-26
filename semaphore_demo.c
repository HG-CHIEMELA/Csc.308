/*
 * Practical Session 3: Semaphore Implementation in C
 * CSC 308 - Operating Systems | Week 6
 *
 * Objective: Compare mutex locks and semaphores for protecting shared
 * resources, and experiment with counting semaphores.
 *
 * Modes (pass as argv[1]):
 *   binary    -> protect a shared counter with a binary semaphore (default)
 *   counting  -> allow N threads to access a "resource pool" simultaneously
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define NUM_THREADS 6
#define INCREMENTS_PER_THREAD 100000
#define POOL_SIZE 3 /* e.g. 3 simultaneous database connections */

/* ---------- Binary semaphore mode: shared counter ---------- */
long shared_counter = 0;
sem_t binary_sem;

void *increment_with_semaphore(void *arg) {
    int id = *(int *)arg;
    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        sem_wait(&binary_sem);
        shared_counter++;
        sem_post(&binary_sem);
    }
    printf("Thread %d finished.\n", id);
    return NULL;
}

void run_binary_semaphore_demo(void) {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    sem_init(&binary_sem, 0, 1); /* binary: only 1 thread at a time */

    printf("=== Binary Semaphore Demo (acts like a mutex) ===\n");
    long expected = (long)NUM_THREADS * INCREMENTS_PER_THREAD;
    printf("Threads: %d | Expected final value: %ld\n\n", NUM_THREADS, expected);

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, increment_with_semaphore, &ids[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nFinal counter: %ld (expected %ld) -> %s\n",
           shared_counter, expected,
           shared_counter == expected ? "CORRECT" : "INCORRECT");

    sem_destroy(&binary_sem);
}

/* ---------- Counting semaphore mode: resource pool ---------- */
sem_t pool_sem;

void *use_resource_pool(void *arg) {
    int id = *(int *)arg;

    printf("Thread %d waiting to access the resource pool...\n", id);
    sem_wait(&pool_sem);

    printf("Thread %d ACQUIRED a resource (pool slot in use).\n", id);
    sleep(1); /* simulate work using the resource, e.g. a DB connection */
    printf("Thread %d RELEASING its resource.\n", id);

    sem_post(&pool_sem);
    return NULL;
}

void run_counting_semaphore_demo(void) {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    sem_init(&pool_sem, 0, POOL_SIZE); /* counting: up to POOL_SIZE concurrent */

    printf("=== Counting Semaphore Demo (resource pool) ===\n");
    printf("Pool size: %d | Threads competing for access: %d\n\n",
           POOL_SIZE, NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, use_resource_pool, &ids[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nAt most %d threads ever held a resource simultaneously.\n", POOL_SIZE);
    sem_destroy(&pool_sem);
}

int main(int argc, char *argv[]) {
    const char *mode = (argc > 1) ? argv[1] : "binary";

    if (strcmp(mode, "counting") == 0) {
        run_counting_semaphore_demo();
    } else {
        run_binary_semaphore_demo();
    }

    printf("\n--- Key Takeaway ---\n");
    printf("Semaphores are more flexible (support a count > 1, i.e. counting\n");
    printf("semaphores), while mutexes are simpler and intended strictly for\n");
    printf("binary mutual exclusion with ownership semantics.\n");

    printf("\n--- Discussion ---\n");
    printf("Use a counting semaphore when N (>1) threads may legitimately use a\n");
    printf("resource pool at once (e.g. a fixed-size DB connection pool, a fixed\n");
    printf("number of printer spooler slots). Use a mutex/binary semaphore when\n");
    printf("only ONE thread may hold the resource at a time.\n");

    return 0;
}
