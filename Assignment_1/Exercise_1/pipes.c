// Created by: Dimitris Voitsidis, Iordana Gaisidou, Stavros Stathoudakis

#include "pipes.h"

// Array of pipes
static pipeS** pipe_arr = NULL;
static int pipe_arr_size = 0;
static int pipes_ctr = 0;

// Creates and opens a pipe for read and write
int pipe_open(int size) {
    pipeS* p;

    // Allocate memory for the pipe
    p = malloc(sizeof(pipeS));
    if(p == NULL) {printf("Error in malloc p!!!\n");}

    p->data = malloc(size * sizeof(char));
    if(p->data == NULL) {printf("Error in malloc p->data!!!\n");}

    // Initialize the pipe
    p->id = pipes_ctr;
    pipes_ctr++;
    p->in = 0;
    p->out = 0;
    p->write = OPEN;
    p->size = size;

    // Add the pipe to the array
    pipe_arr = realloc(pipe_arr, sizeof(pipeS*) * (pipes_ctr));
    if(pipe_arr == NULL){printf("Error in realloc pipe_arr!!!\n");}
    
    pipe_arr[pipes_ctr - 1] = p;

    pipe_arr_size++;

    // Return the pipe pointer
    return p->id;
}

// Writes a character in a pipe
int pipe_write(int p, char c) {
    // Check if the pipe exists
    for (int i=0; i < pipes_ctr; i++) { 
        if (pipe_arr[i] != NULL && p == pipe_arr[i]->id){
            if (pipe_arr[i]->write == OPEN) {
                // If the pipe is full wait
                while (pipe_arr[i]->in == (pipe_arr[i]->out - 1) ||
                (pipe_arr[i]->out == 0 && pipe_arr[i]->in == (pipe_arr[i]->size - 1))) {/*wait*/}

                // Else write the character    
                pipe_arr[i]->data[pipe_arr[i]->in] = c;

                // Reset the in pointer if out of bounds (ring buffer)
                if ((pipe_arr[i]->size) - 1 == pipe_arr[i]->in) {
                    pipe_arr[i]->in = 0;
                } else {
                    (pipe_arr[i]->in)++;
                }
                
                // Return success
                return (1);
            } else {
                // Return failure
                return (-1);
            }
        }
    }

    // Return failure
    return (-1);
}

// Function to close the write end of a pipe
int pipe_writeDone(int p) {
    // Check if the pipe exists
    for (int i=0; i < pipes_ctr; i++) {
        if (pipe_arr[i] != NULL && p == pipe_arr[i]->id) {
            if(pipe_arr[i]->write == OPEN){
                pipe_arr[i]->write = CLOSED;
            } else {
                // Return failure
                return (-1);
            }

            // Return success
            return (1);
        }
    }

    // Return failure
    return (-1);
}

// Function to read a character from a pipe
int pipe_read(int p, char *c) {
    pipeS* pipe;

    // Check if the pipe exists
    for (int i=0; i < pipes_ctr; i++) {
        if (pipe_arr[i] != NULL && p == pipe_arr[i]->id) {
            // Wait for the pipe to be written
            while(pipe_arr[i]->in == pipe_arr[i]->out && pipe_arr[i]->write == OPEN) {/*wait*/}

            // If the pipe is empty and closed destroy it
            if (pipe_arr[i]->in == pipe_arr[i]->out && pipe_arr[i]->write == CLOSED) {
                pipe_arr_size--;
                pipe = pipe_arr[i];
                pipe_arr[i] = NULL;
                free(pipe->data);
                free(pipe);

                // If the array is empty free it
                if(pipe_arr_size == 0){
                    pipes_ctr = 0;
                    free(pipe_arr);
                    pipe_arr = NULL;
                }

                // Return destroyed
                return (0);
            }
            
            // Else read the character
            *c = pipe_arr[i]->data[pipe_arr[i]->out];

            // Reset the out pointer if out of bounds (ring buffer)
            if ((pipe_arr[i]->size) - 1 == pipe_arr[i]->out) {
                pipe_arr[i]->out = 0;
            } else {
                (pipe_arr[i]->out)++;
            }

            // Return success
            return (1);
        }
    }

    // Return failure
    return (-1);
}