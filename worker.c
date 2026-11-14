#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include "worker.h"

void run_worker_normal_demo(void) {
    printf("\n=== Worker Process: Normal Completion (fork + waitpid) ===\n");

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    } else if (pid == 0) {
        printf("[Worker Child PID %d] Executing compute task...\n", getpid());
        long sum = 0;
        for (long i = 1; i <= 1000000; i++) {
            sum += i;
        }
        printf("[Worker Child PID %d] Task complete (checksum=%ld). Exiting with status 0.\n", getpid(), sum);
        exit(0);
    } else {
        int status;
        printf("[Parent PID %d] Spawned worker child PID %d. Waiting via waitpid()...\n", getpid(), pid);
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("[Parent PID %d] Result: Worker PID %d completed NORMALLY (exit code %d).\n",
                   getpid(), pid, WEXITSTATUS(status));
        } else {
            printf("[Parent PID %d] Unexpected worker status.\n", getpid());
        }
    }
}

void run_worker_failure_demo(void) {
    printf("\n=== Worker Process: Controlled Simulated Failure Detection ===\n");

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    } else if (pid == 0) {
        printf("[Worker Child PID %d] Running simulation job...\n", getpid());
        usleep(100000);
        printf("[Worker Child PID %d] [SIMULATED FAILURE] Injecting controlled non-zero exit (code 42).\n", getpid());
        exit(42);
    } else {
        int status;
        printf("[Parent PID %d] Monitoring worker child PID %d via waitpid()...\n", getpid(), pid);
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("[Parent PID %d] FAILURE DETECTED: Worker PID %d terminated abnormally with simulated exit code %d.\n",
                   getpid(), pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("[Parent PID %d] FAILURE DETECTED: Worker PID %d killed by signal %d (%s).\n",
                   getpid(), pid, WTERMSIG(status), strsignal(WTERMSIG(status)));
        }
    }
}

void run_resource_limit_demo(void) {
    printf("\n=== Resource Limit Demonstration (setrlimit RLIMIT_CPU) ===\n");

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    } else if (pid == 0) {
        struct rlimit rl;
        rl.rlim_cur = 1; // 1-second soft CPU limit
        rl.rlim_max = 2; // 2-second hard CPU limit

        if (setrlimit(RLIMIT_CPU, &rl) == -1) {
            perror("setrlimit");
            exit(1);
        }

        printf("[Worker Child PID %d] Applied RLIMIT_CPU (soft=1s, hard=2s). Starting CPU-intensive loop...\n", getpid());
        fflush(stdout);

        // Burn CPU cycles to exceed the 1-second soft limit
        volatile unsigned long counter = 0;
        while (1) {
            counter++;
        }
        exit(0);
    } else {
        int status;
        printf("[Parent PID %d] Waiting for CPU-limited worker PID %d...\n", getpid(), pid);
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return;
        }

        if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            printf("[Parent PID %d] Observed Effect: Kernel terminated worker PID %d via signal %d (%s) after exceeding RLIMIT_CPU.\n",
                   getpid(), pid, sig, strsignal(sig));
        } else if (WIFEXITED(status)) {
            printf("[Parent PID %d] Worker exited with status %d.\n", getpid(), WEXITSTATUS(status));
        }
    }
}
