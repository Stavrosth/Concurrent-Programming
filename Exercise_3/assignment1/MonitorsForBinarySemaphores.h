// Created by: Dimitris Voitsidis, Iordana Gaisidou, Stavros Stathoudakis
#ifndef MONITORS_FOR_BINARY_SEMAPHORES_H
#define MONITORS_FOR_BINARY_SEMAPHORES_H

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

void my_pthread_error(int errorVal);

// Struct for the read-write monitor
typedef struct {
    pthread_mutex_t mtx;
    pthread_cond_t readQ, writeQ;

    int readers, readers_waiting;
    int writers, writers_waiting;
} rw_monitor_var_t;

void rw_monitor_start_read();
void rw_monitor_stop_read();
void rw_monitor_start_write();
void rw_monitor_stop_write();

// Struct for the semaphore monitor
typedef struct {
    pthread_mutex_t mtx;
    pthread_cond_t semQ;
} sem_monitor_var_t;

// Semaphore Monitor Functions
void sem_monitor_semaphore_init(sem_monitor_var_t *sem_monitor);
void sem_monitor_semaphore_down(sem_monitor_var_t *sem_monitor, int *sem_val);
int sem_monitor_semaphore_up(sem_monitor_var_t *sem_monitor, int *sem_val);

#endif // MONITORS_FOR_BINARY_SEMAPHORES_H