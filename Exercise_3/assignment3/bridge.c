// Created by: Dimitris Voitsidis, Iordana Gaisidou, Stavros Stathoudakis

#include "bridge.h"

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
    my_pthread_error(pthread_cond_init(&monitor->redQ, NULL));
    my_pthread_error(pthread_cond_init(&monitor->blueQ, NULL));
}

void* car(void *Cargs){
    carArgS *args = (carArgS *)Cargs;
    pthread_t self = pthread_self();

    printf("%sHello from %s Car %ld.\033[0m\n", args->color ? "\033[0;34m" : "\033[0;31m", (args->color ? "BLUE" : "RED"), self);

    my_pthread_error(pthread_mutex_lock(&args->monitor->mtx));
    while(1){
        // If the bridge is empty and the car is not the same color as the bridge
        if(*(args->carsCrossing) == 0 && *(args->color_bridge) != args->color){
            *(args->color_bridge) = args->color;
            *(args->carsCrossed) = 0;
        }

        // If the bridge is not the same color as the car or the bridge is full 
        // or the max number of cars crossed has been reached and there are still different color cars waiting,
        // sleep
        if(*(args->color_bridge) != args->color || *(args->carsCrossing) == args->maxCarsCrossing 
            || (*(args->carsCrossed) >= args->maxCarsCrossed && *(args->waitingOthers) > 0)){
            
            (*(args->waitingUs))++;
            if(args->color == RED){
                my_pthread_error(pthread_cond_wait(&args->monitor->redQ, &args->monitor->mtx));
            } else { // args->color == BLUE
                my_pthread_error(pthread_cond_wait(&args->monitor->blueQ, &args->monitor->mtx));
            }
            (*(args->waitingUs))--;
            continue;
        } // else
        break;
    }

    // Crossing the bridge
    (*(args->carsCrossing))++;
    (*(args->carsCrossed))++;
    if(*(args->waitingUs) > 0){
        if(args->color == RED){
            my_pthread_error(pthread_cond_signal(&args->monitor->redQ));
        } else { // args->color == BLUE
            my_pthread_error(pthread_cond_signal(&args->monitor->blueQ));
        }
    }
    my_pthread_error(pthread_mutex_unlock(&args->monitor->mtx)); 
    
    // Artificial delay to simulate crossing the bridge
    printf("%s  %s Car %ld is crossing the bridge.\033[0m\n", args->color ? "\033[1;34m" : "\033[1;31m", (args->color ? "BLUE" : "RED"), self );
    sleep(2);
    printf("%s  %s Car %ld has crossed the bridge.\033[0m\n",args->color ? "\033[1;34m" : "\033[1;31m", (args->color ? "BLUE" : "RED"), self);

    // Decrease the number of cars crossing the bridge
    my_pthread_error(pthread_mutex_lock(&args->monitor->mtx));
    (*(args->carsCrossing))--;

    printf("\033[1mCROSSING: %d ,CROSSED: %d, WAITING US: %d, WAITING OTHERS: %d \033[0m\n",
        *(args->carsCrossing), *(args->carsCrossed), *(args->waitingUs), *(args->waitingOthers));

    // If the bridge is empty and there are DIFFERENT color cars waiting, wake them up
    if(*(args->carsCrossing) == 0 && *(args->waitingOthers) > 0){
        if(args->color == BLUE){
            my_pthread_error(pthread_cond_signal(&args->monitor->redQ));
        } else { // args->color == RED
            my_pthread_error(pthread_cond_signal(&args->monitor->blueQ));
        }
    }
    
    // If more of same color cars can cross and there are SAME color cars waiting
    // or there are no different color cars waiting and there are same color cars waiting, wake them up
    if((*args->carsCrossed < args->maxCarsCrossed || *(args->waitingOthers) == 0) && *(args->waitingUs) > 0){
        if(args->color == RED){
            my_pthread_error(pthread_cond_signal(&args->monitor->redQ));
        } else { // args->color == BLUE
            my_pthread_error(pthread_cond_signal(&args->monitor->blueQ));
        }
    }

    my_pthread_error(pthread_mutex_unlock(&args->monitor->mtx));

    printf("%sBye from %s Car %ld.\033[0m\n", args->color ? "\033[0;34m" : "\033[0;31m", (args->color ? "BLUE" : "RED"), self);

    return NULL;
}


int main(int argc, char *argv[]){
    int N; // number of cars
    int res;
    char carColor;
    carColorE color_bridge;
    pthread_t *cars = NULL;
    monitor_var_t monitor;
    carArgS red_args;
    carArgS blue_args;
    int carsCrossing = 0;
    int carsCrossed = 0;
    int waitingBlue = 0;
    int waitingRed = 0;
    int i = 0;

    // check for correct number of arguments
    if (argc != 2 || atoi(argv[1]) <= 0){
        printf("Usage: %s <number of cars>\n", argv[0]);
        return -1;
    }

    N = atoi(argv[1]);

    // initialize semaphores
    init_monitor(&monitor);

    // initialize arguments for RED cars
    red_args.color = RED;
    red_args.carsCrossing = &carsCrossing;
    red_args.carsCrossed = &carsCrossed;
    red_args.maxCarsCrossing = N;
    red_args.maxCarsCrossed = 2*N;
    red_args.color_bridge = &color_bridge;
    red_args.waitingUs = &waitingRed;
    red_args.waitingOthers = &waitingBlue;
    red_args.monitor = &monitor;

    // initialize arguments for BLUE cars
    blue_args.color = BLUE;
    blue_args.carsCrossing = &carsCrossing;
    blue_args.carsCrossed = &carsCrossed;
    blue_args.maxCarsCrossing = N;
    blue_args.maxCarsCrossed = 2*N;
    blue_args.color_bridge = &color_bridge;
    blue_args.waitingUs = &waitingBlue;
    blue_args.waitingOthers = &waitingRed;
    blue_args.monitor = &monitor;

    while(1) {
        printf("Press R to create a RED car or B to create a BLUE car.\n");
        printf("Press any other key to exit\n");

        scanf(" %c", &carColor);
        if (carColor == 'R' || carColor == 'r'){ 
            cars = realloc(cars, (i + 1) * sizeof(pthread_t));
            if(cars == NULL) {
                perror("ERROR: realloc cars error\n");
                return -1;
            }
            
            res = pthread_create(&cars[i], NULL, car, &red_args);
            if (res) {
                fprintf(stderr, "ERROR: pthread_create car %d value is: %d\n",i, res);
                perror("");
                return -1;
            }

            i++;
        } else if (carColor == 'B' || carColor == 'b'){
            cars = realloc(cars, (i + 1) * sizeof(pthread_t));
            if(cars == NULL) {
                perror("ERROR: realloc cars error\n");
                return -1;
            }
            
            res = pthread_create(&cars[i], NULL, car, &blue_args);
            if (res) { 
                fprintf(stderr, "ERROR: pthread_create car %d error value is: %d\n",i, res);
                perror("");
                return -1;
            }

            i++;
        } else {
            printf("Exiting...\n");
            break;
        }
    }

    for (int j = 0; j < i; j++) {
        res = pthread_join(cars[j], NULL);
        if ( res != 0) {
            fprintf(stderr, "ERROR: pthread_join car %d error value is: %d\n", j, res);
            perror("");
        }
    }

    // destroy semaphores
    my_pthread_error(pthread_mutex_destroy(&monitor.mtx));
    my_pthread_error(pthread_cond_destroy(&monitor.blueQ));
    my_pthread_error(pthread_cond_destroy(&monitor.redQ));

    free(cars);

    printf("Bye from main\n");

    return(0);
}