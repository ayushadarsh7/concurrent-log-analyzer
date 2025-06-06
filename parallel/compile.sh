#!/bin/bash
gcc -std=c11 -Wall  -fopenmp -o log_parser log_parser.c
./log_parser
