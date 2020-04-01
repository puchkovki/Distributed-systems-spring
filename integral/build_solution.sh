#!/bin/bash
FULLDIR=$(dirname "$0")
mpicc -Wall -Wextra -pedantic -O3 -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wcast-qual \
-Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fsanitize=undefined \
-fno-sanitize-recover=all -fstack-protector -o $FULLDIR/test $FULLDIR/solution.c -lm || exit 1
for ((i = 1; i <= 28; ++i))
do
mpirun -np $i $FULLDIR/test $1 >> res/integral.txt
done
exit $?