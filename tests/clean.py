#!/usr/bin/env python3

import sys
import os

if len(sys.argv) < 2:
    print("Usage: ./clean.py file")

raw_file = open(sys.argv[1], 'r')
for line in raw_file.readlines():
    puzzle = line.split(" ")
    level = puzzle[3].strip().split(".")
    path = "se" + level[0] + "_" + level[1]
    if not os.path.exists(path):
        os.mkdir(path)

    filename = path + "-" + puzzle[0]
    with open(os.path.join(path, filename), 'w') as out_file:
        for i in range(0,81,9):
            out_file.write(puzzle[1][i:i + 9] + "\n")
