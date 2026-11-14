#include <stdio.h>
#include <stdbool.h>
#include "bankers.h"

#define P 5 // Number of processes
#define R 3 // Number of resource types (CPU cores, Memory blocks, I/O channels)

static void run_safety_check(const char *label, int avail[R], int max[P][R], int alloc[P][R]) {
    int need[P][R];
    int work[R];
    bool finish[P] = {false};
    int safe_seq[P];
    int count = 0;

    printf("\n=== Banker's Safety Analysis: %s ===\n", label);

    // Compute Need = Max - Allocation
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < R; j++) {
            need[i][j] = max[i][j] - alloc[i][j];
        }
    }

    for (int j = 0; j < R; j++) {
        work[j] = avail[j];
    }

    printf("Available Resources: [CPU=%d, MEM=%d, IO=%d]\n\n", avail[0], avail[1], avail[2]);
    printf("%-8s %-15s %-15s %-15s\n", "Process", "Allocation", "Maximum", "Need");
    for (int i = 0; i < P; i++) {
        printf("P%-7d [%d, %d, %d]       [%d, %d, %d]       [%d, %d, %d]\n",
               i,
               alloc[i][0], alloc[i][1], alloc[i][2],
               max[i][0], max[i][1], max[i][2],
               need[i][0], need[i][1], need[i][2]);
    }

    // Banker's Safety Loop
    while (count < P) {
        bool found = false;
        for (int i = 0; i < P; i++) {
            if (!finish[i]) {
                bool can_allocate = true;
                for (int j = 0; j < R; j++) {
                    if (need[i][j] > work[j]) {
                        can_allocate = false;
                        break;
                    }
                }

                if (can_allocate) {
                    for (int j = 0; j < R; j++) {
                        work[j] += alloc[i][j];
                    }
                    safe_seq[count++] = i;
                    finish[i] = true;
                    found = true;
                }
            }
        }

        if (!found) {
            printf("\nResult: UNSAFE STATE DETECTED!\n");
            printf("Reason: Remaining unfinished processes have Need > Available work resources.\n");
            return;
        }
    }

    printf("\nResult: SAFE STATE VERIFIED.\n");
    printf("Safe Execution Sequence: < ");
    for (int i = 0; i < P; i++) {
        printf("P%d%s", safe_seq[i], (i < P - 1) ? " -> " : " ");
    }
    printf(">\n");
}

void run_bankers_safe_demo(void) {
    int avail[R] = {3, 3, 2};
    int max[P][R] = {
        {7, 5, 3},
        {3, 2, 2},
        {9, 0, 2},
        {2, 2, 2},
        {4, 3, 3}
    };
    int alloc[P][R] = {
        {0, 1, 0},
        {2, 0, 0},
        {3, 0, 2},
        {2, 1, 1},
        {0, 0, 2}
    };
    run_safety_check("Safe Resource State Test", avail, max, alloc);
}

void run_bankers_unsafe_demo(void) {
    // Reduced available resources so no process can satisfy its Need
    int avail[R] = {1, 0, 0};
    int max[P][R] = {
        {7, 5, 3},
        {3, 2, 2},
        {9, 0, 2},
        {2, 2, 2},
        {4, 3, 3}
    };
    int alloc[P][R] = {
        {0, 1, 0},
        {2, 0, 0},
        {3, 0, 2},
        {2, 1, 1},
        {0, 0, 2}
    };
    run_safety_check("Unsafe Resource State Test", avail, max, alloc);
}
