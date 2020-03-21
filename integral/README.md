# **Определенный интеграл**

## **Задача**

Вычисление определенного интеграла $ \int_a^b f(x)\,dx.$ , используя метод трапеции. Параметры программы передаются через аргументы командной строки (число разбиений отрезка N).

$
f(x) = \sqrt{4 - x^2} 
$

Результат работы программы — значение интеграла с точностью до 6 знаков после запятой (тип `double`). Распаралелить программу, используя среду MPI. 

Требования:
1. Обязательно использовать вызовы ```MPI_Send/MPI_Recv```.
2. Построить графики ускорения и эффективности для числа процессов от 1 до 28.

## **Решение**

Для распараллеливания программ я делю отрезок на количество интервалов, равное кол-ву процессов, запущенных системой. Интервалы непересекаются, и значения, вычисленные 

## **Запуск**

**Важно**: я работал и запускал не на учебном кластере, поэтому мой build скрипт отличается от предложенного преподавателем. Нужный для общего кластера — teacher.sh
1. Запустите файл ```build.sh количество_процессов число_разбиений_отрезка```.
2. Или запустите в командной оболочке Unix:

```
mpicc -Wall -Wextra -pedantic -O3 -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wcast-qual \
-Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fsanitize=undefined \
-fno-sanitize-recover=all -fstack-protector -o test solution.cpp -lm
mpirun -np number_of_processes ./test partition_of_the_grid
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