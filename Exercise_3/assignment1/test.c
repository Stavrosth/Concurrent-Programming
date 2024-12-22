#include "binarySemaphores.h"
#include "unistd.h"

int mysem_getid(mysem_t *s) {
    return s->sem_id;
}

int mysem_getval(mysem_t *s) {
    return s->sem_val;
}

void* thread(void* Wargs) {
    int return_up, return_down;
    mysem_t* semaphore = (mysem_t*) Wargs;
    pthread_t self = pthread_self();

    printf("Hello from thread %ld.\n", self);

    printf("THREAD %ld: call mysem_down\n", self);
    return_down = mysem_down(semaphore);
    printf("THREAD %ld: return of mysem_down: %d, value of semaphore %d\n", self , return_down, mysem_getval(semaphore));

    sleep(2);
    printf("THREAD %ld: call mysem_up\n", self);
    return_up = mysem_up(semaphore);
    printf("THREAD %ld: return of mysem_up: %d, value of semaphore %d\n", self, return_up, mysem_getval(semaphore));

    printf("THREAD %ld: call mysem_up\n", self);
    return_up = mysem_up(semaphore);
    printf("THREAD %ld: return of mysem_up: %d, value of semaphore %d\n", self, return_up, mysem_getval(semaphore));

    printf("THREAD %ld: call mysem_up\n", self);
    return_up = mysem_up(semaphore);
    printf("THREAD %ld: return of mysem_up: %d, value of semaphore %d\n", self, return_up, mysem_getval(semaphore));
    return NULL;
}

int main(int argc, char *argv[]) {
    int return_init, return_up, return_down, return_destroy;
    pthread_t t1,t2;
    mysem_t semaphore;
    int res;

    printf("MAIN: call mysem_init\n");
    return_init = mysem_init(&semaphore, 1);
    printf("MAIN: return of mysem_init: %d, value of semaphore %d\n", return_init, mysem_getval(&semaphore));

    res = pthread_create(&t1, NULL, thread, &semaphore);
    if (res) {
        fprintf(stderr, "ERROR: pthread_create %ld error value is:%d\n", t1 ,res);
    }

    res = pthread_create(&t2, NULL, thread, &semaphore);
    if (res) {
        fprintf(stderr, "ERROR: pthread_create %ld error value is:%d\n", t2 ,res);
    }

    printf("MAIN: call mysem_down\n");
    return_down = mysem_down(&semaphore);
    printf("MAIN: return of mysem_down: %d, value of semaphore %d\n", return_down, mysem_getval(&semaphore));
    sleep(2);

    printf("MAIN: call mysem_up\n");
    return_up = mysem_up(&semaphore);
    printf("MAIN: return of mysem_up: %d, value of semaphore %d\n", return_up, mysem_getval(&semaphore));
    sleep(2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return_destroy = mysem_destroy(&semaphore);
    printf("MAIN: return of mysem_destroy: %d, value of semaphore %d\n", return_destroy, mysem_getval(&semaphore));
    
    return 0;
}