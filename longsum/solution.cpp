#include <stdlib.h>
#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>

int main(int argc, char** argv) {

    if (argc < 3) {
        std::cout << "Not enough arguments" << std::endl;
    }

    // Считать с командной строки названий файлов, в которых лежат искомые числа
    std::string first_number = argv[1];
    std::string second_number = argv[2];
    std::ifstream first(first_number);
    std::ifstream second(second_number);

    // ООбъекты класса ofstream, при связке с файлами по умолчанию содержат
    // режимы открытия файлов  ios_base::out | ios_base::trunc. То есть файл
    // будет создан, если не существует. Если же файл существует, то его 
    // содержимое будет удалено, а сам файл будет готов к записи.
    std::ofstream result("result.txt");

    if((!first.is_open()) || (!second.is_open()) || (!result.is_open())) {
        std::cout << ("Error opening files\n") << std::endl;
        return EXIT_FAILURE;
    }

    MPI_Status status = {};
    // Устанавливаем размер коммуникатора и ранг процесса
	int size, rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    /*if (rank == 0) {

    }*/

    first.close();
    second.close();
    result.close();

    MPI_Finalize();

    return EXIT_SUCCESS;
}