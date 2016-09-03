#!/bin/bash

./bfdfs-load bfdfs-blob.o CMakeFiles
g++ -std=c++11 -o test test.cpp bfdfs-blob.o
