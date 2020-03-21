#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <mpi.h>


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
    //Для корректной работы функции
    if (argc < 2)
	{
        printf("No number of partitions!");
		exit(1);
	}

    //Устанавливаем размер коммуникатора и ранг процесса
	int size, rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //Количество шагов
    size_t N = atoll(argv[1]);

    //Количество последовательных выполнений программы для получения среднего времени выполнения
    size_t numexp;
    if(argc < 3)
    {
        numexp = 1;
    } else {
        numexp = atoll(argv[2]);
    }

    //Среднее время выполнения
    double averaged_time = 0;
    for(int i = 0; i < numexp; i++)
    {
        //Начинаем отсчет времени
        double t = MPI_Wtime();
        //Задаем границы интегрирования
        double a = 0, b = 2;
        //Задаем мелкость разбиения отрезка
        double h = (b - a) / N;
        
        //Передаем каждому процессу "свои" индексы интегрирования
        size_t left_index = rank * (N / size);
        size_t right_index = (rank != size - 1) ? (rank + 1) * (N / size) : N;
        //Определяем интеграл на заданном интервале
        double result = Integral(left_index, right_index, h);

        //Отсылаем значения нулевому процессу
        if(rank != 0)
        {
            MPI_Send(&result, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }    

        //Нулевой процесс собирает результаты от остальных процессов
        if(rank == 0)
        {
            for(int i = 1; i < size; i++)
            {
                double tmp;
                MPI_Recv(&tmp, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                result  += tmp;
            }
            //Рассчитываем время выполнения нашей программы
            t = MPI_Wtime() - t;

            averaged_time += t;
        }
    }

    //Вывод размера коммуникатора, используемого программой, и среднего время исполнения для заданного размера
    if(rank == 0)
    {
        printf("%d %lf\n", size, averaged_time/numexp);
    }

	MPI_Finalize();
	return 0;
}