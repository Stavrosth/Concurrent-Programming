#include "binarySemaphores.h"
#include "unistd.h"

void* thread(void* Wargs) {
    int return_up, return_down;
    mysem_t* semaphore = (mysem_t*) Wargs;
    pthread_t self = pthread_self();

    printf("Hello from thread %ld.\n", self);

    printf("THREAD %ld: call mysem_down\n", self);
    return_down = mysem_down(semaphore);
    printf("THREAD %ld: return of mysem_down: %d, value of semaphore %d\n", self, return_down, semctl(semaphore->semid, 0, GETVAL));

    sleep(2);
    printf("THREAD %ld: call mysem_up\n", self);
    return_up = mysem_up(semaphore);
    printf("THREAD %ld: return of mysem_up: %d, value of semaphore %d\n", self, return_up, semctl(semaphore->semid, 0, GETVAL));

    printf("THREAD %ld: call mysem_up\n", self);
    return_up = mysem_up(semaphore);
    printf("THREAD %ld: return of mysem_up: %d, value of semaphore %d\n", self, return_up, semctl(semaphore->semid, 0, GETVAL));

    printf("THREAD %ld: call mysem_up\n", self);
    return_up = mysem_up(semaphore);
    printf("THREAD %ld: return of mysem_up: %d, value of semaphore %d\n", self, return_up, semctl(semaphore->semid, 0, GETVAL));
    return NULL;
}

int main(int argc, char *argv[]) {
    int return_init, return_up, return_down, return_destroy;
    pthread_t t1,t2;
    mysem_t semaphore;
    int res;

    printf("MAIN: call mysem_init\n");
    return_init = mysem_init(&semaphore, 1);
    printf("MAIN: return of mysem_init: %d, value of semaphore %d\n", return_init, semctl(semaphore.semid, 0, GETVAL));

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
    printf("MAIN: return of mysem_down: %d, value of semaphore %d\n", return_down, semctl(semaphore.semid, 0, GETVAL));
    sleep(2);

    printf("MAIN: call mysem_up\n");
    return_up = mysem_up(&semaphore);
    printf("MAIN: return of mysem_up: %d, value of semaphore %d\n", return_up, semctl(semaphore.semid, 0, GETVAL));
    sleep(2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return_destroy = mysem_destroy(&semaphore);
    printf("MAIN: return of mysem_destroy: %d, value of semaphore %d\n", return_destroy, semctl(semaphore.semid, 0, GETVAL));
    
    return 0;
}