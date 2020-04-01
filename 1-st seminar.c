#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    int numtasks, rank; // Номер и число процессов.

    MPI_Init(&argc, &argv); // Инициализация MPI.

    MPI_Comm_size(MPI_COMM_WORLD, &numtasks); // Число потоков.
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);     // Номер текущего потока.

    printf("Number of tasks= %d My rank= %d\n", numtasks, rank);
    MPI_Finalize(); // Завершение работы с MPI.
    return 0;
}