#include "mythreads.h"

static mythr_list_t ready;
static mythr_t *sleeping, *stopped;
static mythr_t main_thread;
static ucontext_t terminate_co;
static struct itimerval thread_timer, disable_timer, curr_timer; // Timer struct for the scheduler
static mysem_t **sem_array = NULL;
static int sem_count = 0;

void mythreads_wake();
void mythreads_SIGVTALRM_handler(int signum);
void mythreads_terminate();
void mythreads_exit();
void mythreads_timer(timerActionE action);
    
// Initialize the environment for the threads
int mythreads_init() {  
    void* terminate_stack;

    // Set up the signal handler
    if(signal(SIGVTALRM, mythreads_SIGVTALRM_handler) == SIG_ERR){
        perror("mythreads_init: signal:");
        exit(-1);
    }

    // Initialise the lists
    // sleeping list
    sleeping = NULL;
    // stopped list
    stopped = NULL;
    // ready list
    ready.head = &main_thread;
    ready.tail = &main_thread;

    // Create the main thread
    main_thread.next = &main_thread;
    main_thread.timetowake = 0;
    
    // get context of main
    if (getcontext(&main_thread.context) == -1) {
        perror("mythreads_init: getcontext");
        exit(-1);
    }

    // Assigns stack to the new coroutine
    terminate_co = main_thread.context;

    terminate_stack = malloc(SIGSTKSZ);
    if (terminate_stack == NULL) {
        perror("mythreads_create: malloc for stack");
        exit(-1);
    }

    terminate_co.uc_stack.ss_sp = terminate_stack;
    terminate_co.uc_stack.ss_size = SIGSTKSZ;
    terminate_co.uc_link = NULL;

    // Create the context for termination
    makecontext(&terminate_co, mythreads_terminate, 0);

    if(atexit(mythreads_exit) != 0){
        perror("mythreads_init: atexit:");
        exit(-1);
    }

    // Configure the timer to expire every 1 `microsecond`
    thread_timer.it_value.tv_sec = 0;      // seconds
    thread_timer.it_value.tv_usec = 1;    // microseconds
    // Interval for periodic expiration
    thread_timer.it_interval.tv_sec = 0;   // seconds
    thread_timer.it_interval.tv_usec = 0;  // microseconds

    // Disable the timer
    disable_timer.it_value.tv_sec = 0;      // seconds
    disable_timer.it_value.tv_usec = 0;    // microseconds
    // Interval for periodic expiration
    disable_timer.it_interval.tv_sec = 0;   // seconds
    disable_timer.it_interval.tv_usec = 0;  // microseconds

    // Start the timer
    mythreads_timer(NEW);

    return 0;
}

// Creates a new thread
int mythreads_create(mythr_t *thr, void (body)(void *), void *arg) {
    void *thr_stack;

    mythreads_timer(DISABLE);

    // Error checking
    if (thr == NULL) {
        fprintf(stderr, "mythreads_create: thr is NULL\n");
        exit(-1);
    }
    if (body == NULL) {
        fprintf(stderr, "mythreads_create: body is NULL\n");
        exit(-1);
    }

    // Assigns stack to the new coroutine
    thr_stack = malloc(SIGSTKSZ);
    if (thr_stack == NULL) {
        perror("mythreads_create: malloc for stack");
        exit(-1);
    }

    thr->context = main_thread.context;
    thr->context.uc_stack.ss_sp = thr_stack;
    thr->context.uc_stack.ss_size = SIGSTKSZ;

    thr->context.uc_link = &terminate_co;
    thr->timetowake = 0;

    // Initialize the context of the new thread
    if (arg == NULL) {
        makecontext(&thr->context, (void(*)(void)) body, 0);
    } else {
        makecontext(&thr->context, (void(*)(void)) body, 1, arg);
    }

    // Add the new thread to ready list
    ready.tail->next = thr;
    thr->next = ready.head;
    ready.tail = thr;

    mythreads_timer(CONTINUE);

    return 0;
}

// Voluntarily give up the CPU to the next thread
int mythreads_yield() {

    mythreads_timer(DISABLE);

    mythreads_wake();

    // If there is only one thread, return
    if (ready.tail == ready.head) {
        // fprintf(stderr, "mythreads_yield: There is only one thread\n"); // FOR DEBUGGING PURPOSES

        mythreads_timer(CONTINUE);

        return 1;
    }// else 

    // Switch to the next thread in ready list
    ready.tail = ready.head;
    ready.head = ready.head->next;

    // Start the timer
    mythreads_timer(NEW);

    // Switch to the next thread
    if (swapcontext(&ready.tail->context, &ready.head->context) != 0) {
        perror("mythreads_SIGALRM_handler:");
        exit(-1);
    }

    return 0;
}

// sleep for a specified amount of time
int mythreads_sleep(int secs) {
    struct timeval tmp_time;
    mythr_t *tmp;
    mythr_t *current_thread;
    
    // stop the timer
    mythreads_timer(DISABLE);
    
    // Error checking
    if (secs <= 0) {
        fprintf(stderr, "mythreads_sleep: secs is not positive\n");
        exit(-1);
    }

    mythreads_wake();

    current_thread = ready.head;

    // Calculate the time to wake up
    gettimeofday(&tmp_time, NULL);
    current_thread->timetowake = tmp_time.tv_sec + (time_t) secs;

    // If there is only one thread in the list sleep and keep CPU
    if(ready.head == ready.tail) {
        
        // Sleep for the least time needed
        if ( sleeping == NULL || current_thread->timetowake <= sleeping->timetowake) {
            sleep((int) (current_thread->timetowake - tmp_time.tv_sec));

            mythreads_timer(NEW);
        } else {
            tmp = sleeping;

            // Find the correct position to insert the thread in the sleeping list
            // They are placed in ascending order of the time they will wake up
            while (tmp->next != NULL && current_thread->timetowake > tmp->next->timetowake) {
                tmp = tmp->next;
            }

            // Insert the thread in the sleeping list
            current_thread->next = tmp->next;
            tmp->next = current_thread;

            // Sleep for the least time needed
            sleep((int) (sleeping->timetowake - tmp_time.tv_sec));

            mythreads_wake();

            mythreads_timer(NEW);

            // Move to the next thread
            if (swapcontext(&current_thread->context, &ready.head->context) != 0) {
                perror("mythreads_sleep: swapcontext:");
                exit(-1);
            }
        }

        return 0;
    } // else

    // Remove the head thread from the ready list
    ready.tail->next = ready.head->next;
    ready.head = ready.head->next;
    
    // add the thread to the sleeping list in the correct position
    tmp = sleeping;

    if (sleeping == NULL) { // if sleeping list is empty
        sleeping = current_thread;
        sleeping->next = NULL;
    } else if(current_thread->timetowake < sleeping->timetowake) { // Enters the list at the beginning
        current_thread->next = sleeping;
        sleeping = current_thread;
    } else {
        // Find the correct position to insert the thread in the sleeping list
        // They are placed in ascending order of the time they will wake up
        while (tmp->next != NULL && current_thread->timetowake > tmp->next->timetowake) {
            tmp = tmp->next;
        }

        // Insert the thread in the sleeping list
        current_thread->next = tmp->next;
        tmp->next = current_thread;
    }

    mythreads_timer(NEW);

    // Move to the next thread
    if (swapcontext(&current_thread->context, &ready.head->context) != 0) {
        perror("mythreads_sleep: swapcontext:");
        exit(-1);
    }

    return 0;
} 

// Check if there is a thread that needs to wake up from sleeping
void mythreads_wake() {
    struct timeval tmp_time;
    mythr_t *tmp_thread;

    // Get the current time
    if(gettimeofday(&tmp_time, NULL)) {
        perror("mythreads_SIGALRM_handler: gettimeofday:");
        exit(-1);
    }

    if (sleeping != NULL) {
        if (sleeping->timetowake <= tmp_time.tv_sec) {
            tmp_thread = sleeping;

            // remove thread from sleeping list
            sleeping->timetowake = 0;
            sleeping = sleeping->next;

            // Add the thread to the ready list
            if (ready.head == NULL) { // if ready list is empty
                tmp_thread->next = tmp_thread;
                ready.head = tmp_thread;
                ready.tail = tmp_thread;
            } else { // Add the thread to the end of the list
                tmp_thread->next = ready.head;
                ready.tail->next = tmp_thread;
                ready.tail = tmp_thread;
            }
        }
    }
}

// Wait for the specified thread to enter the stopped state
int mythreads_join(mythr_t *thr) {
    mythr_t *tmp;

    mythreads_timer(DISABLE);

    // Error checking
    if (thr == NULL) {
        fprintf(stderr, "mythreads_join: thr is NULL\n");
        exit(-1);
    }

    /* Could be better/more efficient and block the thread until the asked thread stops*/
    // Wait for the specified thread to enter the stopped state
    while (1) {
        // Wait untill there is a thread in the stopped list
        while ( stopped == NULL ) {
            mythreads_yield(); 
        }

        // Search for the thread in the stopped list
        tmp = stopped;

        // Traverse through the stopped list
        while (tmp != NULL) {
            if (thr == tmp) {
                break;
            }
            tmp = tmp->next;
        }

        if (tmp == thr) { // success
            break;
        } else {
            mythreads_yield();
        }
    }

    mythreads_timer(CONTINUE);

    return 0;
}

// Destroy the thread and no-one can access it after
int mythreads_destroy(mythr_t *thr) {
    mythr_t *tmp;
    bool found = false;
    int i;

    mythreads_timer(DISABLE);

    // Error checking
    if (thr == NULL) {
        fprintf(stderr, "mythreads_join: thr is NULL\n");
        exit(-1);
    }

    /* Start searching the thread in the lists */

    // Search for the thread in the stopped list
    if(stopped != NULL) {
        tmp = stopped;

        if ( thr == tmp) {
            found = true;
            stopped = tmp->next;
        } else {
            // Traverse through the stopped list
            while (tmp->next != NULL) {
                if (thr == tmp->next) {
                    found = true;
                    tmp->next = tmp->next->next;
                    break;
                }
                tmp = tmp->next;
            }
        }
    }

    // Search for the thread in the ready list
    if (found == false && ready.head != ready.tail) { 
        tmp = ready.head;
        
        if(tmp == thr) { // suicide
            fprintf(stderr, "A thread just suicided!");
            found = true;
            ready.head = ready.head->next;
            ready.tail->next = ready.head;
        }   
    }

    // Search for the thread in the sleeping list
    if (found == false && sleeping != NULL) {
        tmp = sleeping;

        if ( thr == tmp) {
            found = true;
            sleeping = tmp->next;
        } else {
            // Traverse through the stopped list
            while (tmp->next != NULL) {
                if (thr == tmp->next) {
                    found = true;
                    tmp->next = tmp->next->next;
                    break;
                }
                tmp = tmp->next;
            }
        }
    }

    // Search for the thread in the semaphore array
    if (found == false) {
        // Traverse through the semaphore array
        for (i = 0; i < sem_count; i++) {
            if(sem_array[i]->blocked_threads.head != NULL) {
                tmp = sem_array[i]->blocked_threads.head;

                // Traverse through the blocked list of the semaphore
                if ( thr == tmp ) {
                    found = true;
                    sem_array[i]->blocked_threads.head = tmp->next;
                } else {
                    while (tmp->next != NULL) {
                        if (thr == tmp->next) {
                            found = true;
                            tmp->next = tmp->next->next;
                            break;
                        }
                        tmp = tmp->next;
                    }
                }
            }

            if (found == true) {
                break;
            }
        }
    }

    // Thread not found
    if (found == false) {
        mythreads_timer(CONTINUE);

        fprintf(stderr, "mythreads_destroy: Thread not found\n");
        
        return -1;
    }

    // Free the stack of the thread
    free(thr->context.uc_stack.ss_sp);
    
    mythreads_timer(CONTINUE);

    return 0;
}

// Create a new semaphore
int mythreads_sem_create(mysem_t *s, int val) {

    mythreads_timer(DISABLE);

    // Error checking
    if (s == NULL) {
        fprintf(stderr, "mythreads_sem_create: s is NULL\n");
        exit(-1);
    }
    
    if (val != 0 && val != 1) {
        fprintf(stderr, "mythreads_sem_create: val is not 0 or 1\n");
        exit(-1);
    }

    // Initialize the semaphore
    s->val = val;
    s->destroyed = false;
    s->blocked_threads.head = NULL;
    s->blocked_threads.tail = NULL;

    // Add the semaphore to the semaphore array
    sem_count++;
    sem_array = realloc(sem_array, sizeof(mysem_t*) * sem_count);
    if(sem_array == NULL) {
        perror("mythreads_sem_create: realloc:");
        exit(-1);
    }
    sem_array[sem_count - 1] = s;
    
    mythreads_timer(CONTINUE);

    return 0;
}

// Decrease the semaphore value
int mythreads_sem_down(mysem_t *s) {
    mythr_t *current_thread;
    struct timeval tmp_time;

    mythreads_timer(DISABLE);

    // Error checking
    if (s == NULL) {
        fprintf(stderr, "mythreads_sem_down: s is NULL\n");
        exit(-1);
    }

    // If the semaphore is destroyed, return 2
    if (s->destroyed) {
        fprintf(stderr, "mythreads_sem_down: s is destroyed\n");

        mythreads_timer(CONTINUE);

        return 2;
    }
    // If the semaphore value is 1 change it to 0 and return
    if (s->val == 1) {
        s->val = 0;
     
        mythreads_timer(CONTINUE);

        return 1; 
    } // else give up the CPU

    mythreads_wake();

    current_thread = ready.head;

    // Remove the thread from the ready list and traverse to the next thread
    if (ready.head == ready.tail) { // if only current thread is in the list
        ready.head = NULL;
        ready.tail = NULL;

        gettimeofday(&tmp_time, NULL);

        sleep((int) (sleeping->timetowake - tmp_time.tv_sec));

        mythreads_wake();
    } else { // Remove the running thread from the ready list
        ready.head = ready.head->next;
        ready.tail->next = ready.head;
    }

    // Add the thread to the blocked list of the semaphore
    if(s->blocked_threads.head == NULL) { //list is empty
        s->blocked_threads.head = current_thread;
        s->blocked_threads.tail = current_thread;
    } else { // Add the thread to the end of the list
        s->blocked_threads.tail->next = current_thread;
        s->blocked_threads.tail = current_thread;
    }

    mythreads_timer(NEW);

    if(swapcontext(&current_thread->context, &ready.head->context) != 0){
        perror("mythreads_sem_down: swapcontext:");
        exit(-1);
    }

    return 0;
}

// Increase the semaphore value
int mythreads_sem_up(mysem_t *s) {
    mythr_t *current_thread;

    mythreads_timer(DISABLE);
    
    // Error checking
    if (s == NULL) {
        fprintf(stderr, "mythreads_sem_up: s is NULL\n");
        exit(-1);
    }

    // if the semaphore is destroyed, return 2
    if (s->destroyed) { 
        fprintf(stderr, "mythreads_sem_up: s is destroyed\n");

        mythreads_timer(CONTINUE);

        return 2;
    }
    // If the semaphore value is already 1, return
    if ( s->val == 1) {
        fprintf(stderr, "mythreads_sem_up: s is already 1\n");
        
        mythreads_timer(CONTINUE);

        return 1;
    } // else

    // Increment the semaphore value if the list is empty
    if(s->blocked_threads.head == NULL) { //list is empty
        s->val = 1;
        
        mythreads_timer(CONTINUE);

        return 0;
    } 
    // Remove the thread from the blocked list 
    if (s->blocked_threads.head == s->blocked_threads.tail) { // if only current thread is in the list
        current_thread = s->blocked_threads.head;
        s->blocked_threads.head = NULL;
        s->blocked_threads.tail = NULL;
    }else { // Remove the thread from the head of the blocked list
        current_thread = s->blocked_threads.head;
        s->blocked_threads.head = s->blocked_threads.head->next;
    }

    // Add the thread to the end of the ready list
    current_thread->next = ready.head;
    ready.tail->next = current_thread;
    ready.tail = current_thread;

    mythreads_timer(CONTINUE);

    return 0;
}

// Destroy the semaphore
int mythreads_sem_destroy(mysem_t *s) {
    int i;
    
    mythreads_timer(DISABLE);

    // Error checking
    if (s == NULL) {
        fprintf(stderr, "mythreads_sem_destroy: s is NULL\n");
        exit(-1);
    }

    // if the semaphore is destroyed, return
    if (s->destroyed) {
        fprintf(stderr, "mythreads_sem_up: s is destroyed\n");

        mythreads_timer(CONTINUE);

        return 2;
    }

    // Set the semaphore to destroyed
    s->destroyed = true;

    // Remove the semaphore from the semaphore array
    for (i = 0; i < sem_count; i++) {
        if (sem_array[i] == s) {
            sem_array[i] = sem_array[sem_count - 1];
            break;
        }
    }

    // Downsize the semaphore array
    sem_count--;
    sem_array = realloc(sem_array, sizeof(mysem_t*) * sem_count);
    if(sem_array == NULL && sem_count != 0) {
        perror("mythreads_sem_destroy: realloc:");
        exit(-1);
    }

    mythreads_timer(CONTINUE);

    return 0;
}

// Signal handler for the timer
void mythreads_SIGVTALRM_handler(int signum) {
    printf("SIGALRM\n");
    // wake threads if they need to wake up
    mythreads_wake();

    // Switch to the next thread
    if(ready.tail == ready.head) {
        // Start the timer
        mythreads_timer(NEW);

        return;
    }

    // Switch to the next thread in the list
    ready.tail = ready.head;
    ready.head = ready.tail->next;

    if (swapcontext(&ready.tail->context, &ready.head->context) != 0) {
        perror("mythreads_SIGALRM_handler:");
        exit(-1);
    }
}

// Is called when a thread wants to terminate
void mythreads_terminate() {
    mythr_t *current_thread;
    struct timeval tmp_time;

    mythreads_timer(DISABLE);
  
    mythreads_wake();

    current_thread = ready.head;

    // Removes the thread from the ready list
    if (ready.head == ready.tail) { // if only current thread is in the list
        ready.head = NULL;
        ready.tail = NULL;

        gettimeofday(&tmp_time, NULL);

        sleep((int) (sleeping->timetowake - tmp_time.tv_sec));

        mythreads_wake();
    } else { // Remove the running thread from the ready list
        ready.tail->next = ready.head->next;
        ready.head = ready.head->next;
    }

    // Add the thread to the stopped list
    if (stopped == NULL) { // if sleeping list is empty
        stopped = current_thread;
        stopped->next = NULL;
    } else { // Enters the list at the beginning
        current_thread->next = stopped;
        stopped = current_thread;
    }

    mythreads_timer(NEW);

    // Switch to the next thread
    if(swapcontext(&current_thread->context, &ready.head->context) != 0){
        perror("mythreads_terminate: swapcontext:");
        exit(-1);
    }

    fprintf(stderr, "SHOULD NEVER SEE THIS!!!\n");

    return;
}

// Function to be called when the program exits
void mythreads_exit() {
    /******  FOR DEBUGGING PURPOSES ******/
    // mythr_t *tmp;

    // printf("\n");
    // printf("terminate MAIN: %ld\n", (long) &main_thread);

    // printf("ready\n");
    // tmp = ready.head;
    // do {
    //     printf("terminate ready THREAD: %ld\n", (long) tmp);
    //     tmp = tmp->next;
    // } while (ready.head != tmp);

    // printf("stop\n");
    // tmp = stopped;
    // while (tmp != NULL) {
    //     printf("terminate stop THREAD: %ld\n", (long) tmp);
    //     tmp = tmp->next;
    // }

    // printf("sleeping\n");
    // tmp = sleeping;
    // while (tmp != NULL) {
    //     printf("terminate sleep THREAD: %ld\n", (long) tmp);
    //     tmp = tmp->next;
    // }

    // printf("semaphores\n");
    // for(int i = 0; i < sem_count; i++) {
    //     printf("terminate semaphore: %d\n", i);
    //     tmp = sem_array[i]->blocked_threads.head;
    //     while (tmp != NULL) {
    //         printf("terminate blocked THREAD: %ld\n", (long) tmp);
    //         tmp = tmp->next;
    //     }
    // }

    // Free the stack of the terminate context
    free(terminate_co.uc_stack.ss_sp);
}

// Returns the calling thread
long mythreads_self() {
    return (long) ready.head;
}

// Timer function
void mythreads_timer(timerActionE action) {
    switch (action)
    {
    case NEW:
        if (setitimer(ITIMER_VIRTUAL, &thread_timer, NULL) != 0) {
            perror("mythreads_terminate: setitimer:");
            exit(-1);
        }
        break;
    case CONTINUE:
        if (setitimer(ITIMER_VIRTUAL, &curr_timer, NULL) != 0) {
            perror("mythreads_create: setitimer:");
            exit(-1);
        }
        break;
    case DISABLE:
        if (setitimer(ITIMER_VIRTUAL, &disable_timer, &curr_timer) != 0) {
            perror("mythreads_sem_destroy: setitimer:");
            exit(-1);
        }
        break;
    default:
        break;
    }
   return; 
}

// Get the value of the semaphore
int mythreads_sem_getval(mysem_t *s) {
    return s->val;
}