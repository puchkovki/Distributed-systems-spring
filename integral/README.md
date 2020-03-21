# **Определенный интеграл**

## **Задача**

Вычисление определенного интеграла, используя метод трапеции. Параметры программы передаются через аргументы командной строки (число разбиений отрезка `N`).

Результат работы программы — значение интеграла с точностью до 6 знаков после запятой (тип `double`). Распараллелить программу, используя среду MPI. 

Требования:
1. Использование вызовов ```MPI_Send/MPI_Recv```.
2. Построение графиков ускорения и эффективности для числа процессов от 1 до 28.

## **Решение**

Отрезок интегрирования делится на интервалы, количество которых равно количеству процессов, запущенных системой. Интервалы не пересекаются, и каждый процесс параллельно считает значение функции на своем интервале. Далее значения передаются с помощью ```MPI_Send``` 0-ому процессу, который с помощью ```MPI_Recv``` суммирует все полученные значения, тем самым получает значение интеграла.

```solution.c``` выводит количество процессов, использованных программой, и значение интеграла. ```graph.c``` выводит среднее время работы программы для определенного количества процессов. 

**Важно**: для запуска ```graph.c``` требуется дополнительный параметр — число последовательных исполнений программы для получения усредненного значения времени работы.

## **Запуск**

**Важно**: данный алгоритм запуска не работает на учебном кластере, так как я работал на другом сервере. Данный алгоритм будет работать на вашем устройстве.
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

В файле ```1.txt``` первой колонкой записано количество процессов, второй — среднее время их выполнения. Выборка — 1000 последовательных исполнений. В файле ```graph1.txt``` второй колонкой записаны ускорения, в файле ```graph2.txt``` — эффективности.
С помощью программы `gnuplot` я нарисовал графики по файлам ```graph№.txt```:
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