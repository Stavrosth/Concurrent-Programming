// Created by: Dimitris Voitsidis, Iordana Gaisidou, Stavros Stathoudakis
#include "PrimalityTesterWithMonitors.h"

// Error checking for pthread_mutex function calls
void my_pthread_error(int errorVal) {
    if(errorVal != 0){
        fprintf(stderr, "Error %d in pthread_mutex_lock!!!\n", errorVal);
        exit(-1);
    }
}

// Initialize monitor
void init_monitor(monitor_var_t *monitor) {
    my_pthread_error(pthread_mutex_init(&monitor->mtx, NULL));
    my_pthread_error(pthread_cond_init(&monitor->mainQ, NULL));
    my_pthread_error(pthread_cond_init(&monitor->workerQ, NULL));
}

// This is derrived from the optimized approach of
// https://www.geeksforgeeks.org/c-program-to-check-prime-number-by-creating-a-function/
// Algorithm to determine if a number is prime or not
// Time complexity of O(√N)
void* worker(void* Wargs) {
    int num;
    bool prime;
    pthread_t self = pthread_self();

    workerArgS* args = (workerArgS*) Wargs;

    printf("Hello from Worker %ld\n", self);

    // Worker loop
    while(1) {
        // Get job
        my_pthread_error(pthread_mutex_lock(&args->monitor->mtx));
        
        while (*(args->job_taken) == true && *(args->terminate) == false) {
            my_pthread_error(pthread_cond_wait(&args->monitor->workerQ, &args->monitor->mtx));
        }

        // Check if worker should terminate
        if(*(args->terminate) == true){
            // Reset worker state so other workers can terminate
            my_pthread_error(pthread_cond_signal(&args->monitor->workerQ));
            my_pthread_error(pthread_mutex_unlock(&args->monitor->mtx));
            printf("Bye from Worker %ld\n", self);
            return NULL;
        }

        num = *(args->job);
        *(args->job_taken) = true;

        // Signal main to give next job
        my_pthread_error(pthread_cond_signal(&args->monitor->mainQ));

        my_pthread_error(pthread_mutex_unlock(&args->monitor->mtx));

        // If number is less than/equal to 1,
        // it is not prime
        if (num <= 1)
            printf("Worker %ld: %d is NOT prime and result is 0\n", self, num);
        else {
            // Assume that n is prime
            prime = true;
            
            // Check how many numbers divide n in
            // range 2 to sqrt(n)
            for (int i = 2; i * i <= num; i++) {
                if (num % i == 0) {
                    prime = false;
                    break;
                }
            }

            // if prime is true then n is prime
            if (prime)
                printf("Worker %ld: %d is prime and result is 0\n", self, num);
            // else n is NOT prime
            else
                printf("Worker %ld: %d is NOT prime and result is 1\n", self, num);
        }
    }
}

int main(int argc, char *argv[]) {
    int res, num;
    int workersNo;
    pthread_t *t;
    bool terminate = false;
    bool job_taken = true;
    monitor_var_t monitor;
    workerArgS worker_args;
    int i;

    if(argc < 2){
        printf("Wrong Arguments! Please give: ./PrimalityTester <number_of_workers> <int1> <int2> ... <intN>\n");
        return 1;
    }

    printf("Hello from Main\n");

    // Initialize monitor
    init_monitor(&monitor);

    // Initialize worker arguments
    worker_args.job = &num;
    worker_args.job_taken = &job_taken;
    worker_args.terminate = &terminate;
    worker_args.monitor = &monitor;

    // Get number of workers from command line
    workersNo = atoi(argv[1]);
    
    // Allocate memory for worker threads
    t = (pthread_t*)malloc(workersNo * sizeof(pthread_t));
    if (t == NULL) {
        perror("ERROR: malloc t");
        return (-1);
    }
    
    // Create worker threads and initialize their arguments
    for( i = 0; i < workersNo; i++ ) {
        res = pthread_create(&t[i], NULL, worker, &worker_args);
        if (res) {
            fprintf(stderr, "ERROR: pthread_create %ld error value is:%d\n", t[i] ,res);
        }
    }

    // Assign jobs to workers
    for (i = 2; i < argc; i++ ) {
        // Wait for worker to take job
        my_pthread_error(pthread_mutex_lock(&monitor.mtx));

        if (job_taken == false) {
            my_pthread_error(pthread_cond_wait(&monitor.mainQ, &monitor.mtx));
        }
        
        num = atoi(argv[i]);
        job_taken = false;

        // Signal worker to take job
        my_pthread_error(pthread_cond_signal(&monitor.workerQ));
        
        my_pthread_error(pthread_mutex_unlock(&monitor.mtx));
    }

    // Wait for all jobs to be taken
    my_pthread_error(pthread_mutex_lock(&monitor.mtx));

    if ( job_taken == false ) {
        my_pthread_error(pthread_cond_wait(&monitor.mainQ, &monitor.mtx));
    }

    // Signal workers to terminate
    terminate = true;

    // Reset worker state so they can terminate
    my_pthread_error(pthread_cond_signal(&monitor.workerQ));
        
    my_pthread_error(pthread_mutex_unlock(&monitor.mtx));

    // Wait for workers to terminate
    for( i = 0; i < workersNo; i++ ) {
        res = pthread_join(t[i], NULL);
        if (res) {
            fprintf(stderr, "ERROR: pthread_join %ld error value is:%d\n", t[i] ,res);
        }
    }

    // Frees memory
    free(t);

    // Destroy monitor
    pthread_mutex_destroy(&monitor.mtx);
    pthread_cond_destroy(&monitor.mainQ);
    pthread_cond_destroy(&monitor.workerQ);
    
    printf("Bye from Main\n");

    return 0;
}

