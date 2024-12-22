import random
import struct
import subprocess

# Helper function to create a binary file with random integers
def create_binary_file(filename, num_integers):
    with open(filename, 'wb') as f:
        for _ in range(num_integers):
            # Write a random integer to the binary file
            f.write(struct.pack('i', random.randint(0, 10000)))

# Helper function to check if the file contains sorted integers
def is_file_sorted(filename):
    with open(filename, 'rb') as f:
        integers = []
        while chunk := f.read(4):  # Read 4 bytes (integer size)
            integers.append(struct.unpack('i', chunk)[0])
    return integers == sorted(integers)

# Main loop
import sys
import subprocess

def main():
    if len(sys.argv) < 3:
        print("Usage: heavytest.py <filename> <max_size>")
        sys.exit(1)

    filename = sys.argv[1]  # Take filename from the command-line argument
    max_size = int(sys.argv[2])  # Take the maximum size for the range from the command-line argument
    errors = []  # List to keep track of errors
    
    for size in range(1, max_size + 1):  # From 1 to max_size
        print(f"Processing file of size: {size}")  # Print current size

        # Step 1: Create a binary file with random integers
        create_binary_file(filename, size)
        
        # Step 2: Run the external merge sort executable
        try:
            with open("stdout.txt", "a") as out, open("stderr.txt", "a") as err:
                subprocess.run(["./ExtMergeSort", filename], check=True, stdout=out, stderr=err)
        except subprocess.CalledProcessError as e:
            errors.append(f"Error running ExtMergeSort with file of size {size}: {e}")
            continue
        
        # Step 3: Check if the file is sorted
        if is_file_sorted(filename):
            print(f"File of size {size} is sorted correctly.")
        else:
            errors.append(f"File of size {size} is not sorted correctly.")

    # Print the number of errors and the details
    num_errors = len(errors)
    if num_errors > 0:
        print(f"\nNumber of errors encountered: {num_errors}")
        print("Errors:")
        for error in errors:
            print(error)
    else:
        print("\nAll files sorted correctly without errors.")

if __name__ == "__main__":
    main()

