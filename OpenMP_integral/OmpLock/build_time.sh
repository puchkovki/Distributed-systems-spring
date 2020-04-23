#!/bin/bash
FULLDIR=$(dirname "$0")
mpicc -Wall -Wextra -pedantic -O3 -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wcast-qual \
-Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fsanitize=undefined \
-fno-sanitize-recover=all -fstack-protector -fopenmp -o $FULLDIR/test $FULLDIR/time.c -lm || exit 1
for ((i = 1; i <= 4; ++i))
do
$FULLDIR/test $1 $i $2 >> res/data.txt
done
exit $?