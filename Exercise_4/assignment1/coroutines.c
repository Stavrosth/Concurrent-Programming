#include "coroutines.h"

static co_t *main_co, *curr_co;

// Initialize the main coroutine
int mycoroutines_init(co_t *main) {
    // Error checking
    if(main == NULL) {
        fprintf(stderr, "mycoroutines_init: main is NULL\n");
        return -1;
    }

    // get context of main and return success
    if (getcontext(main)) {
        perror("mycoroutines_init: getcontext");
        return -1;
    }

    main_co = main;
    curr_co = main_co;
    
    return 0;
}

// Create a new coroutine
int mycoroutines_create(co_t *co, void (body)(void *), void *arg) {
    char *co_stack;

    // Error checking
    if (co == NULL) {
        fprintf(stderr, "mycoroutines_create: new_co is NULL\n");
        return -1;
    }
    if(body == NULL) {
        fprintf(stderr, "mycoroutines_create: body is NULL\n");
        return -1;
    }

    // Assigns stack to the new coroutine
    co_stack = malloc(SIGSTKSZ);
    if (co_stack == NULL) {
        perror("mycoroutines_create: malloc for stack");
        exit(-1);
    }
    co->uc_stack.ss_sp = co_stack;
    co->uc_stack.ss_size = SIGSTKSZ;
    
    // Assigns the coroutine to return to to the new coroutine
    co->uc_link = main_co;

    // Initialize the context of the new coroutine
    if (arg == NULL) {
        makecontext(co, (void(*)(void)) body, 0);
    } else {
        makecontext(co, (void(*)(void)) body, 1, arg);
    }

    return 0;
}

// Switch to another coroutine
int mycoroutines_switchto(co_t *co) {
    co_t *tmp_co;

    // Error checking
    if (co == NULL) {
        fprintf(stderr, "mycoroutines_switch: to_co is NULL\n");
        return -1;
    }
    
    // Save the current coroutine
    tmp_co = curr_co;
    curr_co = co;

    // Switch to the new coroutine
    if (swapcontext(tmp_co, co) != 0) {
        perror("mycoroutines_switch: setcontext");
        return -1;
    }

    return 0;
}

// Destroy a coroutine
int mycoroutines_destroy(co_t *co) {
    // Error checking
    if(co == NULL) {
        fprintf(stderr, "mycoroutines_destroy: co is NULL\n");
        return -1;
    }

    // Free the stack of the coroutine and return success
    free(co->uc_stack.ss_sp);

    return 0;
}