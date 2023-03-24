#!/bin/bash
exe=./109062131_hw1

min_support=0.2
input_filename=$1
output_filename=$input_filename.out

make clean && make -j
rm -f outputs/$output_filename
time $exe $min_support testcases/$input_filename outputs/$output_filename
# time memusage -T $exe $min_support testcases/$input_filename outputs/$output_filename
timeout 10 python scripts/diff.py outputs/$output_filename testcases/$output_filename