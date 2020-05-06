#!/bin/bash
echo "Compilation"
FULLDIR=$(dirname "$0")
mpicc -Wall -Wextra -pedantic -O3 -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wcast-qual \
-Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fsanitize=undefined \
-fno-sanitize-recover=all -fstack-protector -o $FULLDIR/test $FULLDIR/time.c -lm || exit 1

echo "Making res directory if necessary"
mkdir -p $FULLDIR/res

echo "Cleaning up res directory"
echo -n > $FULLDIR/res/data.txt

echo "Making data file with times"
for ((i = 1; i <= 28; ++i))
do
mpirun -np $i $FULLDIR/test $1 $2 >> $FULLDIR/res/data.txt
done

echo "Compilation"
FULLDIR=$(dirname "$0")
gcc -Wall -Wextra -pedantic -O3 -o $FULLDIR/test $FULLDIR/data.c || exit 1

echo "Making auxiliary files acceleration.txt and efficiency.txt"
$FULLDIR/test

echo "Plotting metrics"
gnuplot <<< "set terminal png size 1024, 720; \
            set xlabel 'Number of processes'; \
            set ylabel 'Acceleration'; \
            set xrange [1:28]; \
            set output 'res/acceleration.png'; \
            plot 'res/acceleration.txt' with lines; \
            set terminal png size 1024, 720; \
            set xlabel 'Number of processes'; \
            set ylabel 'Efficiency'; \
            set xrange [1:28]; \
            set output 'res/efficiency.png'; \
            plot 'res/efficiency.txt' with lines; "
echo "Plot image stored at res"
exit $?
# Для нормальных результатов следует брать значения порядка T =  0.000001 M = 50000