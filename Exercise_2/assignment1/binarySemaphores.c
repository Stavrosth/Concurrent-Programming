// Created by: Dimitris Voitsidis, Iordana Gaisidou, Stavros Stathoudakis
#include "binarySemaphores.h"

// Array of semaphores
static mysem_t** sem_arr = NULL;
static int sem_arr_size = 0;

static pthread_mutex_t rd_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t wr_mutex = PTHREAD_MUTEX_INITIALIZER;

static int rds=0;

// Error checking for pthread_mutex function calls
void my_pthread_mutex_error(int errorVal) {
    if(errorVal != 0){
        fprintf(stderr, "Error %d in pthread_mutex_lock!!!\n", errorVal);
        exit(-1);
    }
}

// Initializes a semaphore
int mysem_init(mysem_t *s, int n) {
    int i;

    // Check if n is 0 or 1
    if(n != 0 && n != 1) {
        return 0;
    }

    // Critical Section Start: Read
    my_pthread_mutex_error( pthread_mutex_lock(&rd_mutex));
    if(rds == 0) {
        my_pthread_mutex_error( pthread_mutex_lock(&wr_mutex));
    }
    rds++;
    my_pthread_mutex_error( pthread_mutex_unlock(&rd_mutex));
    
    // Check if the semaphore is already initialized
    for (i=0; i < sem_arr_size; i++) {
        // If the semaphore is already initialized return -1
        if (s == sem_arr[i]) {
            my_pthread_mutex_error( pthread_mutex_lock(&rd_mutex));
            rds--;
            if(rds == 0) {
                my_pthread_mutex_error( pthread_mutex_unlock(&wr_mutex));
            }
            my_pthread_mutex_error( pthread_mutex_unlock(&rd_mutex));
            // Critical Section End: Read

            return -1;
        }
    }

    my_pthread_mutex_error( pthread_mutex_lock(&rd_mutex));
    rds--;
    if(rds == 0) {
        my_pthread_mutex_error( pthread_mutex_unlock(&wr_mutex));
    }
    my_pthread_mutex_error( pthread_mutex_unlock(&rd_mutex));
    // Critical Section End: Read

    // If the semaphore is not initialized

    // Create the semaphore
    s->semid = semget(IPC_PRIVATE, 1, S_IRWXU | IPC_CREAT);
    if (s->semid == -1) {
        perror("Error in semget in mysem_init!!!");
        exit(-1);
    }

    // Set the semaphore value
    if (semctl(s->semid, 0, SETVAL, n) == -1) {
        perror("Error in semctl SETVAL in mysem_init!!!");
        exit(-1);
    }

    // Critical Section Start: Write
    my_pthread_mutex_error( pthread_mutex_lock(&wr_mutex));

    // Allocate memory in the array for the semaphore
    sem_arr = realloc(sem_arr, sizeof(mysem_t*) * (sem_arr_size + 1));
    if(sem_arr == NULL) {
        my_pthread_mutex_error( pthread_mutex_unlock(&wr_mutex));
        fprintf(stderr, "Error in realloc sem_arr!!!\n");
        exit(-1);
    }

    // Add the semaphore to the array
    sem_arr[sem_arr_size] = s;
    sem_arr_size++;

    my_pthread_mutex_error( pthread_mutex_unlock(&wr_mutex));
    // Critical Section End: Write

    return 1;
}

// Decreases the semaphore value
int mysem_down(mysem_t *s) {
    int i;
    int cntr = 0;
    struct sembuf sem_op = {0, -1, 0};

    // Critical Section Start: Read
    my_pthread_mutex_error( pthread_mutex_lock(&rd_mutex));
    if(rds == 0) {
        my_pthread_mutex_error( pthread_mutex_lock(&wr_mutex));
    }
    rds++;
    my_pthread_mutex_error( pthread_mutex_unlock(&rd_mutex));

    // Check if the semaphore is initialized
    for (i=0; i < sem_arr_size; i++) {
        if (s == sem_arr[i]) {
            cntr = 1;
            break;
        }
    }

    // If the semaphore is not initialized return -1
    if(cntr == 0) {
        my_pthread_mutex_error( pthread_mutex_lock(&rd_mutex));
        rds--;
        if(rds == 0) {
            my_pthread_mutex_error( pthread_mutex_unlock(&wr_mutex));
        }
        my_pthread_mutex_error( pthread_mutex_unlock(&rd_mutex));
        // Critical Section End: Read

        return -1;
    }

    my_pthread_mutex_error( pthread_mutex_lock(&rd_mutex));
    rds--;
    if(rds == 0) {
        my_pthread_mutex_error( pthread_mutex_unlock(&wr_mutex));
    }
    my_pthread_mutex_error( pthread_mutex_unlock(&rd_mutex));
    // Critical Section End: Read

    // Decrease the semaphore value
    if (semop(s->semid, &sem_op, 1) == -1) {
        perror("Error in semop in mysem_down!!!");
        exit(-1);
    }

    return 1;
}

// Increases the semaphore value
int mysem_up(mysem_t *s) {
    int i;
    int cntr = 0;
    struct sembuf sem_op = {0, 1, 0};
    int returnValue;
    
    // Critical Section Start: Read
    my_pthread_mutex_error( pthread_mutex_lock(&rd_mutex));
    if(rds == 0) {
        my_pthread_mutex_error( pthread_mutex_lock(&wr_mutex));
    }
    rds++;
    my_pthread_mutex_error( pthread_mutex_unlock(&rd_mutex));

    // Check if the semaphore is initialized
    for (i=0; i < sem_arr_size; i++) {
        if (s == sem_arr[i]) {
            cntr = 1;
            break;
        }
    }

    // If the semaphore is not initialized return -1
    if(cntr == 0) {
        my_pthread_mutex_error( pthread_mutex_lock(&rd_mutex));
        rds--;
        if(rds == 0) {
            my_pthread_mutex_error( pthread_mutex_unlock(&wr_mutex));
        }
        my_pthread_mutex_error( pthread_mutex_unlock(&rd_mutex));
        // Critical Section End: Read

        return -1;
    }

    my_pthread_mutex_error( pthread_mutex_lock(&rd_mutex));
    rds--;
    if(rds == 0) {
        my_pthread_mutex_error( pthread_mutex_unlock(&wr_mutex));
    }
    my_pthread_mutex_error( pthread_mutex_unlock(&rd_mutex));
    // Critical Section End: Read
    
    // If the semaphore is initialized

    // Check if the semaphore value is already 1
    returnValue = semctl(s->semid, 0, GETVAL);
    if(returnValue == 1) {
       return 0;
    } else if(returnValue == -1) {
        perror("Error in semctl GETVAL in mysem_up!!!");
        exit(-1);
    }

    // Increase the semaphore value
    if (semop(s->semid, &sem_op, 1) == -1) {
        perror("Error in semop in mysem_up!!!");
        exit(-1);
    }

    return 1;
}

// Destroys the semaphore
int mysem_destroy(mysem_t *s) {
    int i;
    int cntr = 0;
    
    // Critical Section Start: Read
    my_pthread_mutex_error( pthread_mutex_lock(&rd_mutex));
    if(rds == 0) {
        my_pthread_mutex_error( pthread_mutex_lock(&wr_mutex));
    }
    rds++;
    my_pthread_mutex_error( pthread_mutex_unlock(&rd_mutex));

    // Check if the semaphore is initialized
    for (i=0; i < sem_arr_size; i++) {
        if (s == sem_arr[i]) {
            cntr = 1;
            break;
        }
    }

    // If the semaphore is not initialized return -1
    if(cntr == 0) {
        my_pthread_mutex_error( pthread_mutex_lock(&rd_mutex));
        rds--;
        if(rds == 0) {
            my_pthread_mutex_error( pthread_mutex_unlock(&wr_mutex));
        }
        my_pthread_mutex_error( pthread_mutex_unlock(&rd_mutex));
        // Critical Section End: Read

        return -1;
    }

    my_pthread_mutex_error( pthread_mutex_lock(&rd_mutex));
    rds--;
    if(rds == 0) {
        my_pthread_mutex_error( pthread_mutex_unlock(&wr_mutex));
    }
    my_pthread_mutex_error( pthread_mutex_unlock(&rd_mutex));
    // Critical Section End: Read

    // if the semaphore is initialized destroy it 

    // Critical Section Start: Write
    my_pthread_mutex_error( pthread_mutex_lock(&wr_mutex));

    // Remove the semaphore from the array
    sem_arr_size--;
    sem_arr[i] = sem_arr[sem_arr_size];
    
    sem_arr = realloc(sem_arr, sizeof(mysem_t*) * sem_arr_size);
    if( sem_arr == NULL && sem_arr_size != 0) {
        my_pthread_mutex_error( pthread_mutex_unlock(&wr_mutex));
        perror("Error in realloc sem_arr!!!");
        exit(-1);
    }

    my_pthread_mutex_error( pthread_mutex_unlock(&wr_mutex));
    // Critical Section End: Write

    // Destroy the semaphore
    if (semctl(s->semid, 0, IPC_RMID) == -1) {
        perror("Error in semctl IPC_RMID in mysem_init!!!");
        exit(-1);
    }

    return 1;
}

// Prints the error value if there is an error
int mysem_error(int returnVal, char *text) {
    if (returnVal != 1) {
        fprintf(stderr, "%s: ERROR: error value is %d\n", text, returnVal);
    }
    return returnVal;
}