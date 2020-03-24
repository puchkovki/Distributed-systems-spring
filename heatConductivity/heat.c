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

	double Time = atof(argv[1]);
	if (Time < 0) {
		printf("Sorry, timemachine hasn't been invented yet!");
	}

    //Число разбиений по координате
	int M = atoi(argv[2]);
	if (M < 2) {
		printf("Invalid values!\n");
	}
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

	// Задаем начальные условия (f(x) = 0 )
	for (m = 0; m < M; m++) 
    {
		u0[m] = u1[m] = 0.0;
	}

	// Задаем граничные условия
	u0[0] = u1[0] = Temperature_1;
	u0[M - 1] = u1[M - 1] = Temperature_2;
	
	//Передаем каждому процессу "свои" индексы интегрирования
    size_t left_index = (rank != 0) ? rank * (M / size) : 1;
    size_t right_index = (rank != size - 1) ? (rank + 1) * (M / size) : M - 1;

	 // Интегрируем по времени
	for (n = 0; n < N; n++) {
		// Обмен краевыми узлами
		MPI_Send(&u1[left_index], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		MPI_Recv(&u1[left_index - 1], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Send(&u1[right_index - 1], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		MPI_Recv(&u1[right_index], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		
		for (m = left_index; m < right_index - 1; m++) {
			u1[m] = u0[m] + 0.3  * (u0[m-1] - 2.0 * u0[m] + u0[m+1]);
		}
		double *t = u0;
		u0 = u1;
		u1 = t;
	}

	//Обновление данных со всех процессов после последней итерации цикла
	if((rank != 0) && (rank != size - 1)) {
		MPI_Send(&u1[left_index], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		MPI_Recv(&u1[left_index - 1], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Send(&u1[right_index - 1], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		MPI_Recv(&u1[right_index], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	if(rank == 0) {
		MPI_Send(&u1[right_index - 1], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		MPI_Recv(&u1[right_index], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	if(rank == size - 1) {
		MPI_Send(&u1[left_index], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		MPI_Recv(&u1[left_index - 1], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	
	// Вывод на экран
	for (m = 0; m < M; m++) {
		printf("%lf %lf\n", m * h, u1[m]);
	}
	
    //Освобождение памяти
	free(u0);
	free(u1);

    MPI_Finalize();
	return 0;
}