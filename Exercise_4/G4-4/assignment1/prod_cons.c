// Created by: Dimitris Voitsidis, Iordana Gaisidou, Stavros Stathoudakis

#include <fcntl.h>
#include <string.h>
#include "coroutines.h"

// checks if a system command encountered any error and kills the program informing the user about the error
void error_check(int r_value,char *str) {
    if(r_value == -1)
    {
        perror(str);
        exit(-1);
    }
}

// reads data from a file 
int my_read(int fd, void *buf, unsigned int goal) {
    unsigned int curr, count;
    
    count = 0;
    curr = 1;
    while(count != goal && curr != 0) {
        curr = read(fd, buf, goal - count);
        error_check(curr, "\nread");

        buf = (char*) buf + curr;
        count += curr;
    }

    return count;
}

// writes data to a file
void my_write(int fd, void *buf, unsigned int goal) {
    unsigned int curr, count;
    
    count = 0;
    curr = 0;
    while(count != goal)
    {
        curr = write(fd, buf, goal - count);
        error_check(curr, "\nwrite");

        buf = (char*) buf + curr;
        count += curr;
    }
}

void producer(void* args) {
    functionArgS* prodArgs = (functionArgS*) args;
    static int fd;

    printf("Hello from Producer!\n");

    // Open <filename>
    fd = open(prodArgs->filename, O_RDONLY);
    if(fd == -1) {
        fprintf(stderr, "Error in <%s> open.\n",prodArgs->filename);
        exit(-1);
    }
    
    do {
        // Write in storage
        *(prodArgs->storage_fullness) = my_read(fd, prodArgs->storage, prodArgs->storage_size);

        printf("Switching to Consumer from Producer!\n");

        mycoroutines_switchto(prodArgs->next);

    } while (*(prodArgs->storage_fullness) == prodArgs->storage_size);

    // Close the file
    if (close(fd) == -1) {
        perror("close");
        exit(-1);
    }

    printf("Bye from Producer!\n");
    
    return;
}

void consumer(void* args) {
    functionArgS* consArgs = (functionArgS*) args;
    static int fd;

    printf("Hello from Consumer!\n");

    // Open <filename>.copy
    fd = open(consArgs->filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if(fd == -1){
        fprintf(stderr, "Error in %s open.\n", consArgs->filename);
        exit(-1);
    }
    
    /************************ Read storage -> write it in <filename>.copy ************************/ 
    while (*(consArgs->storage_fullness) == consArgs->storage_size){
        my_write(fd, consArgs->storage, *(consArgs->storage_fullness));

        printf("Switching to Producer from Consumer!\n");

        mycoroutines_switchto(consArgs->next);
    } 

    my_write(fd, consArgs->storage, *(consArgs->storage_fullness));

    // Close the file
    if (close(fd) == -1) {
        perror("close");
        exit(-1);
    }

    printf("Bye from Consumer!\n");

    mycoroutines_switchto(consArgs->next);
}

int main (int argc, char* argv[]) {
    char *storage;
    co_t main_cor, prod_cor, cons_cor;
    functionArgS prod_args, cons_args;

    char *filename, *filenameCopy;   
    int storage_fullness = 0;
    int storage_size;
    int fileNameSize;
    int fd, fd_copy;
    char *initial_buffer,*final_buffer;
    int buffer_size = 1024;
    int read_initial_retval, read_final_retval;
    bool correct_res = true;

    // Checking the program's arguments -> ./test <filename_to_be_copied>
    if ( argc != 3 ) {
        printf("Wrong Arguments! Please give: ./test <storage size> <file name> \n");
        return 1;
    }
    
    printf("Hello from Main!\n");

    // Initialize the coroutines
    if (mycoroutines_init(&main_cor)) { return -1; }
    prod_cor = main_cor;
    cons_cor = main_cor;
   
    // Extracts the filename out of the input arguments
    storage_size = atoi(argv[1]);
    filename = argv[2];

    // Create the storage between producer and consumer
    storage = malloc(storage_size);
    if (storage == NULL) {
        perror("malloc");
        return -1;
    }
    
    // create a filename.copy
    fileNameSize = strlen(filename) + strlen(".copy") + 1;
    filenameCopy = malloc(fileNameSize);

    strcpy(filenameCopy,filename);
    strcat(filenameCopy,".copy");

    // Producer coroutine arguments
    prod_args.filename = filename;
    prod_args.storage = storage;
    prod_args.storage_fullness = &storage_fullness;
    prod_args.storage_size = storage_size;
    prod_args.next = &cons_cor;

    // Consumer coroutine arguments
    cons_args.filename = filenameCopy;
    cons_args.storage = storage;
    cons_args.storage_fullness = &storage_fullness;
    cons_args.storage_size = storage_size;
    cons_args.next = &prod_cor;

    // Producer Coroutine: Reads filename and writes it to storage
    mycoroutines_create(&prod_cor, producer, &prod_args);

    // Consumer Coroutine: Reads storage and writes it to filenameCopy
    mycoroutines_create(&cons_cor, consumer, &cons_args);

    printf("Switching to Producer from Main!\n");

    // Start the coroutines
    mycoroutines_switchto(&prod_cor);

    printf("\nDiff started from Main\n");

    // Compares the two files
    fd = open(filename, O_RDONLY);
    fd_copy = open(filenameCopy, O_RDONLY);

    if (fd == -1 || fd_copy == -1) {
        perror("open for comparison");
        return -1;
    }

    initial_buffer = malloc(buffer_size);
    final_buffer = malloc(buffer_size);

    if (initial_buffer == NULL || final_buffer == NULL) {
        perror("malloc for buffers for comparison");
        return -1;
    }

    do {
        read_initial_retval = my_read(fd, initial_buffer, 1024);
        read_final_retval = my_read(fd_copy, final_buffer, 1024);

        if (read_initial_retval != read_final_retval
            || strncmp(initial_buffer, final_buffer, buffer_size) != 0) {
        
            correct_res = false;
            break;
        }
    } while (read_initial_retval == buffer_size && read_final_retval == buffer_size);

    if (correct_res == true) {
        printf("\nThe files are the same!\n");
    } else {
        printf("\nThe files are different!\n");
    }

    // Clean up
    mycoroutines_destroy(&prod_cor);
    mycoroutines_destroy(&cons_cor);
    free(storage);
    free(filenameCopy);
    close(fd);
    close(fd_copy);

    printf("\nBye from Main!\n");

    return 0;
}