#!/usr/bin/env bash

set -Exeuo pipefail

gcc -o huff_test ./huff_test.c -fsanitize=address -g3

#files=(
    #"1.dot"
    #"2.dot"
    #"tree1.dot"
#)

readarray -d '' files < <(fd -0 ".*\.dot")

for file_in in "${files[@]}"; do
    #file_out="${file_in%.dot}.png"
    file_out="${file_in%.dot}.png"
    dot -Tpng -o $file_out $file_in
done
