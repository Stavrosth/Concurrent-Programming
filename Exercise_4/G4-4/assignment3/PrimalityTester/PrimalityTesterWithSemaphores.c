#include <math.h>
#include"../../assignment2/mythreads.h"

// Struct used to pass arguments in pthread_create
typedef struct workerArg {
    int* job; // Job to be done
    bool* terminate; // Flag to terminate worker
    mysem_t* semWorker; // Semaphore to signal worker
    mysem_t* semMain; // Semaphore to signal main
}workerArgS;

// This is derrived from the optimized approach of
// https://www.geeksforgeeks.org/c-program-to-check-prime-number-by-creating-a-function/
// Algorithm to determine if a number is prime or not
// Time complexity of O(√N)
void worker(void* Wargs) {
    int num;
    bool prime;
    long self = mythreads_self();

    workerArgS* args = (workerArgS*) Wargs;

    printf("Hello from Worker %ld\n", self);

    // Worker loop
    while(1) {
        // Get job
        mythreads_sem_down(args->semWorker);

        // Check if worker should terminate
        if(*(args->terminate) == true){
            // Reset worker state so other workers can terminate
            mythreads_sem_up(args->semWorker);
            printf("Bye from Worker %ld\n", self);
            //mythreads_yield();
            //mythreads_sem_down(args->semWorker);
            return;
        }

        num = *(args->job);

        // Signal main to give next job
        mythreads_sem_up(args->semMain);        

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
    mythr_t *t;
    bool terminate = false;
    mysem_t semMain, semWorker;
    workerArgS worker_args;

    if(argc < 2){
        printf("Wrong Arguments! Please give: ./PrimalityTester <number_of_workers> <int1> <int2> ... <intN>\n");
        return 1;
    }

    printf("Hello from Main\n");

    mythreads_init();

    // Initialize semaphores
    mythreads_sem_create(&semMain, 1);
    mythreads_sem_create(&semWorker, 0);

    // Initialize worker arguments
    worker_args.job = &num;
    worker_args.semWorker = &semWorker;
    worker_args.semMain = &semMain;
    worker_args.terminate = &terminate;

    // Get number of workers from command line
    workersNo = atoi(argv[1]);
    
    // Allocate memory for worker threads
    t = (mythr_t*)malloc(workersNo * sizeof(mythr_t));
    if (t == NULL) {
        perror("ERROR: malloc t");
        return (-1);
    }
    
    // Create worker threads and initialize their arguments
    for( int k = 0; k < workersNo; k++ ) {
        res = mythreads_create(&t[k], worker, &worker_args);
        if (res) {
            fprintf(stderr, "ERROR: pthread_create %ld error value is:%d\n", (long) &t[k] ,res);
        }
    }

    // Assign jobs to workers
    for ( int i = 2; i < argc; i++ ) {
        // Wait for worker to take job
        mythreads_sem_down(&semMain);
        
        num = atoi(argv[i]);
        
        // Signal worker to take job
        mythreads_sem_up(&semWorker);
    }

    // Wait for all jobs to be taken
    mythreads_sem_down(&semMain);

    // Signal workers to terminate
    terminate = true;

    // Reset worker state so they can terminate
    mythreads_sem_up(&semWorker);

    // Wait for workers to terminate
    for( int k = 0; k < workersNo; k++ ) {
        res = mythreads_join(&t[k]);
        if (res) {
            fprintf(stderr, "ERROR: pthread_join %ld error value is:%d\n", (long) &t[k] ,res);
        }

        res = mythreads_destroy(&t[k]);
        if (res) {
            fprintf(stderr, "ERROR: pthread_destroy %ld error value is:%d\n", (long) &t[k] ,res);
        }
    }

    // Destroy semaphores
    mythreads_sem_destroy(&semMain);
    mythreads_sem_destroy(&semWorker);

    // Frees memory
    free(t);

    printf("Bye from Main\n");

    return 0;
}

