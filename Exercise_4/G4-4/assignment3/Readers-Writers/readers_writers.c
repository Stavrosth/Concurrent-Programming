#include "../assignment2/mythreads.h"
#include <time.h>

typedef struct {
    mysem_t *write_sem;
    mysem_t *read_sem;
    int iterations;
    int time;
    int *rds;
} functionArgS;

// Reads from the buffer
void reader(void* Rargs) {
    functionArgS* args = (functionArgS*) Rargs;
    long self = mythreads_self();

    printf("Hello from Writer %ld\n", self);

    for (int i = 0; i < args->iterations; i++) {
        mythreads_sem_down(args->read_sem);
        if (*(args->rds) == 0) {
            mythreads_sem_down(args->write_sem);
        }

        (*(args->rds))++;

        mythreads_sem_up(args->read_sem);

        // Simulate reading
        printf("\033[1;31mReader %ld: Iteration %d Start Reading for %d\033[0m\n", self, i, args->time);
        printf("\033[1mReaders: %d\033[0m\n", *(args->rds));
        for (int j = 0; j < 1024*1024*8; j++) {}
        // mythreads_sleep(args->time);
        printf("\033[0;31mReader %ld: Iteration %d Stop Reading\033[0m\n", self, i);

        mythreads_sem_down(args->read_sem);

        (*(args->rds))--;
        if (*(args->rds) == 0) {
            mythreads_sem_up(args->write_sem);
        }

        mythreads_sem_up(args->read_sem);

        mythreads_yield();
    }

    printf("Bye from Reader %ld\n", self);

    return;
}

// Writes in the buffer
void writer(void* Wargs) {
    functionArgS* args = (functionArgS*) Wargs;
    long self = mythreads_self();

    printf("Hello from Writer %ld\n", self);

    for (int i = 0; i < args->iterations; i++) {
        mythreads_sem_down(args->write_sem);

        // Simualate writing
        printf("\033[1mReaders: %d\033[0m\n", *(args->rds));
        printf("\033[1;34m Writer %ld: Iteration %d Start Writing for %d\033[0m\n", self, i, args->time);
        // mythreads_sleep(args->time);
        for (int j = 0; j < 1024*1024*8; j++) {}
        printf("\033[0;34m Writer %ld: Iteration %d Stop Writing\033[0m\n", self, i);

        mythreads_sem_up(args->write_sem);

        mythreads_yield();
    }

    printf("Bye from Writer %ld\n", self);

    return;
}

int main(int argc, char *argv[]) {
    mythr_t *reader_threads;
    mythr_t *writer_threads;
    mysem_t read_sem, write_sem;
    functionArgS *reader_args;
    functionArgS *writer_args;
    int readers_no, writers_no, iterations, rds;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <readers> <writers> <iterations> \n", argv[0]);
        exit(-1);
    }

    printf("Hello from Main\n");

    srand(time(NULL));

    // Get the number of readers, writers and iterations from the command line
    readers_no = atoi(argv[1]);
    writers_no = atoi(argv[2]);
    iterations = atoi(argv[3]);
    rds = 0;

    // Initialize the environment for the threads
    mythreads_init();

    // Initialize the semaphores
    mythreads_sem_create(&read_sem, 1);
    mythreads_sem_create(&write_sem, 1);

    // Allocate memory for the threads and their arguments
    reader_threads = (mythr_t*) malloc(readers_no * sizeof(mythr_t));
    writer_threads = (mythr_t*) malloc(writers_no * sizeof(mythr_t));
    reader_args = (functionArgS*) malloc(readers_no * sizeof(functionArgS));
    writer_args = (functionArgS*) malloc(writers_no * sizeof(functionArgS));
    if (reader_threads == NULL || writer_threads == NULL || reader_args == NULL || writer_args == NULL) {
        perror("ERROR: mallocs");
        return (-1);
    }

    // Initialize the arguments for the readers
    for (int i = 0; i < readers_no; i++) {
        reader_args[i].write_sem = &write_sem;
        reader_args[i].read_sem = &read_sem;
        reader_args[i].iterations = iterations;
        reader_args[i].rds = &rds;
        reader_args[i].time = abs(rand() % 4) + 1;
    }

    // Initialize the arguments for the writers
    for (int i = 0; i < writers_no; i++) {
        writer_args[i].write_sem = &write_sem;
        writer_args[i].read_sem = &read_sem;
        writer_args[i].iterations = iterations;
        writer_args[i].rds = &rds;
        writer_args[i].time = abs(rand() % 4) + 1;
    }

    // Create the writers threads
    for (int i = 0; i < writers_no; i++) {
        mythreads_create(&writer_threads[i], (void (*)(void *)) writer, &writer_args[i]);
    }

    // Create the readers threads
    for (int i = 0; i < readers_no; i++) {
        mythreads_create(&reader_threads[i], (void (*)(void *)) reader, &reader_args[i]);
    }

    // Join and destroy the readers threads
    for (int i = 0; i < readers_no; i++) {
        mythreads_join(&reader_threads[i]);
        mythreads_destroy(&reader_threads[i]);
    }

    // Join and destoy the writers threads
    for (int i = 0; i < writers_no; i++) {
        mythreads_join(&writer_threads[i]);
        mythreads_destroy(&writer_threads[i]);
    }
    
    // Destroy the semaphores
    mythreads_sem_destroy(&read_sem);
    mythreads_sem_destroy(&write_sem);

    printf("Bye from Main\n");

    return 0;
}