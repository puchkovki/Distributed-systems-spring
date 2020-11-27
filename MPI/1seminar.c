#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    // Номер и число процессов
    int numtasks, rank;
    // Инициализация MPI
    MPI_Init(&argc, &argv);
    // Число потоков
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    // Номер текущего потока
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("Number of tasks= %d My rank= %d\n", numtasks, rank);

    // Завершение работы с MPI.
    MPI_Finalize();
    return 0;
}
