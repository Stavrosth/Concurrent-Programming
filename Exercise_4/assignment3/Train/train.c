#include"../../assignment2/mythreads.h"

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

void train(void* Targs) {
    TargsS* args = (TargsS*) Targs;

    printf("\033[0;33mHello from Train\033[0m\n");

    // while the user does not want to exit
    while (*(args->destroy) == false) {
        // wait for train to be full or the user to exit
        mythreads_sem_down(args->semTrain);

        if(*(args->destroy) == true && *(args->boarded_passengers) == 0) {
            break;
        }
        
        // Artificial delay of 1 second to simulate train journey
        printf("\033[1;33mTrain starts journey!\033[0m\n");
        mythreads_sleep(2);
        printf("\033[1;33mTrain completed the journey!\033[0m\n");
        
        // Release passengers for disembarkation
        mythreads_sem_up(args->semDisembark);
    }

    // Signals passengers to not board and terminate if there are any waiting
    mythreads_sem_down(args->mtx);
    if ( *(args->waiting_passengers) != 0) {
        mythreads_sem_up(args->semBoarding);
    }
    mythreads_sem_up(args->mtx);

    while(*(args->boarded_passengers) != 0) {
        mythreads_sem_down(args->semTrain);
    }
    
    printf("\033[0;33mBye from Train\033[0m\n");

    return;
}

void passenger(void* Pargs) {
    TargsS* args = (TargsS*) Pargs;

    printf("\033[0;36mHello from Passenger %ld\033[0m\n", mythreads_self());

    sleep(1);

    // Signals the other passengers that a passenger is waiting
    mythreads_sem_down(args->mtx);
    (*(args->waiting_passengers))++;
    mythreads_sem_up(args->mtx);

    // passenger waits to board
    mythreads_sem_down(args->semBoarding);

    // Signals the other passengers that a passenger is no loger waiting
    mythreads_sem_down(args->mtx);
    (*(args->waiting_passengers))--;
    mythreads_sem_up(args->mtx);

    // Check if the user wants to exit
    if (*(args->destroy) == true) {
        // Signals other passengers to exit boarding
        mythreads_sem_up(args->semBoarding);
        
        printf("\033[1;36mPassenger %ld did not even get on the train\033[0m\n", mythreads_self());
        return;
    }

    printf("\033[1;36mPassenger %ld boarded\033[0m\n", mythreads_self());

    // Signals the other passengers that a passenger has boarded
    (*(args->boarded_passengers))++;

    // Checks if the train is full
    if (*(args->boarded_passengers) == args->train_capacity) {
        mythreads_sem_up(args->semTrain);
    } else {
        // Lets other passengers board
       mythreads_sem_up(args->semBoarding);
    }

    // Waits to disembark after journey ends or the user wants to exit
    mythreads_sem_down(args->semDisembark);

    // Check if the user wants to exit
    printf("\033[1;36mPassenger %ld completed the journey and disembarked\033[0m\n", mythreads_self());

    (*(args->boarded_passengers))--;
    if(*(args->destroy) == true && *(args->boarded_passengers) == 0) {
        // Signals train to terminate
        mythreads_sem_up(args->semTrain);
    }

    // Checks if the train is empty and signals the waiting passengers to board
    if (*(args->boarded_passengers) == 0 && *(args->destroy) == false) {
        // Signals other passengers to board
        mythreads_sem_up(args->semBoarding);
    } else {
        // Signals the rest passengers to disembark
        mythreads_sem_up(args->semDisembark);
    }

    return;
}

int main(int argc, char* argv[]) {
    int N; // train capacity
    mythr_t trainT; // train thread
    mythr_t* passengers = NULL; // passenger threads
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

    mythreads_init();

    // initialize semaphores
    mythreads_sem_create(&semTrain, 0);
    mythreads_sem_create(&semBoarding, 1);
    mythreads_sem_create(&semDisembark, 0);
    mythreads_sem_create(&mtx, 1);

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
    res = mythreads_create(&trainT, train, &Targs);
    if (res) {
        fprintf(stderr, "ERROR: pthread_create train error value is: %d\n", res);
        perror("");
        return -1;
    }
    
    // create passenger threads when the user presses enter
    while(1) {
        mythreads_yield();
        printf("Press ENTER to create passenger %d\n", i);
        printf("Press any other key to exit\n");

        if (getchar() != '\n') {

            printf("Exiting...\n");

            destroy = true;

            // Signals train to terminate
            mythreads_sem_up(&semTrain);
          
            break;
        }

        printf("Creating passenger %d\n", i);

        // allocate memory for passenger struct
        passengers = realloc(passengers, (i + 1) * sizeof(mythr_t));
        if(passengers == NULL) {
            perror("ERROR: realloc passengers error\n");
            return -1;
        }
        
        // create passenger thread
        res = mythreads_create(&passengers[i], passenger, &Targs);
        if (res) {
            fprintf(stderr, "ERROR: pthread_create train error value is: %d\n", res);
            perror("");
            return -1;
        }

        i++;
    }

    // wait for train and passenger threads to terminate
    res = mythreads_join(&trainT);
    if ( res != 0) {
        fprintf(stderr, "ERROR: pthread_join train error value is: %d\n", res);
        perror("");
    }
    for (int j = 0; j < i; j++) {
        res = mythreads_join(&passengers[j]);
        if ( res != 0) {
            fprintf(stderr, "ERROR: pthread_join passenger %d error value is: %d\n", j, res);
            perror("");
        }

        res = mythreads_destroy(&passengers[j]);
        if ( res != 0) {
            fprintf(stderr, "ERROR: pthread_destroy passenger %d error value is: %d\n", j, res);
            perror("");
        }
    }

    // destroy semaphores
    mythreads_sem_destroy(&semTrain);
    mythreads_sem_destroy(&semBoarding);
    mythreads_sem_destroy(&semDisembark);
    mythreads_sem_destroy(&mtx);

    free(passengers);

    printf("Bye from Main.\n");

    return 0;
}