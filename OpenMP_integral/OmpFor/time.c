#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <omp.h>
#include <time.h> 

// Определение функции
double Func(double x) {
    // Недействительные значения не должны вносить вклад в интеграл
    if (x > 2) {
        return 0;
    }
    return sqrt(4 - x*x);
}

int main(int argc, char **argv) {
    // Количество шагов
    size_t N = 1000000;
    // Запрошенное кол-во процессов
    int size = 1;
    // Количество последовательных выполнений программы
    // для получения среднего времени выполнения
    size_t numexp = 1;

    if (argc > 1) {
        N = atoll(argv[1]);
        if (argc > 2) {
            size = atoi(argv[2]);
            if (argc > 3) {
                numexp = atoll(argv[3]);
            }
        }
    }

    // Задаем границы интегрирования
    double a = 0, b = 2;
    // Задаем мелкость разбиения отрезка
    double h = (b - a) / N;
    double result = 0.0;

    // Среднее время выполнения
    double averaged_time = 0.0;
    for (size_t j = 0; j < numexp; j++) {
        // Начинаем отсчет времени
        double start = omp_get_wtime();
        result = (Func(0) + Func(N * h)) / 2;

        // Задаем кол-во процессов для следующего распараллеливания
        omp_set_num_threads(size);
        // Статическое распределение итераций с шагом в 10^4
        // правильное суммирование всех параллельных вычислений
        #pragma omp parallel for schedule(static, 10000) reduction(+: result)
            for (size_t i = 1; i < N; i++) {
                result += Func(i * h);
            }

        result *= h;
        averaged_time += (omp_get_wtime() - start);
    }

    // Вывод кол-ва процессов, используемых программой, и значение интеграла
    printf("%d %lf\n", size, averaged_time / numexp);
    return EXIT_SUCCESS;
}
