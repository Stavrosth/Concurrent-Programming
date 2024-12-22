#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct monitor{
    pthread_mutex_t mtx;
    pthread_cond_t trainQ;
    pthread_cond_t disembarkQ;
    pthread_cond_t boardingQ;
} monitor_var_t;

typedef enum {WAITING, TRAVELING, TRAVELED} travelE;

// Struct used to pass arguments in the passenger thread
typedef struct {
    int train_capacity; // Train capacity
    bool* destroy; // Flag to terminate train
    travelE* travel; // Flag to indicate if the train is traveling
    int* boarded_passengers; // Number of passengers boarded
    int* waiting_passengers; // Number of passengers waiting
    monitor_var_t* monitor; // Monitor for train
} TargsS;

// Functions for monitors
void init_monitor(monitor_var_t *monitor);
void my_pthread_error(int errorVal);