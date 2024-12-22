
# ASSIGNMENT 4.2: Custom user-level-threads and Custom Semaphores

## Description 

This exercise focuses on creating a library that creates Custom user-level-threads and custom binary Semaphores which the developer can then use in other C programs.
Because The threads are user-level, the system views them as a single process so we had to implement our own scheduler which decides when the user-level thread should give up the CPU. This is done through a timer of 1 microsecond that is reseted everytime that the process changes hands.
**NOTE**: The user should **NOT** use the mythreads_join function after using the mythreads_destroy function.


## Library Functions

The library includes the following functions:

- **Mythreads init**: Initializes the environment for the threads. 
- **Mythreads create**: Creates a new thread.
- **Mythreads yield**: Voluntarily gives up the CPU to the next thread if available.
- **Mythreads sleep**: Sleeps for a specified amount of time.
- **Mythreads join**: Waits for the specified thread to return.
- **Mythreads destroy**: Destroyes the thread so no-one can access it after.
- **Mythreads self**: Returns the calling thread. Used for debugging purposes.
- **Mythreads semaphore create**: Creates a new binary semaphore.
- **Mythreads semaphore down**: Decreases the semaphore value or give up the cpu if value is already 0.
- **Mythreads semaphore up**: Increases the semaphore value if not already 1.
- **Mythreads semaphore destroy**: Destroys the semaphore.
- **Mythreads semaphore getval**: Returns the value of the semaphore. Used for debugging purposes.

## Compile and Test
The folder contains a Makefile with the following functionality:

**make** or **make all :** Shows all the targets available in the Makefile.

**make compile :** Compiles the code.

**make run:** Compiles and runs the test program 

**make cleanall :** Removes the executable as well as all the object files.


## File Structure

**mythreads.c:** Contains the implementation of the library functions.

**mythreads.h:** Header file for the library.

**simpletest.c** Represents the test of the functions of the library.

**Makefile:** Implements the make commands mentioned above.

**README.md:** This is the current file that you are reading :) .