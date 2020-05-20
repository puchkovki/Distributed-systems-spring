#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define Length 1.0
#define Temperature_1 1.0
#define Temperature_2 5.0

int main(int argc, char **argv)
{
	// Время, когда требуется посчитать распределение температуры в стержне
	double Time = 1.0;
	// Число разбиений по координате
	size_t M = 10;
	// Количество паралельных процессов
	size_t size = 1;


	if (argc > 1) {
		// Считываем время, когда хотим узнать распределение температуры в стержне
        Time = atof(argv[1]);
		if (Time < 0) {
			printf("Sorry, timemachine hasn't been invented yet!");
			return EXIT_FAILURE;
		}
		if (argc > 2) {
            // Число разбиений по координате
			M = atoll(argv[2]);
			if (M < 2) {
				// Иначе метод не сходится
				printf("Invalid values!\n");
				return EXIT_FAILURE;
			}
            if(argc > 3) {
				size = atoll(argv[3]);
				if(M <= size) { // Если мелкость разбиения координаты настолько 
				// мала, что не будут использованы все процессы
					printf("Required number of processes is unreasonablecompared to coordinate partition!\n");
					return EXIT_FAILURE;
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

	// Создание массив замков
    omp_lock_t* lock = (omp_lock_t*) malloc(sizeof(omp_lock_t) * 2 * size);
    //Инициализация массива замков
	for (size_t i = 0; i < 2 * size; ++i) {
		omp_init_lock(&lock[i]);
	}	

	// Вспомогательная переменная, показывающая кол-во процессов, закончивших данную итерацию цикла
	size_t epoc = 0;

	// Задаем кол-во процессов для следующего распараллеливания
	omp_set_num_threads(size);
	#pragma omp parallel private(n, m)
	{
		size_t id = omp_get_thread_num();
		// Цикл по времени
		for (n = 0; n < N; n++) {
			// Обнуляем глобальную эпоху
			#pragma omp single
			{
				epoc = 0;
			}

			// Явная четырехточечная схема
			for (m = left_index[id]; m < left_index[id + 1]; ++m) {
				
				if ((m == left_index[id]) && (id != 0)) {
					// Запоминаем боковой узел
					omp_set_lock(&lock[id - 1 + size]);
					double left = u0[left_index[id] - 1];
					omp_unset_lock(&lock[id - 1 + size]);

					// Проводим защищенно вычисления
					omp_set_lock(&lock[id]);
					u1[m] = u0[m] + 0.3 * (left - 2.0 * u0[m] + u0[m + 1]);
					omp_unset_lock(&lock[id]);
				}
				
				if ((m == left_index[id + 1] - 1) && (id != size - 1)) {
					// Запоминаем боковой узел
					omp_set_lock(&lock[id + 1]);
					double right = u0[left_index[id + 1]];
					omp_unset_lock(&lock[id + 1]);

					// Проводим защищенно вычисления
					omp_set_lock(&lock[id + size]);
					u1[m] = u0[m] + 0.3 * (u0[m - 1] - 2.0 * u0[m] + right);
					omp_unset_lock(&lock[id + size]);
				}

				u1[m] = u0[m] + 0.3 * (u0[m - 1] - 2.0 * u0[m] + u0[m + 1]);
			}
				
			// Атомарно инкрементируем, показывая, что процесс закончил работу
			#pragma omp atomic
			epoc++;

			#pragma omp single
			{
				// Не обновляем результат, пока не проработали все процессы
				while (epoc < size) {
					__asm volatile ("pause" ::: "memory");
				}

				// Обновление результатов
				double *t = u0;
				u0 = u1;
				u1 = t;
			}
		}
	}
		
    // Удаление замка
	for (size_t i = 0; i < 2 * size; ++i) {
    	omp_destroy_lock(&lock[i]);
	}
	
	// Вывод на экран
	for (m = 0; m < M; m++) {
		printf("%lf %lf\n", m * h, u1[m]);
	}
	
    // Освобождение памяти
	free(u0);
	free(u1);

	return EXIT_SUCCESS;
}