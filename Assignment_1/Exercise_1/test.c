// Created by: Dimitris Voitsidis, Iordana Gaisidou, Stavros Stathoudakis

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include "pipes.h"

// Struct used to pass arguments in pthread_create
typedef struct threadArg {
    int pipein;
    int pipeout;
    char* filename;
    bool  returned;
}threadArgS;

// Thread one routine: reads <filename> -> writes in pipe1 -> reads pipe2 -> writes in <filename>.copy2
void* thread1(void* Targs) {
    int fd;
    int fd_copy2;
    int p_read_check;
    char buffer;
    size_t fileNameSz;
    char *filenameCopy2;

    threadArgS* args = (threadArgS*) Targs;

    printf("Hello from Thread1!\n");

    /************************ Read <filename> -> write in pipe1 ************************/
    // Open <filename>
    fd = open(args->filename , O_RDONLY);
    if(fd == -1){
        fprintf(stderr, "Error in <%s> open.\n",args->filename);
        exit(-1);
    }

    // Write in pipe1
    while (read(fd, &buffer, sizeof(char)) != 0) {
        if (pipe_write(args->pipein, buffer) == -1 ){
            fprintf(stderr, "Error in pipe_write of pipe1 in thread1.\n");
            exit(-1);
        }
    }
    
    // Finish writing in pipe1
    if ( pipe_writeDone(args->pipein) == -1) {
        fprintf(stderr, "Error in pipe_writeDone of pipe1 in thread1.\n");
        exit(-1);
    }

    /************************ Read pipe2 -> write in <filename>.copy2 ************************/

    // Create and open <filename>.copy2 with permissions to RWX for everyone
    fileNameSz = strlen(args->filename) + strlen(".copy2") + 1;
    filenameCopy2 = malloc(fileNameSz);

    strcpy(filenameCopy2,args->filename);
    strcat(filenameCopy2,".copy2");

    fd_copy2 = open(filenameCopy2, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if(fd_copy2 == -1){
        fprintf(stderr, "Error in %s open.\n", filenameCopy2);
        exit(-1);
    }

    // Read from pipe2 
    while( (p_read_check = pipe_read(args->pipeout, &buffer)) == 1) {
        if ( write(fd_copy2, &buffer, sizeof(char)) == -1) {
            fprintf(stderr, "Error in %s write.\n", filenameCopy2);
            exit(-1);
        }
    }

    if(p_read_check == -1){
        fprintf(stderr, "Error in pipe1 read in thread1.\n");
        exit(-1);
    }
    
    printf("Pipe2 destroyed.\n");

    free(filenameCopy2);

    args->returned = true;
    
    printf("Bye from Thread1!\n");

    return(NULL);
}

// Thread two routine: reads pipe1 -> writes in <filename>.copy -> reads <filename>.copy -> writes in pipe2
void* thread2(void* Targs) {
    int fd_copy;
    int p_read_check;
    char buffer;
    size_t fileNameSz;
    char *filenameCopy;

    threadArgS* args = (threadArgS*) Targs;

    printf("Hello from Thread2!\n");

    // Create and open <filename>.copy with permissions to RWX for everyone
    fileNameSz = strlen(args->filename) + strlen(".copy") + 1;
    filenameCopy = malloc(fileNameSz);

    strcpy(filenameCopy,args->filename);
    strcat(filenameCopy,".copy");

    fd_copy = open(filenameCopy, O_RDWR | O_CREAT | O_TRUNC, 0777);
    if(fd_copy == -1){
        fprintf(stderr, "Error in %s open.\n", filenameCopy);
        exit(-1);
    }

    /************************ Read pipe1 -> write it in <filename>.copy ************************/ 
    while( (p_read_check = pipe_read(args->pipeout, &buffer)) == 1) {
        if ( write(fd_copy, &buffer, sizeof(char)) == -1) {
            fprintf(stderr, "Error in %s write.\n", filenameCopy);
            exit(-1);
        }
    }

    if(p_read_check == -1){
        fprintf(stderr, "Error in pipe1 read in thread2.\n");
        exit(-1);
    }

    printf("Pipe1 destroyed.\n");

    /************************ Read <filename>.copy -> write it in pipe2 ************************/

    // Reset the file descriptor to the beginning of the file
    if (lseek(fd_copy, 0, SEEK_SET) == -1) {
        perror("Error seeking to the beginning of the file");
        exit(-1);
    }

    while (read(fd_copy, &buffer, sizeof(char)) != 0) {
        if (pipe_write(args->pipein, buffer) == -1 ){
            fprintf(stderr, "Error in pipe write of pipe2.\n");
            exit(-1);
        }
    }
    
    // Finish writing in pipe2
    if ( pipe_writeDone(args->pipein) == -1) {
        fprintf(stderr, "Error in pipe write done of pipe2.\n");
        exit(-1);
    }

    free(filenameCopy);

    args->returned = true;

    printf("Bye from Thread2!\n");

    return(NULL);
}

int main (int argc, char* argv[]) {
    int pipe1, pipe2;
    pthread_t t1, t2;

    threadArgS args1, args2;
    char* filename;
    int res;     

    // Checking the program's arguments -> ./test <filename_to_be_copied>
    if ( argc != 2 ) {
        printf("Wrong Arguments! Please give: ./test <filename> \n");
        return 1;
    }
    
    printf("Hello from Main!\n");

    // Extracts the filename out of the input arguments
    filename = argv[1];

    // Creates two pipes size of 64 bytes
    pipe1 = pipe_open(64);
    pipe2 = pipe_open(64);

    // Thread ONE arguments : <pipein> = pipe1, <pipeout> = pipe2, <filename>
    args1.filename = filename;
    args1.pipein = pipe1;
    args1.pipeout = pipe2;
    args1.returned = false;

    // Thread TWO arguments : <pipein> = pipe2, <pipeout> = pipe1, <filename>
    args2.filename = filename;
    args2.pipein = pipe2;
    args2.pipeout = pipe1;
    args2.returned = false;

    // THREAD ONE: Writes the filename's text data to pipe1 and to filename.copy2
    res = pthread_create(&t1, NULL, thread1, &args1);
    if (res) {
        fprintf(stderr, "ERROR: pthread_create %ld error value is:%d\n", t1 ,res);
    }

    // THREAD TWO: Reads pipe1 and copies the text data to filename.copy and writes it to pipe2
    res = pthread_create(&t2, NULL, thread2, &args2);
    if (res) {
        fprintf(stderr, "ERROR: pthread_create %ld error value is:%d\n", t2 ,res);
    }

    while(args1.returned == false || args2.returned == false){ /*wait*/};

    printf("Bye from Main!\n");

    return 0;
}