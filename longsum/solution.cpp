#include <vector>
#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>

typedef std::vector<int> Number;
#define million 1000000000

// Перевод числа из вида строки в вектор знаковых целых чисел
Number convertToNumber(std::string& line) {
    Number number;

    // Приведение к знаковому типу, так как length() возращает беззнаковый тип и может
    // произойти переполнение
    // Считываем 9 цифр и конвертируем в число
    for (int i = (int)line.length(); i > 0; i-= 9) {
        // Считываем по 9 символов (максимум для int), 
        // если их меньше 9, то сколько осталось
        if (i >= 9) {
            number.push_back(atoi( line.substr(i - 9, 9).c_str() ));
        } else {
            number.push_back(atoi( line.substr(0, i).c_str() ));
        }
    }

    return number;
}

// Приведение векторов к общему размеру
void alignToCommonSize (Number& number, size_t& N) {
    while (number.size() < N) {
        number.push_back(0);
    }
    return;
}

void summarize(Number& first, Number& second, size_t& left, size_t& right, int& overflow) {
    for (int i = left; i < right; ++i) {
		first[i] += second[i] + overflow;
        if (first[i] > 999999999) {
            overflow = 1;
            first[i] -= million;
        } else {
            overflow = 0;
        }
	}
    return;
}

int getCountsOfDigits(int n) {
    int digits = 1;
    
    if (n > 10) {
        digits++;
        if (n > 100) {
            digits++;
            if (n > 1000) {
                digits++;
                if (n > 10000) {
                    digits++;
                    if (n > 100000) {
                        digits++;
                        if (n > 1000000) {
                            digits++;
                            if (n > 10000000) {
                                digits++;
                                if (n > 100000000) {
                                    digits++;
                                    if (n > 1000000000) {
                                        digits++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return digits;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "Not enough arguments to the file " << argv[0] << std::endl;
        return EXIT_FAILURE;
    }

    // Считать с командной строки названий файлов, в которых лежат искомые числа
    std::string first_file = argv[1];
    std::string second_file = argv[2];
    std::ifstream first_stream(first_file);
    std::ifstream second_stream(second_file);

    // Объекты класса ofstream, при связке с файлами по умолчанию содержат
    // режимы открытия файлов  ios_base::out | ios_base::trunc. То есть файл
    // будет создан, если не существует. Если же файл существует, то его 
    // содержимое будет удалено, а сам файл будет готов к записи.
    std::ofstream result("result.txt");

    if((!first_stream.is_open()) || (!second_stream.is_open()) || (!result.is_open())) {
        std::cout << ("Error opening files\n") << std::endl;
        return EXIT_FAILURE;
    }
    
    // Устанавливаем размер коммуникатора и ранг процесса
	int size, rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Строки, содержащие числа из файлов
    std::string first_string, second_string;
    // Запись чисел
    while (std::getline(first_stream, first_string));
    while (std::getline(second_stream, second_string));
    
    // Закончили работу с файлами-числами
    first_stream.close();
    second_stream.close();

    Number first_number = convertToNumber(first_string), second_number = convertToNumber(second_string);
    // Размер самого длинного числа
    size_t N = first_number.size() > second_number.size() ? first_number.size() : second_number.size();

    alignToCommonSize(first_number, N);
    alignToCommonSize(second_number, N);
    
    // Определяем зоны ответственности каждого процесса
    size_t left_index = rank * (N / size);
    size_t right_index = (rank != size - 1) ? (rank + 1) * (N / size) : N;

   
    int overflow = 0;
    // Вычисление суммы двух массивов
    summarize(first_number, second_number, left_index, right_index, overflow);
    
    if (rank != (size - 1)) {
        MPI_Send(&overflow, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    }
    if (rank != 0) {
        MPI_Recv(&overflow, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);        
    }
    /*for(auto i: first_number) {
            std::cout << i << " ";
    }
    if (rank == 0)
        std::cout << std::endl << left_index << " " << right_index << std::endl;
    for(auto i: second_number) {
        std::cout << i << std::endl;
    }*/
    while (overflow == 1) {
        for (int i = left_index; i < right_index; ++i) {
            first_number[i] += overflow;
            if (first_number[i] > 999999999) {
                overflow = 1;
                first_number[i] -= million;
            } else {
                overflow = 0;
            }
        }
        if (rank != (size - 1)) {
            MPI_Send(&overflow, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
        }
        if (rank != 0) {
            MPI_Recv(&overflow, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);        
        }
    }

    if ((overflow == 1) && (right_index = N)) {
        first_number[right_index]++;
    }
    int epoc = 0;
    while (epoc < rank) {
        __asm volatile ("pause" ::: "memory");
    }
    epoc++;

    // Сбор данных со всех процессов после последней итерации цикла
	if(size > 1) {
		if(rank == 0) {
			for(int i = 1; i < size; ++i) {
				MPI_Recv(first_number.data() + left_index, right_index - left_index, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		} else {
			MPI_Send(first_number.data() + left_index, right_index - left_index, MPI_INT, 0, 0, MPI_COMM_WORLD);
		}
	}    

    MPI_Finalize();
    
    for (int i = (int)right_index; i > (int)left_index ; --i) {
        int sizeOfInt = getCountsOfDigits(first_number[i]);
        while (sizeOfInt < 9) {
            result << "0";
            ++sizeOfInt;
        }

        result << first_number[i];
    }
    result.close();
    /*for(auto i: first_number) {
        std::cout << i << std::endl;
    }
    //std::cout << first << std::endl;
    //std::cout << second << std::endl;*/

    return EXIT_SUCCESS;
}