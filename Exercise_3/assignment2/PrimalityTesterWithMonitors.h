#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct {
    pthread_mutex_t mtx;
    pthread_cond_t mainQ;
    pthread_cond_t workerQ;
} monitor_var_t;

// Struct used to pass arguments in pthread_create
typedef struct {
    int* job; // Job to be done
    bool* job_taken; // Flag to signal job taken
    bool* terminate; // Flag to terminate worker
    monitor_var_t* monitor; // monitor
}workerArgS;

void my_pthread_error(int errorVal);
void init_monitor(monitor_var_t *monitor);
void* worker(void* Wargs);