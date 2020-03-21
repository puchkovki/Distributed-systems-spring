# **Определенный интеграл**

## **Задача**

Вычисление определенного интеграла, используя метод трапеции. Параметры программы передаются через аргументы командной строки (число разбиений отрезка N).

Результат работы программы — значение интеграла с точностью до 6 знаков после запятой (тип `double`). Распараллелить программу, используя среду MPI. 

Требования:
1. Обязательно использовать вызовы ```MPI_Send/MPI_Recv```.
2. Построить графики ускорения и эффективности для числа процессов от 1 до 28.

## **Решение**

Для распараллеливания я делю отрезок интегрирования на интервалы, их количество равно количеству процессов, запущенных системой. Интервалы не пересекаются, и значения, вычисленные каждым процессом, передаются с помощью ```MPI_Send``` 0-ому процессу. С помощью ```MPI_Recv``` 0-ой процесс суммирует все полученные значения, тем самым получает значение интеграла.

В программе ```solution.c``` я вывожу количество процессов, использованных программой, и значение интеграла. 

После удостоверения, что программа работает верно (= верно считает интеграл для любого числа процессов) программой ```graph.c``` я вывожу среднее время работы для определенного количества процессов. **Важно**: для запуска ```graph.c``` требуется дополнительный параметр — число последовательных исполнений программы для получения усредненного значения времени работы.

## **Запуск**

**Важно**: я работал и запускал не на учебном кластере, поэтому мой build скрипт отличается от предложенного преподавателем.
1. Запустите файл ```build.sh количество_процессов число_разбиений_отрезка```.
2. Или запустите в командной оболочке Unix:

```
mpicc -Wall -Wextra -pedantic -O3 -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wcast-qual \
-Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fsanitize=undefined \
-fno-sanitize-recover=all -fstack-protector -o test solution.cpp -lm
mpirun -np number_of_processes ./test partition_of_the_grid
```

Скрипт для общего кластера смотрите на сайте lms.mipt.ru в вкладке "Параллельное программирование". В методичке "Учебный кластер" описан процесс поставки задачи в очередь и исполнения на учебном кластере.

## **Результат**

В файле ```1.txt``` в первой колонке записано количество процессов, во второй — среднее время исполнения. Для усреднения программа исполнялась 1000 раз. В файле ```graph1.txt``` во второй колонке записаны значения ускорения, в файле ```graph2.txt``` — значения эффективностей.
Далее с помощью команды `gnuplot` я показывал рисовал графики следующими командами:
```
set terminal png size 1024, 720
set xlabel "Number of processes"
set ylabel "Acceleration/Efficiency"
set xrange [1:24]
set output 'graph№.png'
plot "graph№.txt" with lines
```
---
# **Distributed systems**
Course of the distributed systems for the students of DCAM, MIPT

## **Definite integral**

Calculation of the definite integral using trapezoidal rule. The number of interval's partition N must be given by the command line parameters. 
The result of the program must be the integral's value to within 6 digit (use type `double`). Parallelize the program using Message Passing Interface.

Requirements:
1. You have to use calls ```MPI_Send/MPI_Recv```.
2. Plot the graphs of acceleration and efficiency for the number of processes from 1 to 28.

## **How to build**

1. Run ```build.sh number_of_processes partition_of_the_grid``` file.
2. Or you may do it in any UNIX shell:

```
mpicc -Wall -Wextra -pedantic -O3 -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wcast-qual \
-Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fsanitize=undefined \
-fno-sanitize-recover=all -fstack-protector -o test solution.cpp -lm
mpirun -np number_of_processes ./test partition_of_the_grid
```