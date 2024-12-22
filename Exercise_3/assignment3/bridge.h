// Created by: Dimitris Voitsidis, Iordana Gaisidou, Stavros Stathoudakis

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    pthread_mutex_t mtx;
    pthread_cond_t redQ;
    pthread_cond_t blueQ;
} monitor_var_t;

typedef enum { RED, BLUE } carColorE;

// Struct used to pass arguments in pthread_create
typedef struct {
    int maxCarsCrossing;
    int maxCarsCrossed;
    int *carsCrossing;
    int *carsCrossed;
    int *waitingUs;
    int *waitingOthers;
    carColorE color;
    carColorE *color_bridge;
    monitor_var_t *monitor;
}carArgS;

// Functions for monitors
void init_monitor(monitor_var_t *monitor);
void my_pthread_error(int errorVal);
void *car(void *Cargs);