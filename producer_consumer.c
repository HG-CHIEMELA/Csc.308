/*
 * Practical Session 2: Producer-Consumer Simulation
 * CSC 308 - Operating Systems | Week 6
 *
 * Objective: Implement the Producer-Consumer problem using POSIX semaphores.
 *
 * Semaphores used:
 *   mutex (binary, init 1) -> protects access to the circular buffer
 *   empty (counting, init N) -> counts empty slots in the buffer
 *   full  (counting, init 0) -> counts filled slots in the buffer
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_ITEMS_TO_PRODUCE 15

int buffer[BUFFER_SIZE];
int in = 0;  /* next write index */
int out = 0; /* next read index */

sem_t mutex;  /* binary semaphore protecting the buffer */
sem_t empty;  /* counts empty slots */
sem_t full;   /* counts full slots */

void print_buffer_status(const char *actor, int item, const char *action) {
    int filled;
    sem_getvalue(&full, &filled);
    printf("[%s] %s item %d | buffer occupancy: %d/%d\n",
           actor, action, item, filled, BUFFER_SIZE);
}

void *producer(void *arg) {
    for (int i = 1; i <= NUM_ITEMS_TO_PRODUCE; i++) {
        usleep(100000); /* simulate variable production time (100ms) */

        sem_wait(&empty);      /* wait for an empty slot */
        sem_wait(&mutex);      /* lock the buffer */

        buffer[in] = i;
        print_buffer_status("Producer", i, "produced");
        in = (in + 1) % BUFFER_SIZE;

        sem_post(&mutex);      /* unlock the buffer */
        sem_post(&full);       /* signal a new full slot */
    }
    printf("[Producer] Done producing %d items.\n", NUM_ITEMS_TO_PRODUCE);
    return NULL;
}

void *consumer(void *arg) {
    for (int i = 1; i <= NUM_ITEMS_TO_PRODUCE; i++) {
        usleep(200000); /* consumer is slower than producer (200ms) */

        sem_wait(&full);       /* wait for a full slot */
        sem_wait(&mutex);      /* lock the buffer */

        int item = buffer[out];
        print_buffer_status("Consumer", item, "consumed");
        out = (out + 1) % BUFFER_SIZE;

        sem_post(&mutex);      /* unlock the buffer */
        sem_post(&empty);      /* signal a new empty slot */
    }
    printf("[Consumer] Done consuming %d items.\n", NUM_ITEMS_TO_PRODUCE);
    return NULL;
}

int main(void) {
    pthread_t producer_thread, consumer_thread;

    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);

    printf("=== Producer-Consumer Simulation ===\n");
    printf("Buffer size: %d | Items to produce/consume: %d\n\n",
           BUFFER_SIZE, NUM_ITEMS_TO_PRODUCE);

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    sem_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    printf("\nSimulation complete. No overflow, no underflow, no race conditions.\n");
    return 0;
}
