#include <vector>
#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>

typedef std::vector<int> Number;
#define million 1000*1000*1000

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
    while ((line.size() > 1) && line.back() == 0) {
        line.pop_back();
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

// Итеративное суммирование элементов
void summarize(Number& first, Number& second, size_t& left, size_t& right,
        size_t& overflow, int& rank, int& size, size_t& overflowDepth) {
    // Пересчет элементов
    for (size_t i = left; i < right; ++i) {
		first[i] += second[i] + overflow;
        if (first[i] >= million) {
            overflow = 1;
            first[i] -= million;
        } else {
            overflow = 0;
        }
	}
    /*std::cout << "Rank is " << rank << " with left index " << left << " and right index " << right << ": ";
    for(int i = left; i < right; ++i) {
            std::cout << first[i] << " ";
    }
    std::cout << std::endl;*/

    // Отсылаем если есть следующий процесс
    if (rank != (size - 1)) {
        MPI_Send(&overflow, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
        //std::cout << "#" << rank << ": send " << overflow << ". Depth is " << overflowDepth << std::endl;
    }

    // Принимаем только в случае, когда есть предыдущий процесс и когда предыдущий
    // процесс сделал итерацию
    if ((rank != 0) && (overflowDepth < rank)){
        MPI_Recv(&overflow, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //std::cout << "#" << rank << ": receive " << overflow << ". Depth is " << overflowDepth << std::endl;
    }

    // Пересчитываем, когда пришло значение 0/1
    if (++overflowDepth <= rank) {
        Number auxiliary(first.size());
        summarize(first, auxiliary, left, right, overflow, rank, size, overflowDepth);
        if (first[first.size() - 1] == 0) {
            ++first[right - 1];
        }
    }

    return;
}

// Подсчет необходимого количество ведущих нулей
int getCountsOfDigits(int n) {
    int digits = 1;
    
    if (n >=10) {
        digits++;
        if (n >=100) {
            digits++;
            if (n >=1000) {
                digits++;
                if (n >=10000) {
                    digits++;
                    if (n >=100000) {
                        digits++;
                        if (n >=1000000) {
                            digits++;
                            if (n >=10000000) {
                                digits++;
                                if (n >=100000000) {
                                    digits++;
                                    if (n >=1000000000) {
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
    // Работаем дальше только в случае, если хватает данных-файлов (чисел)
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

    // Прекращаем работу, если открытие файлов прошло некорректно
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
    // Запись чисел в строки
    while (std::getline(first_stream, first_string));
    while (std::getline(second_stream, second_string));
    
    // Конец работы с файлами-числами
    first_stream.close();
    second_stream.close();

    // Запись чисел-строк в вектор знаковых чисел
    Number first_number = convertToNumber(first_string), second_number = convertToNumber(second_string);
    // Размер самого длинного числа
    size_t N = first_number.size() > second_number.size() ? first_number.size() : second_number.size();

    alignToCommonSize(first_number, N);
    alignToCommonSize(second_number, N);
    
    // Определяем зоны ответственности каждого процесса
    size_t left_index = rank * (N / size);
    size_t right_index = (rank != size - 1) ? (rank + 1) * (N / size) : N;

    size_t overflow = 0, overflowDepth = 0;
    // Вычисление суммы двух массивов
    summarize(first_number, second_number, left_index, right_index, overflow, rank, size, overflowDepth);

    // Сбор данных со всех процессов после последней итерации цикла
	if(size > 1) {
		if(rank == 0) {
			for(int i = 1; i < size; ++i) {
                size_t left = (i * (N / size)), right = (i != size - 1) ? (i + 1) * (N / size) : N;

				MPI_Recv(first_number.data() + left, right - left, MPI_INT, i,
                0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                std::cout << "#" << rank << ": receive "
                << *(first_number.data() + left) << " at " << left << " position " << std::endl;
			}
		} else {
			MPI_Send(first_number.data() + left_index, right_index - left_index,
            MPI_INT, 0, 0, MPI_COMM_WORLD);
            std::cout << "#" << rank << ": send "
            << *(first_number.data() + left_index) << " at " << left_index << " position " << std::endl;
            //return EXIT_SUCCESS;
		}
	}    
    //MPI_Barrier(MPI_COMM_WORLD);

    // Запись результата в файл
    // В случае отсутствия элементов записываем 0
    if (rank == 0) {
        result << (first_number.empty() ? 0 : first_number.back());
        for (int i = N - 2; i >= 0; --i) {
            // Рассчитываем кол-во пропущенных ведущих нулей
            int sizeOfInt = getCountsOfDigits(first_number[i]);
            //std::cout << "Size of element " << i << " is " << sizeOfInt << std::endl;
            // Выводим недостающие ведущие нули
            while (sizeOfInt < 9) {
                result << "0";
                ++sizeOfInt;
            }
            // Запись в файл
            result << first_number[i];
        }
    }
    
    // Завершаем работу с файлом-вывода
    result.close();

    /*if (rank == 0) {
        for (int i = N - 1; i >= 0; --i) {
            int sizeOfInt = getCountsOfDigits(first_number[i]);
            //std::cout << "Size of element " << i << " is " << sizeOfInt << std::endl; 
            while (sizeOfInt < 9) {
                result << "0";
                ++sizeOfInt;
            }

            std::cout << first_number[i];
        }
    }*/

    MPI_Finalize();
    return EXIT_SUCCESS;
}