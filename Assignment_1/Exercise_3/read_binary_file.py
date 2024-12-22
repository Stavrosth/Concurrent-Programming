import struct
import sys

# Check if the user provided the input file name as an argument
if len(sys.argv) < 2:
    print("Usage: python script.py <input_file>")
    sys.exit(1)

# Get the input file name from the command line arguments
input_file = sys.argv[1]

# Open the binary file for reading
with open(input_file, 'rb') as file:
    # Read and print integers from the file
    while True:
        binary_data = file.read(4)  # Read 4 bytes (size of an integer)
        if not binary_data:
            break
        random_integer = struct.unpack('i', binary_data)[0]  # Convert from binary to integer
        print(random_integer, end=' ')
print('')
