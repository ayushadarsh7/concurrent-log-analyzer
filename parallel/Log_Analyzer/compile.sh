#!/bin/bash
gcc -std=c11 -Wall -fopenmp -o analyzer analyzer.c -lpcre
./analyzer
