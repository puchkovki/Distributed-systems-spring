#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <omp.h>

// Определение функции
double Func(double x) {
    // Недействительные значения не должны вносить вклад в интеграл
    if (x > 2) {
        return 0;
    }
    return sqrt(4 - x*x);
}

// Формула Котеса рассчета определенного интеграла для равномерной сетки
double Integral(size_t left_index, size_t right_index, double h) {
    double I = (Func(right_index * h) + Func(left_index * h)) / 2;
    for (size_t i = left_index + 1; i < right_index; i++) {
        I += Func(i * h);
    }
    return I * h;
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

    // Создание замка
    omp_lock_t lock;
    // Инициализация замка
    omp_init_lock(&lock);

    for (size_t i = 0; i < numexp; i++) {
        // Начинаем отсчет времени
        double start = omp_get_wtime();

        // Устанавливаем требуемое кол-во процессов
        omp_set_num_threads(size);
        // Начало параллельной секции
        #pragma omp parallel
        {
            // Устанавливаем ранг процесса
            int rank = omp_get_thread_num();

            // Передаем каждому процессу "свои" индексы интегрирования
            size_t left_index = rank * (N / size);
            size_t right_index =
                (rank != size - 1) ? (rank + 1) * (N / size) : N;
            // Определяем интеграл на заданном интервале
            double integral = Integral(left_index, right_index, h);

            // Заблокировать замок
            omp_set_lock(&lock);
            // Сбор значений со всех потоков
            result += integral;
            // Разблокировать замок
            omp_unset_lock(&lock);
        }

        // Суммирование времени работы
        averaged_time += (omp_get_wtime() - start);
    }
    // Удаление замка
    omp_destroy_lock(&lock);

    // Вывод кол-ва процессов и усредненного времени работы
    printf(" %d %lf\n", size, averaged_time / numexp);

    return EXIT_SUCCESS;
}
