#!/bin/bash
gcc -std=c11 -Wall -fopenmp -o parser log_parser.c
./parser
