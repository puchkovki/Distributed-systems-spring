#!/bin/bash
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