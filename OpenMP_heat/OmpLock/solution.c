#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define Length 1.0
#define Temperature_1 1.0
#define Temperature_2 5.0

int main(int argc, char **argv)
{
	double Time = 1.0;
	int M = 10, size = 1;
	// Количество последовательных выполнений программы для получения среднего времени выполнения
    size_t numexp = 1;


	if (argc > 1) {
		// Считываем время, когда хотим узнать распределение температуры в стержне
        Time = atof(argv[1]);
		if (Time < 0) {
			printf("Sorry, timemachine hasn't been invented yet!");
			return EXIT_FAILURE;
		}
		if (argc > 2) {
            // Число разбиений по координате
			M = atoi(argv[2]);
			if (M < 2) {
				// Иначе метод не сходится
				printf("Invalid values!\n");
				return EXIT_FAILURE;
			}
            if(argc > 3) {
				size = atoll(argv[3]);
				if(M <= size) {
				// Если мелкость разбиения координаты настолько мала, 
				// что не будут использованы все процессы
					printf("Required number of processes is unreasonable compared to coordinate partition!\n");
					return EXIT_FAILURE;
				}
				if (argc > 4) {
					numexp = atoll(argv[3]);
				}
            }
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
	for (m = 0; m < M; m++) 
    {
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
	for(int i = 1; i < size; i++) {
		left_index[i] = left_index[i - 1] + (M / size) + ((i - 1) < ((M % size) - 2));
	}

	double time = 0.0;

	omp_set_num_threads(size);
	for(size_t j = 0; j < numexp; j++) {
		// Начинаем отсчет времени
		double start = omp_get_wtime();

		for (n = 0; n < N; n++) {	 // Цикл по времени
			// Явный метод
			#pragma omp parallel
            {
                int id = omp_get_thread_num();
				for (m = left_index[id]; m < left_index[id + 1]; m++) {
					u1[m] = u0[m] + 0.3  * (u0[m - 1] - 2.0 * u0[m] + u0[m + 1]);
				}
            }
			// Обновление результатов
			double *t = u0;
			u0 = u1;
			u1 = t;
		}
		// Рассчитываем время работы программы
		time += omp_get_wtime() - start;
	}
	
	// Вывод на экран
	/*for (m = 0; m < M; m++) {
		printf("%lf %lf\n", m * h, u1[m]);
	}*/
	printf("\n %d %lf\n", size, time / numexp);
	
    //Освобождение памяти
	free(u0);
	free(u1);

	return EXIT_SUCCESS;
}