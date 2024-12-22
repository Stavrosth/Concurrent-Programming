## ASSIGNMENT 2.4: Train

## Description
This exercise simulates a train ride with the train as well as each of the passengers represented by a thread each. The train completes the designated journey only if N passengers are onboard, and only when all the passengers have disembarked the next passengers can start boarding. The communication between the train and the passengers is done by using binary semaphores of our own custom library.

All the threds and semaphores used are custom, from the library mythreads.h


## Compile and Test
The folder contains a Makefile with the following functionality:

**make** or **make all :** Shows all the targets available in the Makefile.

**make compile :** Compiles the code.

**make run N='<train_capacity>' :** Compiles and runs the program, using the train capacity that the user has entered.


## File Structure

**train.c:** Contains the implementation of the exercise mentioned above.

**Μakefile:** Implements the make commands described above.

**README.md:** This is the current file that you are reading :) .