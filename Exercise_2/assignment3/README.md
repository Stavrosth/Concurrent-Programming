## Assignment 2.3: Narrow Bridge

## Desciption
This exercise simulates a narrow bridge on which cars can pass to the other side. The cars are represented by threads that only communicate with eachother using the binary semaphores of our own custom library, without the use of a middleman thread. 

The implementation of this program has the following requirements :

1) Because the bridge is very narrow, cars of only one direction can pass to the other side.

2) There must be no more than N (assigned by the user) cars on the bridge

3) One vehicle must not wait forever for it's turn to pass to the other side


## Compile and Test
The folder contains a Makefile with the following functionality:

**make** or **make all :** Shows all the targets available in the Makefile.

**make compile :** Compiles the code.

**make run N='<max_number_of_cars_on_bridge>' :** Compiles and runs the program, using the argument of the user as the maximum number of cars allowed on the bridge at the same time.


## File Structure

**bridge.c:** Contains the implementation of the exercise mentioned above.

**Μakefile:** Implements the make commands described above.

**README.md:** This is the current file that you are reading :) .