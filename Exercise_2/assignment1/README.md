
# ASSIGNMENT 2.1: Binary Semaphrores


## Description

This exercise focuses on creating a library that handles binary semaphores which the developer can then use in other C programs.
In this assignment we created a test that is intended to test the functionality of the library. For this purpose we created two threads that use all the functions of the library.

## Library Functions

The library includes the following functions:

- **Mysem Init**: Initialises a binary semaphore.
- **Mysem down**: Decreases the value of the semaphore from 1 to 0.
- **Mysem up**: Increases the value of the semaphore from 0 to 1.
- **Mysem destroy**: Destroys a semaphore.
- **Mysem error**: Prints the error value of a function if it does not succeed.


## Compile and Test
The folder contains a Makefile with the following functionality:

**make** or **make all :** Shows all the targets available in the Makefile.

**make compile :** Compiles the code.

**make run:** Compiles and runs the test program 

**make cleanall :** Removes the executable as well as all the object files.


## File Structure

**binarySemaphores.c:** Contains the implementation of the library functions.

**binarySemaphores.h:** Header file for the library.

**test.c** Represents the test of the functions of the library.

**Makefile:** Implements the make commands mentioned above.

**README.md:** This is the current file that you are reading :) .
