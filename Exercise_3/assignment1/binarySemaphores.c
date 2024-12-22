// Created by: Dimitris Voitsidis, Iordana Gaisidou, Stavros Stathoudakis
#include "binarySemaphores.h"


// Array of semaphores
static mysem_t** sem_arr = NULL;
static int sem_arr_size = 0;

// Initializes a semaphore
int mysem_init(mysem_t *s, int n) {
    static int id_assign = 1; 
    int i;

    // Check if n is 0 or 1
    if(n != 0 && n != 1) {
        return 0;
    }

    // Critical Section Start: Read
    rw_monitor_start_read();

    // Check if the semaphore is already initialized
    for (i=0; i < sem_arr_size; i++) {
        // If the semaphore is already initialized return -1
        if (s->sem_id == sem_arr[i]->sem_id) {
            rw_monitor_stop_read();
            // Critical Section End: Read
            return -1;
        }
    }

    rw_monitor_stop_read();
    // Critical Section End: Read

    s->sem_id = id_assign;
    id_assign++;
    s->sem_val = n;

    // Initialize the monitor
    sem_monitor_semaphore_init(&s->sem_monitor);

    // Critical Section Start: Write
    rw_monitor_start_write();

    // Allocate memory in the array for the semaphore
    sem_arr = realloc(sem_arr, sizeof(mysem_t*) * (sem_arr_size + 1));
    if(sem_arr == NULL) {
        rw_monitor_stop_write();
        fprintf(stderr, "Error in realloc sem_arr!!!\n");
        exit(-1);
    }

    // Add the semaphore to the array
    sem_arr[sem_arr_size] = s;
    sem_arr_size++;

    rw_monitor_stop_write();
    // Critical Section End: Write

    return 1;
}

// Decreases the semaphore value
int mysem_down(mysem_t *s) {
    int i;
    bool exist = false;

    rw_monitor_start_read();

    // Check if the semaphore is initialized
    for (i=0; i < sem_arr_size; i++) {
        if (s->sem_id == sem_arr[i]->sem_id) {
            exist = true;
            break;
        }
    }

    // If the semaphore is not initialized return -1
    if(exist == false) {
        rw_monitor_stop_read();
        // Critical Section End: Read

        return -1;
    }

    rw_monitor_stop_read();
    // Critical Section End: Read

    // Decrease the semaphore value
    sem_monitor_semaphore_down(&s->sem_monitor, &s->sem_val);

    return 1;
}

// Increases the semaphore value
int mysem_up(mysem_t *s) {
    int i;
    int res;
    bool exist = false;

    // Critical Section Start: Read
    rw_monitor_start_read();

    // Check if the semaphore is initialized
    for (i=0; i < sem_arr_size; i++) {
        if (s->sem_id == sem_arr[i]->sem_id) {
            exist = true;
            break;
        }
    }

    // If the semaphore is not initialized return -1
    if(exist == false) {
        rw_monitor_stop_read();
        // Critical Section End: Read

        return -1;
    }

    rw_monitor_stop_read();
    // Critical Section End: Read

    // If the semaphore is initialized

    // Check if the semaphore value is already 1
    res = sem_monitor_semaphore_up(&s->sem_monitor, &s->sem_val);
    if(res == 1) { return 0; }

    return 1;
}

// Destroys the semaphore
int mysem_destroy(mysem_t *s) {
    int i;
    bool exist = false;

    // Critical Section Start: Read
    rw_monitor_start_read();

    // Check if the semaphore is initialized
    for (i=0; i < sem_arr_size; i++) {
        if (s->sem_id == sem_arr[i]->sem_id) {
            exist = true;
            break;
        }
    }

    // If the semaphore is not initialized return -1
    if(exist == false) {
        rw_monitor_stop_read();
        // Critical Section End: Read

        return -1;
    }

    rw_monitor_stop_read();
    // Critical Section End: Read

    // Critical Section Start: Write
    rw_monitor_start_write();

    // Remove the semaphore from the array
    sem_arr_size--;
    sem_arr[i] = sem_arr[sem_arr_size];

    sem_arr = realloc(sem_arr, sizeof(mysem_t*) * sem_arr_size);
    if( sem_arr == NULL && sem_arr_size != 0) {
        rw_monitor_stop_write();
        perror("Error in realloc sem_arr!!!");
        exit(-1);
    }

    rw_monitor_stop_write();

    return 1;
}

// Prints the error value if there is an error
int mysem_error(int returnVal, char *text) {
    if (returnVal != 1) {
        fprintf(stderr, "%s: ERROR: error value is %d\n", text, returnVal);
    }
    return returnVal;
}