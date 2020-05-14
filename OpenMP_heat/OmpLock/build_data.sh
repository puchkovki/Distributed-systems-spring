#!/bin/bash
echo "Compilation"
FULLDIR=$(dirname "$0")
gcc -Wall -Wextra -pedantic -O3 -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wcast-qual \
-Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fsanitize=undefined \
-fno-sanitize-recover=all -fstack-protector -fopenmp -o $FULLDIR/test $FULLDIR/solution.c -lm || exit 1

echo "Making data file with times"
for ((i = 1; i <= 16; i*=2))
do
    echo -n > $FULLDIR/res/$i.txt
    $FULLDIR/test 0.$i 100 1 >> $FULLDIR/res/$i.txt
done

echo "Plotting metrics"
gnuplot <<< "set terminal png size 1024, 720; \
            set xlabel 'x'; \
            set ylabel 'Temperature'; \
            set xrange [0:0.9]; \
            set yrange [1:5]; \
            set output 'res/data.png'; \
            set title 'T(x)'; \
            plot 'res/1.txt' u 1:2 title 'Time = 0.1' w lines, 'res/2.txt' u 1:2 title 'Time = 0.2' w linespoints, 'res/4.txt' u 1:2 title 'Time = 0.4' w linespoints, 'res/8.txt' u 1:2 title 'Time = 0.8' w linespoints, 'res/16.txt' u 1:2 title 'Time = 1.6' w linespoints; "
echo "Plot image stored at res"
exit $?