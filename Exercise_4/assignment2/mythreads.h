#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sched.h>

typedef enum timerAction { NEW, CONTINUE, DISABLE} timerActionE;

// thread struct
typedef struct mythread_t {
    ucontext_t context; 
    struct mythread_t *next; // next thread in the list
    time_t timetowake; // time to wake up
} mythr_t;

// List of threads
typedef struct {
    mythr_t *head;
    mythr_t *tail;
} mythr_list_t;

// List of semaphores
typedef struct mysemaphore_t {
    int val; // semaphore value
    bool destroyed; // semaphore status
    mythr_list_t blocked_threads; // list of blocked threads on this semaphore
} mysem_t;

// Thread functions
int mythreads_init();
int mythreads_create(mythr_t *thr, void (body)(void *), void *arg);
int mythreads_yield();
int mythreads_sleep(int secs);
int mythreads_join(mythr_t *thr);
int mythreads_destroy(mythr_t *thr);
long mythreads_self();

// Semaphore functions
int mythreads_sem_create(mysem_t *s, int val);
int mythreads_sem_down(mysem_t *s);
int mythreads_sem_up(mysem_t *s);
int mythreads_sem_destroy(mysem_t *s);
int mythreads_sem_getval(mysem_t *s);