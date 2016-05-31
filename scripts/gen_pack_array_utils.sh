#!/bin/sh

python $(dirname $0)/gen_bit_writer.py 3 >src/utils/pack_int3_array.h
python $(dirname $0)/gen_bit_writer.py 5 >src/utils/pack_int5_array.h
