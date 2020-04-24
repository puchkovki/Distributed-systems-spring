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

int main(int argc, char **argv) {
    // Количество шагов
    size_t N = 1000000;
    // Запрошенное кол-во процессов
    int size = 1;
    // Количество последовательных выполнений программы для получения среднего времени выполнения
    size_t numexp = 1;

    if (argc > 1) {
        N = atoll(argv[1]);
		if (argc > 2) {
            size = atoi(argv[2]);
            if(argc > 3) {
                numexp = atoll(argv[3]);
            }
        }
    }

    //Задаем границы интегрирования
    double a = 0, b = 2;
    //Задаем мелкость разбиения отрезка
    double h = (b - a) / N;
    double result = (Func(0) + Func(N * h)) / 2;

    omp_set_num_threads(size);
    #pragma omp parallel
    {
        //Устанавливаем размер коммуникатора и ранг процесса
        int rank = omp_get_thread_num();

        #pragma omp for reduction(+: result) nowait
        for(size_t i = 1; i < N; i++) {
            result += Func(i * h);
        }
    }

    result *= h;
    //Вывод кол-ва процессов, используемого программой, и значение интеграла
    printf("%d %lf\n", size, result);

	return EXIT_SUCCESS;
}