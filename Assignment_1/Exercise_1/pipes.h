// Created by: Dimitris Voitsidis, Iordana Gaisidou, Stavros Stathoudakis

#ifndef PIPE_H
#define PIPE_H

#include <stdio.h>
#include <stdlib.h>

// Enum for the write state of the pipe
typedef enum writeEnum {CLOSED, OPEN} writeE;

// Struct representing a pipe
typedef struct pipe {
    int id;
    int in;
    int out;
    writeE write;
    int size;
    char* data;
} pipeS;

// Functions
int pipe_open(int size);
int pipe_write(int p, char c);
int pipe_writeDone(int p);
int pipe_read(int p, char* c);

#endif // PIPE_H