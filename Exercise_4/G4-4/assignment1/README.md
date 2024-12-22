# EXERCISE 4.1: FIFO pipes


## Description

This exercise focuses on creating an independent Coroutines Library for Concurrent code execution, using the getcontext(), makecontext(), swapcontext() functions fro the ucontext.h library. The user has to manually change through coroutines.
In this exercise we created a test that is intended to test the functionality of the library. For this purpose we created two coroutines that exchange a file's contents through a buffer/storage that the user can decide it's size, in order to create a copy of the initial file.

## Library Functions

The library includes the following functions:

- **Mycoroutines Init**: Initializes the main coroutine
- **Mycoroutines Create**: Creates a new coroutine
- **Mycoroutines Switchto**: Switches to another coroutine
- **Mycoroutines Destroy**: Destroyes a coroutine


## Compile and Test
The folder contains a Makefile with the following functionality:

**make** or **make all :** Shows all the targets available in the Makefile.

**make compile :** Compiles the code.

**make run s=\<storage_size> f=\<filename> :** Compiles and runs the program, using the specified file as input.

**make clean :** Removes the compiled output files (.bin, .o), along with any .std and .error files.

**make clean.copy :** Deletes any .copy files created by running the program.

**make cleanall :** Combines make clean and make clean.copy to remove all generated files.

**make diff f=\<filename> :** Compares the given file with its generated copies (\<filename> and \<filename>.copy).


## File Structure

**coroutines.c:** Contains the implementation of the library functions.

**coroutines.h:** Header file for the library.

**pro_cons.c:** Contains the test program as described in the first section.

**Makefile:** Implements the make commands mentioned above.

**\<filename>.txt:** The input file(s) for the executable. The \<filename> mentioned above.

**README.md:** This is the current file that you are reading :) .
