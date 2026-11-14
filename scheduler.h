#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef struct {
    int id;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority;          // Lower number = higher priority
    char status[16];
    int completion_time;
    int turnaround_time;
    int waiting_time;
} Job;

void run_fcfs_demo(void);
void run_rr_demo(void);
void run_priority_demo(void);
void run_scheduling_comparison(void);

#endif
