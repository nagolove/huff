#!/usr/bin/env bash

# Печатать выполняемую команду оболочки
set -x
# Прерывать выполнение сценария если код возврата команды не обработан условием и не равен 0
set -e

gcc -o huff_test ./huff_test.c -fsanitize=address
