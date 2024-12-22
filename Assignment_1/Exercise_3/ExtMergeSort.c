// Created by: Dimitris Voitsidis, Iordana Gaisidou, Stavros Stathoudakis
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct mergeArg {
    char* filename;
    int left;
    int right;
    bool returned;
}mergeArgS;

// reads data from a file 
int my_read(int fd, void *buf, unsigned int goal) {
    unsigned int curr, count;
    
    count = 0;
    curr = 1;
    while(count != goal && curr != 0) {
        curr = read(fd, buf, goal - count);
        if(curr==-1){
            perror("Error in read");
            fprintf(stderr, "curr: %d\n", curr);
        }

        buf = (char*) buf + curr;
        count += curr;
    }

    return count;
}

// writes data to a file
int my_write(int fd, void *buf, unsigned int goal) {
    unsigned int curr, count;
    
    count = 0;
    curr = 1;
    while(count != goal && curr != 0) {
        curr = write(fd, buf, goal - count);
        if(curr==-1){
            perror("Error in write");
            fprintf(stderr, "curr: %d\n", curr);
        }

        buf = (char*) buf + curr;
        count += curr;
    }

    return count;
}

// This is from
// https://www.geeksforgeeks.org/c-program-for-merge-sort/
// Merges two subarrays of arr[].
// First subarray is arr[left..mid]
// Second subarray is arr[mid+1..right]
void intMerge(int arr[], int left, int mid, int right) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    // Create temporary arrays
    int leftArr[n1], rightArr[n2];

    // Copy data to temporary arrays
    for (i = 0; i < n1; i++)
        leftArr[i] = arr[left + i];
    for (j = 0; j < n2; j++)
        rightArr[j] = arr[mid + 1 + j];

    // Merge the temporary arrays back into arr[left..right]
    i = 0;
    j = 0;
    k = left;
    while (i < n1 && j < n2) {
        if (leftArr[i] <= rightArr[j]) {
            arr[k] = leftArr[i];
            i++;
        }
        else {
            arr[k] = rightArr[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of leftArr[], if any
    while (i < n1) {
        arr[k] = leftArr[i];
        i++;
        k++;
    }

    // Copy the remaining elements of rightArr[], if any
    while (j < n2) {
        arr[k] = rightArr[j];
        j++;
        k++;
    }
}

// The subarray to be sorted is in the index range [left-right]
void intMergeSort(int arr[], int left, int right) {
    if (left < right) {
      
        // Calculate the midpoint
        int mid = left + (right - left) / 2;

        // Sort first and second halves
        intMergeSort(arr, left, mid);
        intMergeSort(arr, mid + 1, right);

        // Merge the sorted halves
        intMerge(arr, left, mid, right);
    }
}

// This is derrived from
// https://www.geeksforgeeks.org/c-program-for-merge-sort/
// Merges two subparts of a file
void extMerge(char* filename, int left, int mid, int right) {
    int i, j, k;
    int size1, size2;
    char tmp[12];
    int leftArr, rightArr;
    int fd1, fd2, fdtemp;

    // Calculate the sizes of the two subarrays
    size1 = (mid - left)/sizeof(int) + 1;
    size2 = (right - mid)/sizeof(int);

    // Convert integer to string in order to create unique file names
    sprintf(tmp, ".%d", mid);

    // Open the files
    fd1 = open(filename, O_RDWR, 0644);
    if (fd1 == -1) {
        perror("Error in open");
        fprintf(stderr, "Error in <%s> open.\n", filename);
        exit(-1);
    }
    fd2 = open(filename, O_RDWR, 0644);
    if (fd2 == -1) {
        perror("Error in open");
        fprintf(stderr, "Error in <%s> open.\n", filename);
        exit(-1);
    }
    fdtemp = open(tmp, O_RDWR | O_CREAT, 0644);
    if (fdtemp == -1) {
        perror("Error in open");
        fprintf(stderr, "Error in <%s> open.\n", tmp);
        exit(-1);
    }

    // Set the file descriptors to the correct positions
    lseek(fd1, left, SEEK_SET);
    lseek(fd2, mid + sizeof(int), SEEK_SET);
    
    // Merge the temporary arrays back into arr[left..right]
    i = 0;
    j = 0;
    my_read(fd1, &leftArr, sizeof(int));
    my_read(fd2, &rightArr, sizeof(int));
    while (i < size1 && j < size2) {
        if (leftArr <= rightArr) {
            my_write(fdtemp, &leftArr, sizeof(int));
            my_read(fd1, &leftArr, sizeof(int));
            i++;
        }
        else {
            my_write(fdtemp, &rightArr, sizeof(int));
            my_read(fd2, &rightArr, sizeof(int));
            j++;
        }
    }

    // Copy the remaining elements of leftArr[], if any
    while (i < size1) {
        my_write(fdtemp, &leftArr, sizeof(int));
        my_read(fd1, &leftArr, sizeof(int));
        i++;
    }

    // Copy the remaining elements of rightArr[], if any
    while (j < size2) {
        my_write(fdtemp, &rightArr, sizeof(int));
        my_read(fd2, &rightArr, sizeof(int));
        j++;
    }

    // Reset the file descriptors to the beginning of the file
    lseek(fd1, left, SEEK_SET);
    lseek(fdtemp, 0, SEEK_SET);

    // Copy the merged array back to the original file
    k = left/sizeof(int);
    while (k != (right)/sizeof(int) + 1) {
        my_read(fdtemp, &leftArr, sizeof(int));
        my_write(fd1, &leftArr, sizeof(int));
        k++;
    }

    // Close the files
    close(fd1);
    close(fd2);
    close(fdtemp);

    // Remove the temporary file
    remove(tmp);

    return;
}

// The subarray to be sorted is in the index range [left-right]
void* extMergeSort(void *Margs) {
    mergeArgS* args = (mergeArgS*) Margs;
    mergeArgS args1, args2;
    pthread_t t1, t2;
    int* arr;
    int left, right, mid;
    int error, mod;
    int fd;
    int size;

    printf("Hello from Thread!\n");

    // Calculate the left, right, midpoint
    left = args->left;
    right = args->right;
    mid = left + (right - left) / 2;
    mod = mid % sizeof(int);
    mid = mid - mod;

    // If the size of the array is less than 64, sort it in memory
    size = (right - left)/sizeof(int) + 1;
    if(size <= 64) {
        // Opens the file
        fd = open(args->filename, O_RDWR, 0644);
        if (fd == -1) {
            perror("Error in open");
            fprintf(stderr, "Error in <%s> open.\n", args->filename);
            exit(-1);
        }

        // Set the file descriptor to the correct position
        lseek(fd, left, SEEK_SET);
        if (fd == -1) {
            perror("Error in lseek");
            exit(-1);
        }

        // Allocates memory for the array
        arr = malloc(size * sizeof(int));
        if (arr == NULL) {
            perror("Error in malloc");
            exit(-1);
        }
        
        // Read the file
        my_read(fd, arr, (right - left) + sizeof(int));

        // Calls the Internal Merge Sort
        intMergeSort(arr, 0, size-1);

        // Reset the file descriptor to the beginning of the file
        error = lseek(fd, -(right - left + sizeof(int)), SEEK_CUR);
        if (error == -1) {
            perror("Error in lseek");
            exit(-1);
        }

        // Write the file
        my_write(fd, arr, (right - left) + sizeof(int));

        // Free the memory
        free(arr);

        // Close the file
        close(fd);
        
        printf("Bye from Thread!\n");

        args->returned = true;

        return NULL;
    }

    // Sort first half
    args1.filename = args->filename;
    args1.right = mid;
    args1.left = left;
    args1.returned = false;
    if (pthread_create(&t1, NULL, extMergeSort, &args1)) {
        perror("Error in pthread_create");
        fprintf(stderr, "ERROR: pthread_create %ld\n", t1);
        exit (-1);
    }

    // Sort second half
    args2.filename = args->filename;
    args2.left = mid + sizeof(int);
    args2.right = right;
    args2.returned = false;
    if ( pthread_create(&t2, NULL, extMergeSort, &args2)) {
        perror("Error in pthread_create");
        fprintf(stderr, "ERROR: pthread_create %ld\n", t2);
        exit (-1);
    }

    // Wait for the two halves to be sorted
    while(args1.returned == false || args2.returned == false) {/*wait*/}

    // Merge the sorted halves
    extMerge(args->filename, left, mid, right);

    printf("Bye from Thread!\n");

    args->returned = true;

    return NULL;
}

int main(int argc, char* argv[]) {   
    int fd, array_size;
    mergeArgS Margs;
    pthread_t t;
    int res;

    printf("Hello from main\n");

    if(argc != 2){
        printf("Wrong Arguments! Please give: ./ExtMergeSort <filename>\n");
        return 1;
    }

    // Set the filename
    Margs.filename = argv[1];

    // Open the file
    fd = open(Margs.filename, O_RDWR, 0644);
    if (fd == -1) {
        perror("Error in open");
        fprintf(stderr, "Error in <%s> open.\n", Margs.filename);
        return 1;
    }

    // Get the size of the file
    array_size = lseek(fd, 0, SEEK_END);
    if (array_size == -1) {
        perror("Error getting file size");
        return 1;
    }

    // Close the file
    close(fd);

    // Set the left and right pointers
    Margs.left = 0;
    Margs.right = array_size - sizeof(int);
    Margs.returned = false;

    // Call the first thread
    res = pthread_create(&t, NULL, extMergeSort, &Margs);
    if (res) {
        perror("Error in pthread_create");
        fprintf(stderr, "ERROR: pthread_create %ld\n", t);
        exit (-1);
    }

    while(Margs.returned == false) {/*wait*/}

    printf("Bye from Main!\n");

    return 0;
}

