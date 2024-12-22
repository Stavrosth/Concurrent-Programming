## ASSIGNMENT 3.4: Train

## Description
This exercise simulates a train ride with the train as well as each of the passengers represented by a thread each. The train completes the designated journey only if N passengers are onboard, and only when all the passengers have disembarked the next passengers can start boarding.
The difference between this assignment and exercise 2.4, is that threads use monitors to achieve synchronization instead of binary semaphores.

## Compile and Test
The folder contains a Makefile with the following functionality:

**make** or **make all :** Shows all the targets available in the Makefile.

**make compile :** Compiles the code.

**make run N='<train_capacity>' :** Compiles and runs the program, using the train capacity that the user has entered.

**make cleanall :** Removes the executables.


## File Structure

**train.c:** Contains the implementation of the exercise mentioned above.

**train.h:** Includes all the libraries and the custom structs that are required for the correct function of the .c file. Also it contains the initialazations for all the functions that we use in order to not have a problem regarding the sequence that the functions are placed in .c file.

**Μakefile:** Implements the make commands described above.

**README.md:** This is the current file that you are reading :) .