import sys
import subprocess
import filecmp
import shutil
import os

# Ensure the number of iterations is passed as a command-line argument
if len(sys.argv) < 3:
    print("Usage: python script.py <file_name> <num_iterations")
    sys.exit(1)

# Arguments from terminal -> <file_name> , <
file_name = sys.argv[1]    # Get the input filename from the argument
num_iterations = int(sys.argv[2])

# STDOUT || STDERR
stdout_file = "stdout.std"
stderr_file = "stderr.std"

# Error tracking
errors_in_copy1 = 0
errors_in_copy2 = 0

# Remove any existing output/error files
for log_file in [stdout_file, stderr_file]:
    if os.path.exists(log_file):
        os.remove(log_file)

# Iterate and run the program
for iteration in range(num_iterations):
    print(f"Running iteration {iteration + 1}...")

    # Run the external program and redirect output/error to files
    with open(stdout_file, 'a') as out, open(stderr_file, 'a') as err:
        subprocess.run(['./test', file_name], stdout=out, stderr=err)

    # Define file copies for comparison
    copy1 = f"{file_name}.copy"
    copy2 = f"{file_name}.copy2"

    # Compare the original file with copy1
    if not filecmp.cmp(file_name, copy1, shallow=False):
        print("Difference detected in copy1!")
        errors_in_copy1 += 1
        shutil.copyfile(copy1, f"{copy1}_{errors_in_copy1}.error")

    # Compare copy1 with copy2
    if not filecmp.cmp(file_name, copy2, shallow=False):
        print("Difference detected in copy2!")
        errors_in_copy2 += 1
        shutil.copyfile(copy2, f"{copy2}_{errors_in_copy2}.error")

# Final summary
print(f"\n\nTotal errors in copy1: {errors_in_copy1}, Total errors in copy2: {errors_in_copy2}")
