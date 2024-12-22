// Created by: Dimitris Voitsidis, Iordana Gaisidou, Stavros Stathoudakis

#ifndef BINARY_SEMAPHORES_H
#define BINARY_SEMAPHORES_H

#include <sys/sem.h>
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdlib.h>

// Struct representing a semaphore
typedef struct semaphore{
    int semid;
} mysem_t;

// Functions for my semaphores
int mysem_init(mysem_t *s, int n);
int mysem_down(mysem_t *s);
int mysem_up(mysem_t *s);
int mysem_destroy(mysem_t *s);
int mysem_error(int returnVal, char *text);

#endif // BINARY_SEMAPHORES_H