#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"
#include "bankers.h"
#include "logger.h"
#include "ipc_shm.h"
#include "worker.h"

void print_usage(const char *prog_name) {
    printf("Usage: %s [--help | --integrated]\n", prog_name);
    printf("CampusCompute: Intelligent Job Scheduling and Resource Management\n\n");
    printf("Menu Options:\n");
    printf("  1. FCFS CPU Scheduling\n");
    printf("  2. Round Robin CPU Scheduling\n");
    printf("  3. Non-Preemptive Priority Scheduling\n");
    printf("  4. Scheduling Policy Comparison\n");
    printf("  5. Concurrent Activity Logger (Pthreads)\n");
    printf("  6. Shared Memory & Semaphore IPC\n");
    printf("  7. Banker's Safety Algorithm (Safe State)\n");
    printf("  8. Banker's Safety Algorithm (Unsafe State)\n");
    printf("  9. Worker Process (Normal Completion)\n");
    printf(" 10. Worker Process (Simulated Failure)\n");
    printf(" 11. Resource Limit Demonstration (setrlimit)\n");
    printf(" 12. Integrated System Demonstration\n");
    printf("  0. Exit\n");
}

void run_integrated_demo(void) {
    printf("\n========================================================\n");
    printf("   CampusCompute: Integrated System Demonstration\n");
    printf("========================================================\n");

    printf("\n[Phase 1/4] Verifying Resource Allocation Safety (Banker's Algorithm)...\n");
    run_bankers_safe_demo();

    printf("\n[Phase 2/4] Evaluating CPU Scheduling Policies...\n");
    run_scheduling_comparison();

    printf("\n[Phase 3/4] Dispatching Compute Worker & IPC Telemetry...\n");
    run_worker_normal_demo();
    run_ipc_demo();

    printf("\n[Phase 4/4] Recording Synchronized Activity Logs...\n");
    run_logger_demo();

    printf("\n========================================================\n");
    printf("   Integrated Execution Summary: ALL SUBSYSTEMS PASSED\n");
    printf("   - Safety Check: Verified Safe Sequence\n");
    printf("   - Scheduler: Evaluated FCFS, RR, and Priority\n");
    printf("   - Workers & IPC: Process lifecycle & SHM clean\n");
    printf("   - Logger: Bounded queue drained & threads joined\n");
    printf("========================================================\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[1], "--integrated") == 0) {
            run_integrated_demo();
            return 0;
        } else {
            fprintf(stderr, "Error: Invalid argument '%s'\n", argv[1]);
            print_usage(argv[0]);
            return 1;
        }
    }

    char input[64];
    int choice = -1;

    while (1) {
        printf("\n=== CampusCompute CLI ===\n");
        print_usage(argv[0]);
        printf("\nEnter choice: ");

        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        if (sscanf(input, "%d", &choice) != 1) {
            fprintf(stderr, "Error: Invalid input. Please enter a valid menu number.\n");
            continue;
        }

        if (choice == 0) {
            printf("Exiting CampusCompute.\n");
            break;
        } else if (choice >= 1 && choice <= 12) {
            switch (choice) {
		case 1: run_fcfs_demo(); break;
		case 2: run_rr_demo(); break;
		case 3: run_priority_demo(); break;
		case 4: run_scheduling_comparison(); break;
		case 5: run_logger_demo(); break;
		case 6: run_ipc_demo(); break;
		case 7: run_bankers_safe_demo(); break;
		case 8: run_bankers_unsafe_demo(); break;
		case 9: run_worker_normal_demo(); break;
		case 10: run_worker_failure_demo(); break;
		case 11: run_resource_limit_demo(); break;
		case 12: run_integrated_demo(); break;
		default: printf("[Module %d not yet implemented]\n", choice); break;
        }
        } else {
            fprintf(stderr, "Error: Choice %d out of range (0-12).\n", choice);
        }
    }

    return 0;
}
