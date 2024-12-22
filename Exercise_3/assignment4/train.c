#include "train.h"

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
    my_pthread_error(pthread_cond_init(&monitor->trainQ, NULL));
    my_pthread_error(pthread_cond_init(&monitor->disembarkQ, NULL));
    my_pthread_error(pthread_cond_init(&monitor->boardingQ, NULL));
}

// Train thread
void* train(void* Targs) {
    TargsS* args = (TargsS*) Targs;
    printf("\033[0;33mHello from Train\033[0m\n");
    
    my_pthread_error(pthread_mutex_lock(&args->monitor->mtx));

    // while the user does not want to exit
    while (*(args->destroy) == false) {
        // wait for train to be full or the user to exit
        if((*(args->boarded_passengers) != args->train_capacity || *(args->travel) == TRAVELED) && *(args->destroy) == false ) {
            my_pthread_error(pthread_cond_wait(&args->monitor->trainQ, &args->monitor->mtx));
        }

        // Check if the user wants to exit
        if(*(args->destroy) == true && *(args->boarded_passengers) == 0) {
            break;
        }
        
        // Artificial delay of 1 second to simulate train journey
        printf("\033[1;33mTrain starts journey!\033[0m\n");
        *(args->travel) = TRAVELING;
        sleep(2);
        printf("\033[1;33mTrain completed the journey!\033[0m\n");
        *(args->travel) = TRAVELED;

        // Release passengers for disembarkation
        my_pthread_error(pthread_cond_signal(&args->monitor->disembarkQ));
    }

    // Signals passengers to not board and terminate if there are any waiting
    if ( *(args->waiting_passengers) != 0) {
        my_pthread_error(pthread_cond_signal(&args->monitor->boardingQ));
    }
    
    // Wait for all passengers to disembark in order to terminate the train
    while(*(args->boarded_passengers) != 0) {
        my_pthread_error(pthread_cond_wait(&args->monitor->trainQ, &args->monitor->mtx));
    }
    
    my_pthread_error(pthread_mutex_unlock(&args->monitor->mtx));

    printf("\033[0;33mBye from Train\033[0m\n");

    return NULL;
}

// Passenger thread
void* passenger(void* Pargs) {
    TargsS* args = (TargsS*) Pargs;

    printf("\033[0;36mHello from Passenger %ld\033[0m\n", pthread_self());

    sleep(1);

    my_pthread_error(pthread_mutex_lock(&args->monitor->mtx));

    // Signals the other passengers that a passenger is waiting
    (*(args->waiting_passengers))++;

    // passenger waits to board if necessary
    while((*(args->travel) != WAITING || *(args->boarded_passengers) == args->train_capacity) && *(args->destroy) == false) {
        my_pthread_error(pthread_cond_wait(&args->monitor->boardingQ, &args->monitor->mtx));
    }

    // Signals the other passengers that a passenger is no loger waiting
    (*(args->waiting_passengers))--;

    // Check if the user wants to exit
    if (*(args->destroy) == true) {
        // Signals other passengers to exit boarding
        my_pthread_error(pthread_cond_signal(&args->monitor->boardingQ));
        
        printf("\033[1;36mPassenger %ld did not even get on the train\033[0m\n", pthread_self());

        my_pthread_error(pthread_mutex_unlock(&args->monitor->mtx));
        
        return NULL;
    }

    printf("\033[1;36mPassenger %ld boarded\033[0m\n", pthread_self());

    // Signals the other passengers that a passenger has boarded
    (*(args->boarded_passengers))++;

    // Checks if the train is full
    if (*(args->boarded_passengers) == args->train_capacity) {
        my_pthread_error(pthread_cond_signal(&args->monitor->trainQ));
    } else {
        // Lets other passengers board
        my_pthread_error(pthread_cond_signal(&args->monitor->boardingQ));
    }

    // Waits to disembark after journey ends or the user wants to exit
    if(*(args->travel) != TRAVELED) {
        my_pthread_error(pthread_cond_wait(&args->monitor->disembarkQ, &args->monitor->mtx));
    }

    // Check if the user wants to exit
    printf("\033[1;36mPassenger %ld completed the journey and disembarked\033[0m\n", pthread_self());

    (*(args->boarded_passengers))--;
    if(*(args->destroy) == true && *(args->boarded_passengers) == 0) {
        // Signals train to terminate
        my_pthread_error(pthread_cond_signal(&args->monitor->trainQ));
    }

    // Checks if the train is empty and signals the waiting passengers to board
    if (*(args->boarded_passengers) == 0 && *(args->destroy) == false) {
        // Signals other passengers to board
        *(args->travel) = WAITING;
        my_pthread_error(pthread_cond_signal(&args->monitor->boardingQ));
    } else {
        // Signals the rest passengers to disembark
        my_pthread_error(pthread_cond_signal(&args->monitor->disembarkQ));
    }

    my_pthread_error(pthread_mutex_unlock(&args->monitor->mtx));

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
    monitor_var_t monitor;
    travelE travel = WAITING;
    
    // check for correct number of arguments
    if (argc != 2 || atoi(argv[1]) <= 0) {
        printf("Usage: %s <train capacity>\n", argv[0]);
        return -1;
    }

    N = atoi(argv[1]);

    // initialize semaphores
    init_monitor(&monitor);

    // intialize arguments for threads
    Targs.train_capacity = N;
    Targs.destroy = &destroy;
    Targs.boarded_passengers = &boarded_passengers;
    Targs.waiting_passengers = &waiting_passengers;
    Targs.monitor = &monitor;
    Targs.travel = &travel;
    
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

            my_pthread_error(pthread_mutex_lock(&monitor.mtx));
            destroy = true;

            // Signals train to terminate
            my_pthread_error(pthread_cond_signal(&monitor.trainQ));
            my_pthread_error(pthread_mutex_unlock(&monitor.mtx));

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

    // destroy monitor
    pthread_mutex_destroy(&monitor.mtx);
    pthread_cond_destroy(&monitor.trainQ);
    pthread_cond_destroy(&monitor.boardingQ);
    pthread_cond_destroy(&monitor.disembarkQ);

    free(passengers);

    printf("Bye from Main.\n");

    return 0;
}



