#!/bin/bash

clang++ -std=c++1y -O3 -DNDEBUG -pthread -Wall -Wextra "${@:2}" ./$1 -o /tmp/x.temp && /tmp/x.temp