#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define Length 1.0
#define Temperature_1 1.0
#define Temperature_2 2.0

int main(int argc, char **argv)
{
    if (argc < 3)
	{
        printf("No expected time and step: %s \n", argv[0]);
		exit(1);
	}

    //Устанавливаем размер коммуникатора и ранг процесса
	int size, rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	size_t Time = atoll(argv[1]);

    //Число разбиений по координате
	size_t M = atoll(argv[2]);
    //Шаг по координате
	double h = Length / M;

    //Шаг по времени (число Куранта)
	double tau = 0.3 * h * h;
    //Число разбиений по времени
	int N = Time / tau;

    //Массивы температуры для момента времени n и n + 1 соответственно
	double *u0 = (double*) malloc(sizeof(double) * M);
	double *u1 = (double*) malloc(sizeof(double) * M);

    //Счетчики для циклов по времени и координате
	int m, n;

	// Задаем начальные условия
	for (m = M - 1; m < M + 1; m--) 
    {
		u0[m] = u1[m] = 0.0;
	}

	// Задаем граничные условия
	u0[0] = u1[0] = Temperature_1;
	u0[M - 1] = u1[M - 1] = Temperature_2;
	
	// Интегрируем по времени
	for (n = 0; n < N; n++) {
		//добавить обмен данными краевых узлов send recv send recv
		for (m = 1; m < M - 1; m++) {//изменить границы отрезков на "свои" для каждого процесса
			u1[m] = u0[m] + tau / h / h  * (u0[m-1] - 2.0 * u0[m] + u0[m+1]);
		}
		double *t = u0;
		u0 = u1;
		u1 = t;
	}
	
	//   Вывод на экран
	for (m = 0; m < M; m++) {
		printf("%lf %lf\n", m * h, u1[m]);
	}
	
    //Освобождение памяти
	free(u0);
	free(u1);

    MPI_Finalize();
	return 0;
}