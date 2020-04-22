#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <omp.h>


//Определение функции
double Func(double x) {
    //Недействительные значения не должны вносить вклад в интеграл
    if(x > 2) 
    {
        return 0;
    }
    return sqrt(4 - x*x);
}

//Формула Котеса рассчета определенного интеграла для равномерной сетки
double Integral(size_t left_index, size_t right_index, double h) {
    double I = (Func(right_index * h) + Func(left_index * h)) / 2;

    for(size_t i = left_index + 1; i < right_index; i++) {
        I += Func(i * h);
    }

    return I * h;
}

int main(int argc, char **argv)
{
    //Для корректной работы функции требуется не менее 2 аргументов
    if (argc < 2)
	{
        printf("No number of partitions!");
		exit(1);
	}

    //Количество шагов
    size_t N = atoll(argv[1]);
    int size = atoi(argv[2]);

    //Задаем границы интегрирования
    double a = 0, b = 2;
    //Задаем мелкость разбиения отрезка
    double h = (b - a) / N;
    double result = 0.0;

    omp_set_num_threads(size);
#pragma omp parallel reduction (+: result)
{
    //Устанавливаем размер коммуникатора и ранг процесса
	int rank = omp_get_thread_num();
    //Передаем каждому процессу "свои" индексы интегрирования
    size_t left_index = rank * (N / size);
    size_t right_index = (rank != size - 1) ? (rank + 1) * (N / size) : N;
    //Определяем интеграл на заданном интервале
    result += Integral(left_index, right_index, h);
}
    //Вывод размера коммуникатора, используемого программой, и значение интеграла
    printf("%d %lf\n", size, result);

	return EXIT_SUCCESS;
}