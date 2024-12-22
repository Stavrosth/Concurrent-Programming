# ASSIGNMENT 4.3: Readers-Writers Problems

## Description
This exercise is intended to test the mythreads.h Library that we implemented on the previous assignment.
The classic reades/writers problem has 3 requirements:
1) Simultaneous Reading has to be possible
2) Reading and Writing should not happen at the same time
3) Simultaneous Writing is prohibited

The problem is solved using the custom user-level-threads and semaphores that we implemented in the previous assignment


## Compile and Test
The folder contains a Makefile with the following functionality:

**make** or **make all :** Shows all the targets available in the Makefile.

**make compile :** Compiles the code.

**make run ARGS='\<readers> \<writers> \<iterations>' :** Compiles and runs the program, using the specified number of readers, writers and iterations for each thread.

**make cleanall :** Removes the executable as well as all the outputs that were created by the python script.


## File Structure

**readers_writers.c:** Contains the implementation of the prime check using threads.

**Μakefile:** Implements the make commands described above.

**README.md:** This is the current file that you are reading :) .