#include <stdio.h>
#include <string.h>
#include "scheduler.h"

#define NUM_JOBS 5
#define TIME_QUANTUM 2

static void load_default_jobs(Job jobs[]) {
    Job defaults[NUM_JOBS] = {
        {1, 0, 5, 5, 2, "READY", 0, 0, 0},
        {2, 1, 3, 3, 1, "READY", 0, 0, 0},
        {3, 2, 8, 8, 4, "READY", 0, 0, 0},
        {4, 3, 6, 6, 1, "READY", 0, 0, 0}, // Tie priority with Job 2
        {5, 4, 2, 2, 3, "READY", 0, 0, 0}
    };
    memcpy(jobs, defaults, sizeof(defaults));
}

static void print_metrics(const char *algo_name, Job jobs[], int n, double *avg_tat, double *avg_wt) {
    double total_tat = 0, total_wt = 0;
    printf("\n--- %s Results ---\n", algo_name);
    printf("%-6s %-8s %-6s %-9s %-11s %-11s %-8s %-10s\n",
           "JobID", "Arrival", "Burst", "Priority", "Completion", "Turnaround", "Waiting", "Status");

    for (int i = 0; i < n; i++) {
        total_tat += jobs[i].turnaround_time;
        total_wt += jobs[i].waiting_time;
        printf("J%-5d %-8d %-6d %-9d %-11d %-11d %-8d %-10s\n",
               jobs[i].id, jobs[i].arrival_time, jobs[i].burst_time,
               jobs[i].priority, jobs[i].completion_time,
               jobs[i].turnaround_time, jobs[i].waiting_time, jobs[i].status);
    }

    *avg_tat = total_tat / n;
    *avg_wt = total_wt / n;
    printf("Average Turnaround Time: %.2f | Average Waiting Time: %.2f\n", *avg_tat, *avg_wt);
}

static void compute_fcfs(Job jobs[], int n, int print_output, double *avg_tat, double *avg_wt) {
    // Sort by arrival_time, tie-break by job ID
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (jobs[j].arrival_time > jobs[j + 1].arrival_time ||
               (jobs[j].arrival_time == jobs[j + 1].arrival_time && jobs[j].id > jobs[j + 1].id)) {
                Job tmp = jobs[j];
                jobs[j] = jobs[j + 1];
                jobs[j + 1] = tmp;
            }
        }
    }

    int current_time = 0;
    if (print_output) {
        printf("\n[FCFS Scheduling] Tie-breaking rule: Earliest Arrival Time, then lowest Job ID.\n");
        printf("Execution Order: ");
    }

    for (int i = 0; i < n; i++) {
        if (current_time < jobs[i].arrival_time) {
            current_time = jobs[i].arrival_time;
        }
        if (print_output) printf("J%d(%d->%d) ", jobs[i].id, current_time, current_time + jobs[i].burst_time);

        current_time += jobs[i].burst_time;
        jobs[i].completion_time = current_time;
        jobs[i].turnaround_time = jobs[i].completion_time - jobs[i].arrival_time;
        jobs[i].waiting_time = jobs[i].turnaround_time - jobs[i].burst_time;
        strcpy(jobs[i].status, "COMPLETED");
    }
    if (print_output) {
        printf("\n");
        print_metrics("FCFS", jobs, n, avg_tat, avg_wt);
    } else {
        double t = 0, w = 0;
        for (int i = 0; i < n; i++) { t += jobs[i].turnaround_time; w += jobs[i].waiting_time; }
        *avg_tat = t / n;
        *avg_wt = w / n;
    }
}

static void compute_rr(Job jobs[], int n, int print_output, double *avg_tat, double *avg_wt) {
    int queue[100], front = 0, rear = 0;
    int visited[NUM_JOBS] = {0};
    int completed = 0, current_time = 0;

    if (print_output) {
        printf("\n[Round Robin Scheduling] Time Quantum Q = %d | Tie-breaking: Arrival order / Ready queue FIFO.\n", TIME_QUANTUM);
        printf("Execution Order: ");
    }

    // Enqueue initial jobs arriving at time 0
    for (int i = 0; i < n; i++) {
        if (jobs[i].arrival_time <= current_time && !visited[i]) {
            queue[rear++] = i;
            visited[i] = 1;
        }
    }

    while (completed < n) {
        if (front == rear) {
            current_time++;
            for (int i = 0; i < n; i++) {
                if (jobs[i].arrival_time <= current_time && !visited[i]) {
                    queue[rear++] = i;
                    visited[i] = 1;
                }
            }
            continue;
        }

        int idx = queue[front++];
        int exec_time = (jobs[idx].remaining_time < TIME_QUANTUM) ? jobs[idx].remaining_time : TIME_QUANTUM;

        if (print_output) printf("J%d(%d->%d) ", jobs[idx].id, current_time, current_time + exec_time);

        current_time += exec_time;
        jobs[idx].remaining_time -= exec_time;

        // Enqueue newly arrived jobs during this slice
        for (int i = 0; i < n; i++) {
            if (jobs[i].arrival_time <= current_time && !visited[i]) {
                queue[rear++] = i;
                visited[i] = 1;
            }
        }

        if (jobs[idx].remaining_time == 0) {
            completed++;
            jobs[idx].completion_time = current_time;
            jobs[idx].turnaround_time = jobs[idx].completion_time - jobs[idx].arrival_time;
            jobs[idx].waiting_time = jobs[idx].turnaround_time - jobs[idx].burst_time;
            strcpy(jobs[idx].status, "COMPLETED");
        } else {
            queue[rear++] = idx;
        }
    }

    if (print_output) {
        printf("\n");
        print_metrics("Round Robin (Q=2)", jobs, n, avg_tat, avg_wt);
    } else {
        double t = 0, w = 0;
        for (int i = 0; i < n; i++) { t += jobs[i].turnaround_time; w += jobs[i].waiting_time; }
        *avg_tat = t / n;
        *avg_wt = w / n;
    }
}

static void compute_priority(Job jobs[], int n, int print_output, double *avg_tat, double *avg_wt) {
    int done[NUM_JOBS] = {0};
    int completed = 0, current_time = 0;

    if (print_output) {
        printf("\n[Non-Preemptive Priority Scheduling] Lower value = Higher priority.\n");
        printf("Tie-breaking rule: Earliest Arrival Time, then lowest Job ID.\n");
        printf("Execution Order: ");
    }

    while (completed < n) {
        int best = -1;
        for (int i = 0; i < n; i++) {
            if (!done[i] && jobs[i].arrival_time <= current_time) {
                if (best == -1 ||
                    jobs[i].priority < jobs[best].priority ||
                   (jobs[i].priority == jobs[best].priority && jobs[i].arrival_time < jobs[best].arrival_time) ||
                   (jobs[i].priority == jobs[best].priority && jobs[i].arrival_time == jobs[best].arrival_time && jobs[i].id < jobs[best].id)) {
                    best = i;
                }
            }
        }

        if (best == -1) {
            current_time++;
            continue;
        }

        if (print_output) printf("J%d[P%d](%d->%d) ", jobs[best].id, jobs[best].priority, current_time, current_time + jobs[best].burst_time);

        current_time += jobs[best].burst_time;
        jobs[best].completion_time = current_time;
        jobs[best].turnaround_time = jobs[best].completion_time - jobs[best].arrival_time;
        jobs[best].waiting_time = jobs[best].turnaround_time - jobs[best].burst_time;
        strcpy(jobs[best].status, "COMPLETED");
        done[best] = 1;
        completed++;
    }

    if (print_output) {
        printf("\n");
        print_metrics("Non-Preemptive Priority", jobs, n, avg_tat, avg_wt);
    } else {
        double t = 0, w = 0;
        for (int i = 0; i < n; i++) { t += jobs[i].turnaround_time; w += jobs[i].waiting_time; }
        *avg_tat = t / n;
        *avg_wt = w / n;
    }
}

void run_fcfs_demo(void) {
    Job jobs[NUM_JOBS];
    double tat, wt;
    load_default_jobs(jobs);
    compute_fcfs(jobs, NUM_JOBS, 1, &tat, &wt);
}

void run_rr_demo(void) {
    Job jobs[NUM_JOBS];
    double tat, wt;
    load_default_jobs(jobs);
    compute_rr(jobs, NUM_JOBS, 1, &tat, &wt);
}

void run_priority_demo(void) {
    Job jobs[NUM_JOBS];
    double tat, wt;
    load_default_jobs(jobs);
    compute_priority(jobs, NUM_JOBS, 1, &tat, &wt);
}

void run_scheduling_comparison(void) {
    Job jobs[NUM_JOBS];
    double fcfs_tat, fcfs_wt, rr_tat, rr_wt, prio_tat, prio_wt;

    load_default_jobs(jobs);
    compute_fcfs(jobs, NUM_JOBS, 0, &fcfs_tat, &fcfs_wt);

    load_default_jobs(jobs);
    compute_rr(jobs, NUM_JOBS, 0, &rr_tat, &rr_wt);

    load_default_jobs(jobs);
    compute_priority(jobs, NUM_JOBS, 0, &prio_tat, &prio_wt);

    printf("\n=== CPU Scheduling Policy Comparison (Same 5-Job Workload) ===\n");
    printf("%-28s %-22s %-20s\n", "Scheduling Policy", "Avg Turnaround Time", "Avg Waiting Time");
    printf("----------------------------------------------------------------------\n");
    printf("%-28s %-22.2f %-20.2f\n", "FCFS", fcfs_tat, fcfs_wt);
    printf("%-28s %-22.2f %-20.2f\n", "Round Robin (Q = 2)", rr_tat, rr_wt);
    printf("%-28s %-22.2f %-20.2f\n", "Non-Preemptive Priority", prio_tat, prio_wt);
}
