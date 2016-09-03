#!/bin/bash

./bfdfs-load -x '*.make' -x '*.cmake' bfdfs-blob.o CMakeFiles
g++ -std=c++11 -o test test.cpp bfdfs-blob.o
