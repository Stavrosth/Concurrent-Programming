import subprocess
import random
import sys
import re

def is_prime(n):

    if n <= 1:
        return False

    for i in range(2, int(n**0.5) + 1):

        if n % i == 0:
            return False

    return True

def count_unique_workers(log):
    worker_ids = set()  # Set to store unique worker IDs
    counter = 0  # Counter for unique workers

    # Regular expression to extract worker IDs from the text
    pattern = r"Hello from Worker (\d+)"

    for line in log:
        match = re.search(pattern, line)
        if match:
            worker_id = match.group(1)
            if worker_id not in worker_ids:
                worker_ids.add(worker_id)  # Add unique worker ID
                counter += 1  # Increment the counter for unique worker

    return counter

def run_test_program(num_runs, max_threads, thread_step, max_ints, max_value):
    num_threads = 0
    errors_in_num_all = 0;
    errors_in_workers = 0;

    while(num_threads < max_threads):
        num_threads += thread_step

        for _ in range(num_runs):
            errors_in_num = 0
            # Randomly choose the number of integers
            num_ints = random.randint(1, max_ints)
            
            # Generate a list of random integers
            integers = [random.randint(0, max_value) for _ in range(num_ints)]
            
            # Create the command with the random values
            command = ['./PrimalityTester', str(num_threads)] + list(map(str, integers))
            
            # Print the command to be executed (for debugging)
            print(f"Running command: {' '.join(command)}")
            
            # Execute the commandint(sys.argv[2]) 
            result = subprocess.run(command, capture_output=True, text=True)

            # ERROR CHECKING
            pattern = r"Worker \d+: (\d+) is (.+) and result is (\d)"
            # Check the output
            for line in result.stdout.splitlines():
                match = re.search(pattern, line)
                if match:
                    num = int(match.group(1))
                    is_prime_result = match.group(2) == "prime"
                    expected_result = is_prime(num)

                    # Check correctness
                    if is_prime_result != expected_result:
                        print(f"Error in processing {num}: Expected {'prime' if expected_result else 'not prime'} but got {'prime' if is_prime_result else 'not prime'}.")
                        errors_in_num += 1
                        errors_in_num_all += 1


            if(errors_in_num == 0):
                print("Right Results!!!")
            else:
                print("Wrong Results!!!")

            errors_in_workers = count_unique_workers(result.stdout)
            
            # Assuming `result` contains the output from a subprocess command
            with open("stdout.std", "a") as stdout_file, open("stderr.std", "a") as stderr_file:

                stdout_file.write(result.stdout)
                stderr_file.write(result.stderr)


    print(f"\nERRORS in primarity checking: {errors_in_num_all}\n")
    print(f"ERRORS in workers checking: {errors_in_workers}\n")


if len(sys.argv) != 6:
    print("Usage: python tester.py <number_of_times> <max_threads> <thread_step> <max_ints_num> <max_int_value>")
    sys.exit(1)

# Adjustable parameters
num_runs = int(sys.argv[1])     # Number of times to run the program
max_threads = int(sys.argv[2])  # Maximum number of threads
thread_step = int(sys.argv[3]) 
max_ints = int(sys.argv[4])     # Maximum number of integers to generate
max_value = int(sys.argv[5])   # Maximum value for the integers

# Run the tests
run_test_program(num_runs, max_threads, thread_step, max_ints, max_value)
