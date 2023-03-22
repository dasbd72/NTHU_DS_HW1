#!/bin/bash
exe=./109062131_hw1

min_support=0.2
input_filename=sample
output_filename=sample.out

make
$exe $min_support testcases/$input_filename outputs/$output_filename
diff outputs/$output_filename testcases/$output_filename