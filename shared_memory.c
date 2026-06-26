/*
 * Practical Session 4: Shared Memory Programming
 * CSC 308 - Operating Systems | Week 6
 *
 * Objective: Implement inter-process communication using shared memory.
 *
 * A parent process writes data into a shared memory segment; a forked
 * child process reads it. Two named semaphores synchronize access so the
 * child never reads before the parent has finished writing.
 *
 * Steps demonstrated:
 *   1. shmget() to create a shared memory segment
 *   2. shmat()  to attach memory in both parent and child
 *   3. Write from one process, read from another
 *   4. Semaphores synchronize access
 *   5. shmdt() and shmctl(IPC_RMID) to clean up
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>

#define SHM_SIZE 1024
#define SEM_WRITE_DONE "/sess4_write_done"
#define SEM_READ_DONE  "/sess4_read_done"

typedef struct {
    int message_id;
    char text[256];
} shared_data_t;

int main(void) {
    /* Use line-buffered stdout so fork() doesn't duplicate buffered output
     * between the parent and child processes. */
    setvbuf(stdout, NULL, _IOLBF, 0);

    /* --- Step 1: Create the shared memory segment --- */
    key_t key = ftok("shared_memory.c", 65); /* unique key derived from this file */
    int shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    printf("[Setup] Shared memory segment created (shmid=%d).\n", shmid);

    /* Named semaphores so parent and child can synchronize handoffs */
    sem_unlink(SEM_WRITE_DONE);
    sem_unlink(SEM_READ_DONE);
    sem_t *write_done = sem_open(SEM_WRITE_DONE, O_CREAT, 0666, 0);
    sem_t *read_done  = sem_open(SEM_READ_DONE,  O_CREAT, 0666, 1);

    if (write_done == SEM_FAILED || read_done == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid > 0) {
        /* ---------------- PARENT: writer ---------------- */
        void *shm_ptr = shmat(shmid, NULL, 0);
        if (shm_ptr == (void *)-1) {
            perror("parent shmat failed");
            exit(1);
        }

        shared_data_t *data = (shared_data_t *)shm_ptr;

        for (int i = 1; i <= 3; i++) {
            sem_wait(read_done); /* wait until child has consumed the previous message */

            data->message_id = i;
            snprintf(data->text, sizeof(data->text),
                     "Message %d from parent (pid=%d)", i, getpid());

            printf("[Parent] Wrote: \"%s\"\n", data->text);

            sem_post(write_done); /* signal: data is ready to read */
        }

        wait(NULL); /* wait for child to exit */

        /* --- Step 5: Clean up --- */
        shmdt(shm_ptr);
        shmctl(shmid, IPC_RMID, NULL);
        sem_close(write_done);
        sem_close(read_done);
        sem_unlink(SEM_WRITE_DONE);
        sem_unlink(SEM_READ_DONE);

        printf("[Parent] Shared memory segment removed. Done.\n");

    } else {
        /* ---------------- CHILD: reader ---------------- */
        void *shm_ptr = shmat(shmid, NULL, 0);
        if (shm_ptr == (void *)-1) {
            perror("child shmat failed");
            exit(1);
        }

        shared_data_t *data = (shared_data_t *)shm_ptr;

        for (int i = 1; i <= 3; i++) {
            sem_wait(write_done); /* wait until parent has written new data */

            printf("[Child]  Read:  \"%s\" (message_id=%d)\n", data->text, data->message_id);

            sem_post(read_done); /* signal: buffer is free for the next write */
        }

        shmdt(shm_ptr);
        sem_close(write_done);
        sem_close(read_done);
        exit(0);
    }

    return 0;
}
