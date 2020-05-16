# **Длинная арифметика**

Реализовать программу сложения чисел произвольной длинны на `MPI`. Провести сравнительные тесты ускорения.

Входные параметры:
1.  Названия двух файлов, содержащие числа, которые хотим просуммировать.

Результат работы программы: сумма двух чисел в файле `result.txt`. 

### **Требования**
1. Распараллелить программу, используя среду MPI (обязательно использовать вызовы `MPI_Send/MPI_Recv`).
2. Построить графики ускорения и эффективности для числа процессов от 1 до 4.

## **Решение**

Программа считывает названия файлов, в которых находятся числа; далее сами числа
из файлов в строки. Далее числа записываются в вектор значимых чисел `int` по 9
цифр в ячейку. Вектор разбивается на блоки (количество блоков = количеству
процессов), которые рассылаются всем процессам (статическая балансировка). В
каждом процессе происходит спекулятивное вычисление результата, после которого
происходит пересылка следующему процессу переноса разряда. После пересылки всех
переносов разряда собирается полный результат, который записывается в файл
`result.txt` root-овым процессом.

В консоль выводится вывести время работы алгоритма (самого процесса сложения). 

## **Запуск**

Компиляция и запуск файла, вывод времени работы в консоль:
```
mpic++ -O0 -o test -std=c++17 solution.cpp -lm 
mpirun --hostfile hostfile -np number_of_proccesses ./test 1.txt 2.txt
```

Компиляция, запуск, запись в файл `res/data.txt`, создание дополнительных файлов
`res/acceleration.txt`, `res/efficiency.txt` и постройка графиков ускорение и
эффективности для количества процессов от 1 до 4:
```
./build.sh 1.txt 2.txt
```

## **Результат**

Результат сложения двух чисел находится в файле `result.txt`. Графики ускорения
и эффективности в `res/acceleration.png`, `res/efficiency.png` соответственно.