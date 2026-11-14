#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <unistd.h>
#include "ipc_shm.h"

#define SHM_NAME "/campus_compute_shm"
#define SEM_NAME "/campus_compute_sem"

typedef struct {
    int job_id;
    int cpu_cores_allocated;
    double cpu_utilization;
    char status_msg[128];
} SharedTelemetry;

void run_ipc_demo(void) {
    printf("\n=== POSIX Shared Memory & Named Semaphore IPC ===\n");

    // Pre-clean any stale IPC objects from a prior interrupted run
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return;
    }

    if (ftruncate(shm_fd, sizeof(SharedTelemetry)) == -1) {
        perror("ftruncate");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return;
    }

    SharedTelemetry *shm_ptr = mmap(NULL, sizeof(SharedTelemetry),
                                    PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return;
    }

    // Create named semaphore initialized to 0 (locked until parent writes)
    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        munmap(shm_ptr, sizeof(SharedTelemetry));
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return;
    }

    printf("[IPC Setup] Created POSIX shared memory '%s' and semaphore '%s'.\n", SHM_NAME, SEM_NAME);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
    } else if (pid == 0) {
        // Child process: wait on semaphore, then read from shared memory
        printf("[Child PID %d] Waiting on named semaphore '%s'...\n", getpid(), SEM_NAME);
        if (sem_wait(sem) == -1) {
            perror("sem_wait");
            exit(1);
        }

        printf("[Child PID %d] Semaphore acquired! Reading shared memory data:\n", getpid());
        printf("  -> Job ID: %d | Cores: %d | Utilization: %.1f%% | Message: \"%s\"\n",
               shm_ptr->job_id, shm_ptr->cpu_cores_allocated,
               shm_ptr->cpu_utilization, shm_ptr->status_msg);

        munmap(shm_ptr, sizeof(SharedTelemetry));
        close(shm_fd);
        sem_close(sem);
        exit(0);
    } else {
        // Parent process: populate shared memory and post semaphore
        usleep(150000); // Let child block on sem_wait first to prove synchronization
        shm_ptr->job_id = 104;
        shm_ptr->cpu_cores_allocated = 4;
        shm_ptr->cpu_utilization = 87.5;
        snprintf(shm_ptr->status_msg, sizeof(shm_ptr->status_msg),
                 "Compute node telemetry synced via POSIX shm");

        printf("[Parent PID %d] Wrote telemetry for Job #%d to shared memory.\n", getpid(), shm_ptr->job_id);
        printf("[Parent PID %d] Posting semaphore '%s' to wake child process.\n", getpid(), SEM_NAME);
        sem_post(sem);

        int status;
        waitpid(pid, &status, 0);
        printf("[Parent PID %d] Child exited with status %d.\n", getpid(), WEXITSTATUS(status));
    }

    // Clean up all created IPC resources
    munmap(shm_ptr, sizeof(SharedTelemetry));
    close(shm_fd);
    shm_unlink(SHM_NAME);
    sem_close(sem);
    sem_unlink(SEM_NAME);

    printf("[IPC Cleanup] Unmapped memory and unlinked '%s' and '%s' cleanly.\n", SHM_NAME, SEM_NAME);
}
