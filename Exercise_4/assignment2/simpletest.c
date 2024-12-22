#include "mythreads.h"
#include "unistd.h"

typedef struct {
    mysem_t *semaphore;
    mythr_t *self;
} functionArgS;

void thread(void* Wargs) {
    int return_up, return_down;
    functionArgS* args = (functionArgS*) Wargs;
    
    printf("Hello from thread %ld.\n", (long) (args->self));

    printf("THREAD %ld: call mythreads_yield\n", (long) (args->self));
    mythreads_yield();
    printf("THREAD %ld: return of mythreads_yield\n", (long) (args->self));

    printf("THREAD %ld: call mythreads_sleep 2\n", (long) (args->self));
    mythreads_sleep(2);
    printf("THREAD %ld: return of mythreads_sleep 2\n", (long) (args->self));

    printf("THREAD %ld: call mythreads_semdown\n", (long) (args->self));
    return_down = mythreads_sem_down(args->semaphore);
    printf("THREAD %ld: return of mythreads_semdown: %d, value of semaphore %d\n", (long) (args->self) , return_down, mythreads_sem_getval(args->semaphore));
    
    for(int i = 0; i < 1024*1024*16; i++) {}

    printf("THREAD %ld: call mythreads_yield\n", (long) (args->self));
    mythreads_yield();
    printf("THREAD %ld: return of mythreads_yield\n", (long) (args->self));

    printf("THREAD %ld: call mythreads_sem_up\n", (long) (args->self));
    return_up = mythreads_sem_up(args->semaphore);
    printf("THREAD %ld: return of mythreads_sem_up: %d, value of semaphore %d\n", (long) (args->self), return_up, mythreads_sem_getval(args->semaphore));

    printf("THREAD %ld: call mythreads_sem_up\n", (long) (args->self));
    return_up = mythreads_sem_up(args->semaphore);
    printf("THREAD %ld: return of mythreads_sem_up: %d, value of semaphore %d\n", (long) (args->self), return_up, mythreads_sem_getval(args->semaphore));

    return;
}

int main(int argc, char *argv[]) {
    int return_init, return_up, return_down, return_destroy;
    mythr_t t1,t2;
    mysem_t semaphore;
    int res;
    functionArgS args1, args2;

    args1.self = &t1;
    args2.self = &t2;
    args1.semaphore = &semaphore;
    args2.semaphore = &semaphore;

    printf("MAIN: call mythreads_init\n");
    res = mythreads_init();
    printf("MAIN: return of mythreads_init: %d\n", res);

    printf("MAIN: call mythreads_sem_create with initial 1\n");
    return_init = mythreads_sem_create(&semaphore, 1);
    printf("MAIN: return of mythreads_sem_create: %d, value of semaphore %d\n", return_init, mythreads_sem_getval(&semaphore));

    printf("MAIN: call mythreads_yield\n");
    mythreads_yield();
    printf("MAIN: return mythreads_yield\n");

    printf("MAIN: call mythreads_sleep 2\n");
    mythreads_sleep(2);
    printf("MAIN: return of mythreads_sleep 2\n");

    printf("MAIN: call mythreads_create1\n");
    res = mythreads_create(&t1, thread, &args1);
    if (res) {
        fprintf(stderr, "ERROR: pthread_create %ld error value is:%d\n", (long) &t1 ,res);
    }
    printf("MAIN: return mythreads_create1 res:%d\n", res);


    printf("MAIN: call mythreads_create2\n");
    res = mythreads_create(&t2, thread, &args2);
    if (res) {
        fprintf(stderr, "ERROR: pthread_create %ld error value is:%d\n", (long) &t2 ,res);
    }
    printf("MAIN: return mythreads_create2 res:%d\n", res);

    printf("MAIN: call mythreads_yield\n");
    mythreads_yield();
    printf("MAIN: return mythreads_yield\n");

    printf("MAIN: call mythreads_sleep 2\n");
    mythreads_sleep(2);
    printf("MAIN: return of mythreads_sleep 2\n");

    printf("MAIN: call mythreads_semdown\n");
    return_down = mythreads_sem_down(&semaphore);
    printf("MAIN: return of mythreads_semdown: %d, value of semaphore %d\n", return_down, mythreads_sem_getval(&semaphore));

    printf("MAIN: call mythreads_semup\n");
    return_up = mythreads_sem_up(&semaphore);
    printf("MAIN: return of mythreads_semup: %d, value of semaphore %d\n", return_up, mythreads_sem_getval(&semaphore));
    
    printf("MAIN: call mythreads_sleep 1\n");
    mythreads_sleep(1);
    printf("MAIN: return of mythreads_sleep 1\n");

    printf("MAIN: call mythreads_join t1\n");
    mythreads_join(&t1);
    printf("MAIN: return mythreads_join t1\n");

    printf("MAIN: call mythreads_join t2\n");
    mythreads_join(&t2);
    printf("MAIN: return mythreads_join t2\n");

    printf("MAIN: call mythreads_sem_destroy\n");
    return_destroy = mythreads_sem_destroy(&semaphore);
    printf("MAIN: return of mythreads_sem_destroy: %d, value of semaphore %d\n", return_destroy, mythreads_sem_getval(&semaphore));
    
    printf("MAIN: call mythreads_destroy t1\n");
    mythreads_destroy(&t1);
    printf("MAIN: return mythreads_destroy t1\n");

    printf("MAIN: call mythreads_destroy t2\n");
    mythreads_destroy(&t2);
    printf("MAIN: return mythreads_destroy t2\n");

    return 0;
}