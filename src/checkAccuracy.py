import subprocess
import time
import os
import csv

MAX_ITER = 50
MAX_TRAIN = 100

expected_output = ["78 - N", "101 - e", "120 - x", "116 - t", "32 -  ", "115 - s", "116 - t", "101 - e", "112 - p", "58 - :", "32 -  ", "103 - g", "101 - e", "116 - t", "32 -  ", "116 - t", "104 - h", "105 - i", "115 - s", "32 -  ", "109 - m", "111 - o", "100 - d", "117 - u", "108 - l", "101 - e", "32 -  ", "105 - i", "110 - n", "116 - t", "111 - o", "32 -  ", "116 - t", "104 - h", "101 - e", "32 -  ", "109 - m", "97 - a", "105 - i", "110 - n", "108 - l", "105 - i", "110 - n", "101 - e", "32 -  ", "107 - k", "101 - e", "114 - r", "110 - n", "101 - e", "108 - l"]

program = './attack/attack'

for train in range(10, MAX_TRAIN + 1, 10):

    # Update the number of training iterations
    cmd = 'sed -i "s/#define TRAIN_ITER .*/#define TRAIN_ITER "' + str(train) + '"/" attack/attack.c'
    os.system(cmd)
    os.system("make -C attack")

    matches = []
    times = []

    for j in range(MAX_ITER):
        print("Training iterations: " + str(train) + " j: " + str(j))
        start_time = time.time()

        output = subprocess.Popen(
            [program],
            stdout=subprocess.PIPE,
            universal_newlines=True
        )

        output.wait()
        output_lines = output.stdout.read().splitlines()

        end_time = time.time()
        execution_time = end_time - start_time

        match = sum([1 for actual_line, expected_line in zip(output_lines, expected_output) if actual_line == expected_line])
        matches.extend([match*100/51])
        times.extend([execution_time])

    with open("RESULTS_" + str(train) + ".csv", 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(times)
        writer.writerow(matches)
