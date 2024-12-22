#include "../../assignment2/mythreads.h"
#include <unistd.h>

typedef enum carColor { RED, BLUE } carColorE;

// Struct used to pass arguments in mythreads_create
typedef struct carArg {
    int maxCarsCrossing;
    int maxCarsCrossed;
    int *carsCrossing;
    int *carsCrossed;
    int *waitingUs;
    int *waitingOthers;
    carColorE color;
    carColorE *color_bridge;
    mysem_t *semUs;
    mysem_t *semOthers;
    mysem_t *mtx;
}carArgS;

void car(void *Cargs){
    carArgS *args = (carArgS *)Cargs;
    long self = mythreads_self();

    printf("%sHello from %s Car %ld.\033[0m\n", args->color ? "\033[0;34m" : "\033[0;31m", (args->color ? "BLUE" : "RED"), self);

    mythreads_sem_down(args->mtx);
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
            mythreads_sem_up(args->mtx);
            mythreads_sem_down(args->semUs);
            mythreads_sem_down(args->mtx);
            (*(args->waitingUs))--;
            continue;
        } // else
        break;
    }

    (*(args->carsCrossing))++;
    (*(args->carsCrossed))++;
    if(*(args->waitingUs) > 0){
        mythreads_sem_up(args->semUs);
    }
    mythreads_sem_up(args->mtx);

    //printf("after, CROSSING: %d ,CROSSED: %d, wUS: %d, wOTHERS: %d \n",*(args->carsCrossing), *(args->carsCrossed), *(args->waitingUs), *(args->waitingOthers));    
    // Artificial delay to simulate crossing the bridge
    printf("%s  %s Car %ld is crossing the bridge.\033[0m\n", args->color ? "\033[1;34m" : "\033[1;31m", (args->color ? "BLUE" : "RED"), self );
    mythreads_sleep(2);
    printf("%s  %s Car %ld has crossed the bridge.\033[0m\n",args->color ? "\033[1;34m" : "\033[1;31m", (args->color ? "BLUE" : "RED"), self);

    // Decrease the number of cars crossing the bridge
    mythreads_sem_down(args->mtx);
    (*(args->carsCrossing))--;

    printf("\033[1mCROSSING: %d ,CROSSED: %d, WAITING US: %d, WAITING OTHERS: %d \033[0m\n",
        *(args->carsCrossing), *(args->carsCrossed), *(args->waitingUs), *(args->waitingOthers));

    // If the bridge is empty and there are DIFFERENT color cars waiting, wake them up
    if(*(args->carsCrossing) == 0 && *(args->waitingOthers) > 0){
        mythreads_sem_up(args->semOthers);
    }
    
    // If more of same color cars can cross and there are SAME color cars waiting
    // or there are no different color cars waiting and there are same color cars waiting, wake them up
    if((*args->carsCrossed < args->maxCarsCrossed || *(args->waitingOthers) == 0) && *(args->waitingUs) > 0){
        mythreads_sem_up(args->semUs);
    }

    mythreads_sem_up(args->mtx);

    printf("%sBye from %s Car %ld.\033[0m\n", args->color ? "\033[0;34m" : "\033[0;31m", (args->color ? "BLUE" : "RED"), self);

    return;
}


int main(int argc, char *argv[]){
    int N; // number of cars
    int res;
    char carColor;
    carColorE color_bridge;
    mythr_t *cars = NULL;
    mysem_t semBlue, semRed, mtx;
    carArgS red_args;
    carArgS blue_args;
    int carsCrossing = 0;
    int carsCrossed = 0;
    int waitingBlue = 0;
    int waitingRed = 0;
    int i = 0;


    // check for correct number of arguments
    if (argc != 2){
        printf("Usage: %s <number of cars>\n", argv[0]);
        return -1;
    }

    N = atoi(argv[1]);

    // initialize semaphores
    mythreads_init();

    mythreads_sem_create(&semBlue, 1);
    mythreads_sem_create(&semRed, 1);
    mythreads_sem_create(&mtx, 1);
    
    // initialize arguments for RED cars
    red_args.color = RED;
    red_args.carsCrossing = &carsCrossing;
    red_args.carsCrossed = &carsCrossed;
    red_args.maxCarsCrossing = N;
    red_args.maxCarsCrossed = 2*N;
    red_args.semUs = &semRed;
    red_args.semOthers = &semBlue;
    red_args.mtx = &mtx;
    red_args.color_bridge = &color_bridge;
    red_args.waitingUs = &waitingRed;
    red_args.waitingOthers = &waitingBlue;

    // initialize arguments for BLUE cars
    blue_args.color = BLUE;
    blue_args.carsCrossing = &carsCrossing;
    blue_args.carsCrossed = &carsCrossed;
    blue_args.maxCarsCrossing = N;
    blue_args.maxCarsCrossed = 2*N;
    blue_args.semUs = &semBlue;
    blue_args.semOthers = &semRed;
    blue_args.mtx = &mtx;
    blue_args.color_bridge = &color_bridge;
    blue_args.waitingUs = &waitingBlue;
    blue_args.waitingOthers = &waitingRed;

    while(1) {
        printf("Press R to create a RED car or B to create a BLUE car.\n");
        printf("Press any other key to exit\n");

        scanf(" %c", &carColor);
        if (carColor == 'R' || carColor == 'r'){ 
            cars = realloc(cars, (i + 1) * sizeof(mythr_t));
            if(cars == NULL) {
                perror("ERROR: realloc cars error\n");
                return -1;
            }
            
            res = mythreads_create(&cars[i], car, &red_args);
            if (res) {
                fprintf(stderr, "ERROR: mythreads_create car %d value is: %d\n",i, res);
                return -1;
            }

            i++;
        } else if (carColor == 'B' || carColor == 'b'){
            cars = realloc(cars, (i + 1) * sizeof(mythr_t));
            if(cars == NULL) {
                perror("ERROR: realloc cars error\n");
                return -1;
            }
            
            res = mythreads_create(&cars[i], car, &blue_args);
            if (res) { 
                fprintf(stderr, "ERROR: mythreads_create car %d error value is: %d\n",i, res);
                return -1;
            }

            i++;
        } else {
            printf("Exiting...\n");
            break;
        }
    }

    for (int j = 0; j < i; j++) {
        res = mythreads_join(&cars[j]);
        if ( res != 0) {
            fprintf(stderr, "ERROR: mythreads_join car %d error value is: %d\n", j, res);
        }

        res = mythreads_destroy(&cars[j]);
        if ( res != 0) {
            fprintf(stderr, "ERROR: mythreads_destroy car %d error value is: %d\n", j, res);
        }
    }

    // destroy semaphores
    mythreads_sem_destroy(&semBlue);
    mythreads_sem_destroy(&semRed);
    mythreads_sem_destroy(&mtx);

    printf("MAIN:Eftasa EDW\n");
    
    free(cars);

    printf("Bye from main\n");

    return(0);
}