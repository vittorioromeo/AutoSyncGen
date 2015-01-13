#!/bin/bash

clang++ -std=c++1y -lsfml-system -lsfml-network -O3 -DNDEBUG -pthread -Wall -Wextra "${@:2}" ./$1 -o /tmp/x.temp && /tmp/x.temp