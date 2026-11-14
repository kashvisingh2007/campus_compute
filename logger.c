#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "logger.h"

#define BUFFER_SIZE 5
#define NUM_PRODUCERS 2
#define MSGS_PER_PRODUCER 4
#define LOG_FILE "campus_compute.log"

typedef struct {
    char entries[BUFFER_SIZE][128];
    int in;
    int out;
    int count;
    int shutdown;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} LogQueue;

static LogQueue queue;

static void enqueue_log(const char *msg) {
    pthread_mutex_lock(&queue.mutex);
    while (queue.count == BUFFER_SIZE && !queue.shutdown) {
        pthread_cond_wait(&queue.not_full, &queue.mutex);
    }
    if (!queue.shutdown) {
        strncpy(queue.entries[queue.in], msg, 127);
        queue.entries[queue.in][127] = '\0';
        queue.in = (queue.in + 1) % BUFFER_SIZE;
        queue.count++;
        printf("[Producer] Queued: %s (Buffer count: %d/%d)\n", msg, queue.count, BUFFER_SIZE);
        pthread_cond_signal(&queue.not_empty);
    }
    pthread_mutex_unlock(&queue.mutex);
}

static void *producer_thread(void *arg) {
    int id = *(int *)arg;
    for (int i = 1; i <= MSGS_PER_PRODUCER; i++) {
        char msg[128];
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        snprintf(msg, sizeof(msg), "[%02d:%02d:%02d] Producer-%d Event #%d: Job telemetry recorded",
                 t->tm_hour, t->tm_min, t->tm_sec, id, i);
        enqueue_log(msg);
        usleep(100000); // 100ms delay to demonstrate concurrency
    }
    printf("[Producer-%d] Finished producing events and exiting.\n", id);
    return NULL;
}

static void *consumer_thread(void *arg) {
    (void)arg;
    FILE *fp = fopen(LOG_FILE, "a");
    if (!fp) {
        perror("fopen log file");
        return NULL;
    }

    while (1) {
        pthread_mutex_lock(&queue.mutex);
        while (queue.count == 0 && !queue.shutdown) {
            pthread_cond_wait(&queue.not_empty, &queue.mutex);
        }

        if (queue.count == 0 && queue.shutdown) {
            pthread_mutex_unlock(&queue.mutex);
            break;
        }

        char entry[128];
        strncpy(entry, queue.entries[queue.out], sizeof(entry));
        queue.out = (queue.out + 1) % BUFFER_SIZE;
        queue.count--;
        printf("[Consumer] Wrote to %s: %s (Buffer count: %d/%d)\n",
               LOG_FILE, entry, queue.count, BUFFER_SIZE);
        pthread_cond_signal(&queue.not_full);
        pthread_mutex_unlock(&queue.mutex);

        fprintf(fp, "%s\n", entry);
        fflush(fp);
        usleep(150000); // Slightly slower consumer to exercise bounded buffer
    }

    fclose(fp);
    printf("[Consumer] Queue drained and shutdown signal received. Closing log file and exiting.\n");
    return NULL;
}

void run_logger_demo(void) {
    printf("\n=== Concurrent Activity Logger (Bounded Producer-Consumer Queue) ===\n");
    printf("Buffer Capacity: %d | Producers: %d | Consumer: 1 | Output: %s\n\n",
           BUFFER_SIZE, NUM_PRODUCERS, LOG_FILE);

    queue.in = 0;
    queue.out = 0;
    queue.count = 0;
    queue.shutdown = 0;
    pthread_mutex_init(&queue.mutex, NULL);
    pthread_cond_init(&queue.not_full, NULL);
    pthread_cond_init(&queue.not_empty, NULL);

    pthread_t prod[NUM_PRODUCERS], cons;
    int prod_ids[NUM_PRODUCERS] = {1, 2};

    pthread_create(&cons, NULL, consumer_thread, NULL);
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_create(&prod[i], NULL, producer_thread, &prod_ids[i]);
    }

    // Wait for all producers to finish
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(prod[i], NULL);
    }

    // Signal consumer to drain remaining items and terminate cleanly
    pthread_mutex_lock(&queue.mutex);
    queue.shutdown = 1;
    pthread_cond_broadcast(&queue.not_empty);
    pthread_mutex_unlock(&queue.mutex);

    pthread_join(cons, NULL);

    pthread_mutex_destroy(&queue.mutex);
    pthread_cond_destroy(&queue.not_full);
    pthread_cond_destroy(&queue.not_empty);

    printf("\n[Sync & Cleanup] All producer and consumer threads joined cleanly. Mutex/condvars destroyed.\n");
}
