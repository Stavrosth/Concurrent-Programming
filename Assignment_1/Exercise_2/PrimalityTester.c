#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

// Struct used to pass arguments in pthread_create
typedef struct workerArg {
    bool busy;
    int job;
    bool* terminate;
    bool terminated;
}workerArgS;

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
        // Wait for job
        while (!(args->busy)){
            // Terminate worker if terminate flag is set
            if(*(args->terminate)) {
                printf("Bye from Worker %ld\n", self);

                args->terminated = true; 

                return NULL;
            }
        }

        // Get job
        num = args->job;

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

        // Reset worker state
        args->busy = false;
    }
}

int main(int argc, char *argv[]) {
    int res, num;
    int j = -1;
    int workersNo;
    pthread_t *t;
    bool terminate = false;

    if(argc < 2){
        printf("Wrong Arguments! Please give: ./PrimalityTester <number_of_workers> <int1> <int2> ... <intN>\n");
        return 1;
    }

    printf("Hello from Main\n");

    workersNo = atoi(argv[1]);

    // Allocate memory for worker threads
    t = (pthread_t*)malloc(workersNo * sizeof(pthread_t));
    if (t == NULL) {
        perror("Error in malloc");
        fprintf(stderr, "ERROR: malloc t\n");
        return (-1);
    }

    // Allocate memory for worker arguments
    workerArgS* args = (workerArgS*)malloc(workersNo * sizeof(workerArgS));
    if (args == NULL) {
        perror("Error in malloc");
        fprintf(stderr, "ERROR: malloc args\n");
        return (-1);
    }

    // Create worker threads and initialize their arguments
    for( int k = 0; k < workersNo; k++ ) {
        args[k].busy = false;
        args[k].terminate = &terminate;
        args[k].terminated = false;

        res = pthread_create(&t[k], NULL, worker, &args[k]);
        if (res) {
            fprintf(stderr, "ERROR: pthread_create %ld error value is:%d\n", t[k] ,res);
        }
    }

    // Assign jobs to workers
    for ( int i = 2; i < argc; i++ ) {
        num = atoi(argv[i]);

        // Round robin assignment of jobs to workers
        while(1){
            j++;
            if(j == workersNo){
                j = 0;
            }

            // If worker is not busy, assign job to it
            if (!(args[j].busy)) {
                args[j].job = num;
                args[j].busy = true;

                break;
            }
        }
    }

    // Wait for workers to finish their jobs
    for (int i = 0; i < workersNo; i++) {
        while((args[i].busy)) {/*wait*/}
    }

    // Terminate workers
    terminate = true;
    
    // Wait for workers to terminate
    for (int i = 0; i < workersNo; i++) {
        while(!(args[i].terminated)) {/*wait*/}
    }

    free(t);
    free(args);

    printf("Bye from Main\n");

    return 0;
}
