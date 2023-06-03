#!/bin/bash
gcc -g -std=c99 ./*.h ./*.c -o ./tt
./tt ./sample.tny # 正例
# ./tt ./syn.tny # 反例