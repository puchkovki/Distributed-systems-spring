#include <stdio.h>
#include <stdlib.h>

#define Length 1.0
#define Temperature_1 1.0
#define Temperature_2 2.0

int main(int argc, char **argv)
{
    if (argc <= 1)
	{
        printf("No number of partitions: %s \n", argv[0]);
		exit(1);
	}

	size_t Time = atoll(argv[1]);
	int M = atoi(argv[2]);
	double h = Length / M;
	double tau = 0.3 * h * h;
	int N = Time / tau;
	double *u0 = (double*) malloc(sizeof(double) * M);
	double *u1 = (double*) malloc(sizeof(double) * M);
	int m, n;
	
	/* Задаем начальные условия. */
	for (m = 0; m < M; m++) {
		u0[m] = u1[m] = 0.0;
	}
	/* Задаем граничные условия. */
	u0[0] = u1[0] = Temperature_1;
	u0[M - 1] = u1[M - 1] = Temperature_2;
	
	/* Интегрируем по времени. */
	for (n = 0; n < N; n++) {
		for (m = 1; m < M - 1; m++) {
			u1[m] = u0[m] + tau / h / h  * (u0[m-1] - 2.0 * u0[m] + u0[m+1]);
		}
		double *t = u0;
		u0 = u1;
		u1 = t;
	}
	
	/* Вывод на экран. */
	for (m = 0; m < M; m++) {
		printf("%lf %lf\n", m * h, u1[m]);
	}
	
	free(u0);
	free(u1);
	return 0;
}