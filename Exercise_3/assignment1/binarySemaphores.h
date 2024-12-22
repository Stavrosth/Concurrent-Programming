// Created by: Dimitris Voitsidis, Iordana Gaisidou, Stavros Stathoudakis
#ifndef BINARY_SEMAPHORES_H
#define BINARY_SEMAPHORES_H

#include "MonitorsForBinarySemaphores.h"

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

//Struct representing a semaphore
typedef struct {
    sem_monitor_var_t sem_monitor;
    int sem_id, sem_val;    
} mysem_t;

// Functions for my semaphores
int mysem_init(mysem_t *s, int n);
int mysem_down(mysem_t *s);
int mysem_up(mysem_t *s);
int mysem_destroy(mysem_t *s);
int mysem_error(int returnVal, char *text);

#endif // BINARY_SEMAPHORES_H