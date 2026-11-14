# CampusCompute: Intelligent Job Scheduling and Resource Management

**Student SRN:** PES1UG25CS245 and PES1UG25CS202
**Course:** Operating Systems (UE25CS242A) — Level 3 Mini-Project

## Overview
`CampusCompute` is a modular C-based Linux system application demonstrating core Operating Systems concepts using POSIX APIs:
- **CPU Scheduling (`scheduler.c`, `scheduler.h`):** Implements FCFS, Round Robin (time quantum = 2), Non-Preemptive Priority Scheduling, and comparative performance metrics.
- **Concurrent Activity Logger (`logger.c`, `logger.h`):** Implements a bounded circular producer-consumer queue synchronized with `pthread_mutex_t` and `pthread_cond_t` writing timestamped logs to `campus_compute.log`.
- **Inter-Process Communication (`ipc_shm.c`, `ipc_shm.h`):** Coordinates parent-child telemetry exchange using POSIX shared memory (`/campus_compute_shm`) and a named semaphore (`/campus_compute_sem`).
- **Deadlock Avoidance (`bankers.c`, `bankers.h`):** Evaluates safe and unsafe resource allocation states using Banker's safety algorithm.
- **Worker Lifecycle & Resource Limits (`worker.c`, `worker.h`):** Manages child processes via `fork()` and `waitpid()` (normal completion and simulated failure) and enforces process resource limits via `setrlimit()` and `getrlimit()`.

## Prerequisites
- Ubuntu Linux (or WSL Ubuntu)
- GCC (`build-essential`)
- GNU Make (`make`)
- POSIX Threads (`-lpthread`) and POSIX Realtime (`-lrt`) libraries

## Build Instructions
Compile the project cleanly from source using the provided `Makefile`:
```bash
make clean && make
```

## Run Commands
- **Interactive CLI Menu:**
  ```bash
  ./campus_compute
  ```
- **CLI Help & Usage:**
  ```bash
  ./campus_compute --help
  ```
- **Integrated End-to-End Demonstration:**
  ```bash
  ./campus_compute --integrated
  ```

## Cleanup Commands
- **Remove compiled object files, executable, and log file:**
  ```bash
  make clean && rm -f campus_compute.log
  ```
- **Verify no leftover processes or POSIX IPC objects remain:**
  ```bash
  ps -ef | grep campus_compute
  ls -la /dev/shm | grep campus_compute
  ```
