#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

typedef ucontext_t co_t;

int mycoroutines_init(co_t *main);
int mycoroutines_create(co_t *co, void (body)(void *), void *arg);
int mycoroutines_switchto(co_t *co);
int mycoroutines_destroy(co_t *co);

typedef struct {
    char *filename;
    void *storage;
    int *storage_fullness;
    int storage_size;
    co_t *next;
} functionArgS;