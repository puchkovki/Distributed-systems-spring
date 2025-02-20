#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define Length 1.0
#define Temperature_1 1.0
#define Temperature_2 5.0

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("No expected time and step: %s \n", argv[0]);
        return EXIT_FAILURE;
    }

    // Устанавливаем размер коммуникатора и ранг процесса
    int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Считываем время, когда хотим узнать распределение температуры в стержне
    double Time = atof(argv[1]);
    if (Time < 0) {
        if (rank == 0) {
            printf("Sorry, timemachine hasn't been invented yet!");
            MPI_Finalize();
            return EXIT_FAILURE;
        }
    }

    // Число разбиений по координате
    int M = atoi(argv[2]);
    if (M < 2) {
        // Иначе метод не сходится
        if (rank == 0) {
            printf("Invalid values!\n");
            MPI_Finalize();
            return EXIT_FAILURE;
        }
    } else if (M <= size) {
        // Если мелкость разбиения координаты настолько мала,
        // что не будут использованы все процессы
        if (rank == 0) {
            printf("Required number of processes is unreasonable compared \
                to coordinate partition!\n");
            MPI_Finalize();
            return EXIT_FAILURE;
        }
    }

    // Шаг по координате
    double h = Length / M;
    // Шаг по времени (число Куранта)
    double tau = 0.3 * h * h;
    // Число разбиений по времени
    int N = Time / tau;

    // Массивы температуры для момента времени n и n + 1 соответственно
    double *u0 = (double*) malloc(sizeof(double) * M);
    double *u1 = (double*) malloc(sizeof(double) * M);

    // Счетчики для циклов по времени и координате
    size_t m, n;

    // Начальные условия (f(x) = 0 )
    for (m = 0; m < M; m++) {
        u0[m] = u1[m] = 0.0;
    }

    // Задаем граничные условия
    u0[0] = u1[0] = Temperature_1;
    u0[M - 1] = u1[M - 1] = Temperature_2;

 // Массив индексов передаваемых точек
    size_t *left_index = (size_t*) malloc(sizeof(size_t) * size + 1);
    left_index[0] = 1;
    // Чтобы избежать костылей при передаче массивов 0-ому процессу,
    // определяю правый конец последнего массива
    left_index[size] = M - 1;
    // Итеративно определяю левые концы отрезков, передаваемые каждому процессу
    // Правый конец i-го процесса = левому концу (i + 1)-го
    for (int i = 1; i < size; i++) {
        left_index[i] = left_index[i - 1] + (M / size) +
            ((i - 1) < ((M % size) - 2));
    }

    // Цикл по времени
    for (n = 0; n < N; n++) {
        // Обмен краевыми узлами
        if (rank != 0) {
            MPI_Send(u0 + left_index[rank], 1, MPI_DOUBLE, rank - 1, 0,
            MPI_COMM_WORLD);
            MPI_Recv(u0 + left_index[rank] - 1, 1, MPI_DOUBLE, rank - 1, 0,
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        if (rank != size - 1) {
            MPI_Send(u0 + left_index[rank + 1] - 1, 1, MPI_DOUBLE, rank + 1, 0,
            MPI_COMM_WORLD);
            MPI_Recv(u0 + left_index[rank + 1], 1, MPI_DOUBLE, rank + 1, 0,
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // Явный метод
        for (m = left_index[rank]; m < left_index[rank + 1]; m++) {
            u1[m] = u0[m] + 0.3  * (u0[m - 1] - 2.0 * u0[m] + u0[m + 1]);
        }
        // Обновление результатов
        double *t = u0;
        u0 = u1;
        u1 = t;
    }

    // Сбор данных со всех процессов после последней итерации цикла
    if (size > 1) {
        if (rank == 0) {
            for (int i = 1; i < size; i++) {
                MPI_Recv(u1 + left_index[i], left_index[i + 1] - left_index[i],
                    MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        } else {
            MPI_Send(u1 + left_index[rank], left_index[rank + 1] -
                left_index[rank], MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
    }

    // Вывод на экран распределения температуры
    if (rank == 0) {
        for (m = 0; m < M; m++) {
            printf("%lf %lf\n", m * h, u1[m]);
        }
    }

    // Освобождение памяти
    free(u0);
    free(u1);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
