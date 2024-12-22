// Created by: Dimitris Voitsidis, Iordana Gaisidou, Stavros Stathoudakis

#include "MonitorsForBinarySemaphores.h"

static rw_monitor_var_t rw_monitor = {.mtx   = PTHREAD_MUTEX_INITIALIZER,
                                    .readQ   = PTHREAD_COND_INITIALIZER,
                                    .writeQ  = PTHREAD_COND_INITIALIZER,
                                    .readers = 0, .readers_waiting = 0,
                                    .writers = 0, .writers_waiting = 0};


// Error checking for pthread function calls
void my_pthread_error(int errorVal) {
    if(errorVal != 0){
        fprintf(stderr, "Error %d in pthread_mutex_lock!!!\n", errorVal);
        exit(-1);
    }
}


// Read-Write Monitor Functions

// Start reading
void rw_monitor_start_read() {
    my_pthread_error( pthread_mutex_lock(&rw_monitor.mtx));

    // If there are writers, wait
    if (rw_monitor.writers > 0) {
        rw_monitor.readers_waiting++;
        my_pthread_error( pthread_cond_wait(&rw_monitor.readQ, &rw_monitor.mtx));

        // if there are readers waiting, signal them
        if (rw_monitor.readers_waiting > 0) {
            rw_monitor.readers_waiting--;
            my_pthread_error( pthread_cond_signal(&rw_monitor.readQ));
        }
    }
    rw_monitor.readers++;

    my_pthread_error( pthread_mutex_unlock(&rw_monitor.mtx));
}

// Stop reading
void rw_monitor_stop_read() {
    my_pthread_error( pthread_mutex_lock(&rw_monitor.mtx));

    rw_monitor.readers--;

    // If there are no readers and there are writers waiting, signal them
    if (rw_monitor.readers == 0 && rw_monitor.writers_waiting > 0) {
        rw_monitor.writers_waiting--;
        my_pthread_error( pthread_cond_signal(&rw_monitor.writeQ));
    }

    my_pthread_error( pthread_mutex_unlock(&rw_monitor.mtx));
}

// Start writing
void rw_monitor_start_write() {
    my_pthread_error( pthread_mutex_lock(&rw_monitor.mtx));
    
    // If there are readers or writers, wait
    if (rw_monitor.readers > 0 || rw_monitor.writers > 0) {
        rw_monitor.writers_waiting++;
        my_pthread_error( pthread_cond_wait(&rw_monitor.writeQ, &rw_monitor.mtx));
    }
    rw_monitor.writers++;

    my_pthread_error( pthread_mutex_unlock(&rw_monitor.mtx));
}

// Stop writing
void rw_monitor_stop_write() {
    my_pthread_error( pthread_mutex_lock(&rw_monitor.mtx));

    rw_monitor.writers--;

    // If there are readers waiting, signal them
    if (rw_monitor.readers_waiting > 0) {
        rw_monitor.readers_waiting--;
        my_pthread_error( pthread_cond_signal(&rw_monitor.readQ));
    // else if there are writers waiting, signal them
    } else if (rw_monitor.writers_waiting > 0) { 
        rw_monitor.writers_waiting--;
        my_pthread_error( pthread_cond_signal(&rw_monitor.writeQ));
    }

    my_pthread_error( pthread_mutex_unlock(&rw_monitor.mtx));
}


// Semaphore Monitor Functions

// Initialize the semaphore monitor
void sem_monitor_semaphore_init(sem_monitor_var_t *sem_monitor) {
    my_pthread_error( pthread_mutex_init(&sem_monitor->mtx, NULL));
    my_pthread_error( pthread_cond_init(&sem_monitor->semQ, NULL));
}

// Down the semaphore
void sem_monitor_semaphore_down(sem_monitor_var_t *sem_monitor, int *sem_val) {
    my_pthread_error( pthread_mutex_lock(&sem_monitor->mtx));

    // If the semaphore is 0, wait
    while (*sem_val == 0) {
        my_pthread_error( pthread_cond_wait(&sem_monitor->semQ, &sem_monitor->mtx));
    }
    *sem_val = 0;

    my_pthread_error( pthread_mutex_unlock(&sem_monitor->mtx));
}

// Up the semaphore
int sem_monitor_semaphore_up(sem_monitor_var_t *sem_monitor, int *sem_val) {
    my_pthread_error( pthread_mutex_lock(&sem_monitor->mtx));

    // If the semaphore is 1, return 1
    if (*sem_val == 1) {
        my_pthread_error( pthread_mutex_unlock(&sem_monitor->mtx));
        return 1;
    // if the semaphore is 0 signal the semaphore and return 0
    } else {
        *sem_val = 1;
        my_pthread_error( pthread_cond_signal(&sem_monitor->semQ));

        my_pthread_error( pthread_mutex_unlock(&sem_monitor->mtx));
        return 0;
    }
}