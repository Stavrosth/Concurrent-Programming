import sys
import random
import struct

# Check if the correct number of arguments are provided
if len(sys.argv) != 3:
    print("Usage: python program.py <filename> <number_of_integers>")
    sys.exit(1)

# Get the filename and number of integers from command-line arguments
filename = sys.argv[1]
n = int(sys.argv[2])

# Open the binary file for writing
with open(filename, 'wb') as file:
    # Generate random integers and write them to the file
    for i in range(n):
        #random_integer = n - i
        random_integer = random.randint(1, 100)  # Random integer between 1 and 100
        binary_data = struct.pack('i', random_integer)  # Convert to binary format
        file.write(binary_data)  # Write the binary data to the file
