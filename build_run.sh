#!/usr/bin/env bash

set -Exeuo pipefail

gcc -o huff_test ./huff_test.c -fsanitize=address -g3

# временно отключить выход из скрипта при получении не нулевого кода возврата
#(set +e; ./huff_test; set -e)

./huff_test

#files=(
    #"1.dot"
    #"2.dot"
    #"tree1.dot"
#)

readarray -d '' files < <(fd -0 ".*\.dot")

for file_in in "${files[@]}"; do
    file_out="${file_in%.dot}.png"
    dot -Tpng -o $file_out $file_in &
done

wait
