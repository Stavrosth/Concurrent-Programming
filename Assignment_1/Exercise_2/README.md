
# EXERCISE 1.2: Primality Tester


## Description
This exercise checks if a series of numbers are prime by creating N threads assigned by the user in order to reduce running time.

## Worker Thread use
The program first creates N workers and then it assigns a number to calculate its primality to the first worker that it finds available. The priority is for all the workers to be assigned a job (if possible). After a worker is done with its job, it raises a flag in order to let the main function to know so it can assign it a new job.

## Compile and Test
The folder contains a Makefile with the following functionality:

**make** or **make all :** Shows all the targets available in the Makefile.

**make compile :** Compiles the code.

**make run ARGS='\<number_of_workers> \<int1> \<int2> ... \<intN>' :** Compiles and runs the program, using the specified number of workers and decides for every input number if it is prime or not.

**make heavytest ARGS='<number_of_times> <max_threads> <thread_step> <max_ints_num> <max_int_value>' :** Compiles the program and runs a heavy test by calling a Python script that runs the code. Explanation of input arguments:

    -number_of_times: defines how many times the c program will run for each number of threads
    
    -max_threads: the maximum number of threads that the c program is called with
    
    -thread_step: the number that the threads are increasing for everytime that the c program is being called (starting value is thread_step)
    
    -max_ints_num: defines the maximum number of input integers the C program can have for every iteration. (The python script randomly chooses how many numbers it will put as an input in the C program for every iteration)
    
    -max_int_value: defines the maximum value that every integer can have when the C program is called upon. (The python script chooses every integer's value randomly)

**make cleanall :** Removes the executable as well as all the outputs that were created by the python script.


## File Structure
![Project diagram](filestructure.svg)

**PrimalityTester.c:** Contains the implementation of the prime check using threads.

**Μakefile:** Implements the make commands described above.

**heavytest.py:** Python script to run the program multiple times and check the correctness of the output.

**README.md:** This is the current file that you are reading :) .

**filestructure.svg:** The photo above.