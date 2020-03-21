# **Распределенные системы**
Семестровый курс по распределенным системам для студентов 3 курса ФУПМ, МФТИ.

## **Определенный интеграл**

Вычисление определенного интеграла, используя метод трапеции. Параметры программы передаются через аргументы командной строки (число разбиений отрезка N).

Результат работы программы — значение интеграла с точностью до 6 знаков после запятой (тип `double`). Распаралелить программу, используя среду MPI. 

Требования:
1. Обязательно использовать вызовы ```MPI_Send/MPI_Recv```.
2. Построить графики ускорения и эффективности для числа процессов от 1 до 28.


## **Одномерное уравнение теплопроводности**

Решение одномерного уравнения теплопроводности при заданных граничных условиях и начальном распределении.

# **Distributed systems**
Course of the distributed systems for the students of DCAM, MIPT

## **Definite integral**

Calculation of the definite integral using trapezoidal rule. The number of interval's partition N must be given by the command line parameters. 
The result of the program must be the integral's value to within 6 digit (use type `double`). Parallelize the program using Message Passing Interface.

Requirements:
1. You have to use calls ```MPI_Send/MPI_Recv```.
2. Plot the graphs of acceleration and efficiency for the number of processes from 1 to 28.

## **Homogeneous equation of heat conduction**
