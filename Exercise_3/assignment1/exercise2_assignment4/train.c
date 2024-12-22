#include <unistd.h>
#include <stdbool.h>

#include"../binarySemaphores.h"

// Struct used to pass arguments in the passenger thread
typedef struct Targs{
    int train_capacity; // Train capacity
    bool* destroy; // Flag to terminate train
    int* boarded_passengers; // Number of passengers boarded
    int* waiting_passengers; // Number of passengers waiting
    mysem_t* mtx; // Semaphore to signal waiting passengers
    mysem_t* semBoarding; // Semaphore to signal boarding passengers
    mysem_t* semTrain; // Semaphore to signal train
    mysem_t* semDisembark; // Semaphore to signal disembarking passengers
} TargsS;

void* train(void* Targs) {
    TargsS* args = (TargsS*) Targs;

    printf("\033[0;33mHello from Train\033[0m\n");

    // while the user does not want to exit
    while (*(args->destroy) == false) {
        // wait for train to be full or the user to exit
        mysem_error(mysem_down(args->semTrain), "mysem_down semTrain");

        if(*(args->destroy) == true && *(args->boarded_passengers) == 0) {
            break;
        }
        
        // Artificial delay of 1 second to simulate train journey
        printf("\033[1;33mTrain starts journey!\033[0m\n");
        sleep(2);
        printf("\033[1;33mTrain completed the journey!\033[0m\n");
        
        // Release passengers for disembarkation
        mysem_error(mysem_up(args->semDisembark), "mysem_up semDisembark");
    }

    // Signals passengers to not board and terminate if there are any waiting
    mysem_error(mysem_down(args->mtx), "mysem_down mtx");
    if ( *(args->waiting_passengers) != 0) {
        mysem_error(mysem_up(args->semBoarding), "mysem_up semBoarding1");
    }
    mysem_error(mysem_up(args->mtx), "mysem_up mtx");

    while(*(args->boarded_passengers) != 0) {
        mysem_error(mysem_down(args->semTrain), "mysem_up semTrain");
    }
    
    printf("\033[0;33mBye from Train\033[0m\n");

    return NULL;
}

void* passenger(void* Pargs) {
    TargsS* args = (TargsS*) Pargs;

    printf("\033[0;36mHello from Passenger %ld\033[0m\n", pthread_self());

    sleep(1);

    // Signals the other passengers that a passenger is waiting
    mysem_error(mysem_down(args->mtx), "mysem_down mtx");
    (*(args->waiting_passengers))++;
    mysem_error(mysem_up(args->mtx), "mysem_up mtx");

    // passenger waits to board
    mysem_error(mysem_down(args->semBoarding), "mysem_down semBoarding");

    // Signals the other passengers that a passenger is no loger waiting
    mysem_error(mysem_down(args->mtx), "mysem_down mtx");
    (*(args->waiting_passengers))--;
    mysem_error(mysem_up(args->mtx), "mysem_up mtx");

    // Check if the user wants to exit
    if (*(args->destroy) == true) {
        // Signals other passengers to exit boarding
        mysem_error(mysem_up(args->semBoarding), "mysem_up semBoarding2");
        
        printf("\033[1;36mPassenger %ld did not even get on the train\033[0m\n", pthread_self());
        return NULL;
    }

    printf("\033[1;36mPassenger %ld boarded\033[0m\n", pthread_self());

    // Signals the other passengers that a passenger has boarded
    (*(args->boarded_passengers))++;

    // Checks if the train is full
    if (*(args->boarded_passengers) == args->train_capacity) {
        mysem_error(mysem_up(args->semTrain), "mysem_up semTrain");
    } else {
        // Lets other passengers board
        mysem_error(mysem_up(args->semBoarding), "mysem_up semBoarding3");
    }

    // Waits to disembark after journey ends or the user wants to exit
    mysem_error(mysem_down(args->semDisembark), "mysem_down semDisembark");

    // Check if the user wants to exit
    printf("\033[1;36mPassenger %ld completed the journey and disembarked\033[0m\n", pthread_self());

    (*(args->boarded_passengers))--;
    if(*(args->destroy) == true && *(args->boarded_passengers) == 0) {
        // Signals train to terminate
        mysem_error(mysem_up(args->semTrain), "mysem_up semTrain");
    }

    // Checks if the train is empty and signals the waiting passengers to board
    if (*(args->boarded_passengers) == 0 && *(args->destroy) == false) {
        // Signals other passengers to board
        mysem_error(mysem_up(args->semBoarding), "mysem_up semBoarding4");
    } else {
        // Signals the rest passengers to disembark
        mysem_error(mysem_up(args->semDisembark), "mysem_up semDisembark");
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    int N; // train capacity
    pthread_t trainT; // train thread
    pthread_t* passengers = NULL; // passenger threads
    int res;
    int boarded_passengers = 0;
    int waiting_passengers = 0;
    int i = 0;
    bool destroy = false;
    TargsS Targs;
    mysem_t semTrain, semBoarding, semDisembark, mtx;
    
    // check for correct number of arguments
    if (argc != 2) {
        printf("Usage: %s <train capacity>\n", argv[0]);
        return -1;
    }

    N = atoi(argv[1]);

    // initialize semaphores
    mysem_error(mysem_init(&semTrain, 0), "mysem_init semTrain");
    mysem_error(mysem_init(&semBoarding, 1), "mysem_init semBoarding");
    mysem_error(mysem_init(&semDisembark, 0), "mysem_init semDisembark");
    mysem_error(mysem_init(&mtx, 1), "mysem_init mtx");

    // intialize arguments for threads
    Targs.train_capacity = N;
    Targs.destroy = &destroy;
    Targs.boarded_passengers = &boarded_passengers;
    Targs.waiting_passengers = &waiting_passengers;
    Targs.semTrain = &semTrain;
    Targs.semBoarding = &semBoarding;
    Targs.semDisembark = &semDisembark;
    Targs.mtx = &mtx;

    // create train thread
    res = pthread_create(&trainT, NULL, train, &Targs);
    if (res) {
        fprintf(stderr, "ERROR: pthread_create train error value is: %d\n", res);
        perror("");
        return -1;
    }
    
    // create passenger threads when the user presses enter
    while(1) {
        printf("Press ENTER to create passenger %d\n", i);
        printf("Press any other key to exit\n");

        if (getchar() != '\n') {

            printf("Exiting...\n");

            destroy = true;

            // Signals train to terminate
            mysem_error(mysem_up(&semTrain), "mysem_up semTrain");
          
            break;
        }

        printf("Creating passenger %d\n", i);

        // allocate memory for passenger struct
        passengers = realloc(passengers, (i + 1) * sizeof(pthread_t));
        if(passengers == NULL) {
            perror("ERROR: realloc passengers error\n");
            return -1;
        }
        
        // create passenger thread
        res = pthread_create(&passengers[i], NULL, passenger, &Targs);
        if (res) {
            fprintf(stderr, "ERROR: pthread_create train error value is: %d\n", res);
            perror("");
            return -1;
        }

        i++;
    }

    // wait for train and passenger threads to terminate
    res = pthread_join(trainT, NULL);
    if ( res != 0) {
        fprintf(stderr, "ERROR: pthread_join train error value is: %d\n", res);
        perror("");
    }
    for (int j = 0; j < i; j++) {
        res = pthread_join(passengers[j], NULL);
        if ( res != 0) {
            fprintf(stderr, "ERROR: pthread_join passenger %d error value is: %d\n", j, res);
            perror("");
        }
    }

    // destroy semaphores
    mysem_error(mysem_destroy(&semTrain), "mysem_destroy semTrain");
    mysem_error(mysem_destroy(&semBoarding), "mysem_destroy semBoarding");
    mysem_error(mysem_destroy(&semDisembark), "mysem_destroy semDisembark");
    mysem_error(mysem_destroy(&mtx), "mysem_destroy mtx");

    free(passengers);

    printf("Bye from Main.\n");

    return 0;
}