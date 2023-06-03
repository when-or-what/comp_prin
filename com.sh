#!/bin/bash
gcc -g -std=c99 ./*.h ./*.c -o ./tt
# ./tt ./func.tny
# gdb --args ./tt ./func.tny
# ./tt ./sample.tny
./tt ./syn.tny