import sys
error = 0

# Check if a filename is passed as a command-line argument
if len(sys.argv) < 2:
    print("Error: Please provide a filename as a command-line argument.")
    sys.exit(1)

# The first argument after the script name is the filename
file_path = sys.argv[1]


# Read the file
with open(file_path, 'r') as file:
    # Read the integers from the file and convert them to a list of integers
    numbers = list(map(int, file.read().split()))

# Iterate over the list and check if the next number is smaller than the current one
for i in range(len(numbers) - 1):
    current = numbers[i]
    next_num = numbers[i + 1]
    if next_num < current:  # Print only if the next number is smaller than the current
        print(f"{next_num} is smaller than {current}")
        error += 1

if error == 0:
    print(f"The file is sorted")