#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <mpi.h>

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

// Суммирование элементов
void summarize(Number& first, Number& second, size_t& left, size_t& right,
        size_t& overflow, int& rank, int& size, size_t& overflowDepth, size_t& N,
        bool& lastOverflowSend, bool& lastOverflowRecv) {
    bool second_null = true;
    for (auto i: second) {
        if (i != 0) {
            second_null = false;
        }
    }
    // Пересчет массива только в случае:
    // это первая итерация
    // на предыдущей произошло переполнение
    if ((overflow != 0) || !second_null) {
        for (size_t i = left; i < right; ++i) {
            first[i] += second[i] + overflow;
            if (first[i] >= million) {
                overflow = 1;
                first[i] -= million;
            } else {
                overflow = 0;
            }
        }
    }
    
    // Отсылаем если есть следующий процесс
    // Не отсылаем, если в прошлый раз отослали 0, так как переполниться снова
    // невозможно
    if ((rank != (size - 1) && !lastOverflowSend)) {
        // На следующей итерации отсылка не нужна
        if (overflow == 1) {
            lastOverflowSend = true;
        }
        MPI_Send(&overflow, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    }

    // Принимаем только в следующих случаях:
    // есть предыдущий процесс
    // все предыдущие процессы уже должны были передать
    // Не принимаем, если в прошлый раз отослали 1, так как переполниться снова
    // невозможно
    if ((rank != 0) && (overflowDepth < rank) && !lastOverflowRecv){
        MPI_Recv(&overflow, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);
        if (overflow == 1) {
            lastOverflowRecv = true;
        }
    }

    // Пересчитываем заново
    if (++overflowDepth <= rank) {
        // Требуется пересчет только с переполненой цифрой
        Number auxiliary(N);
        summarize(first, auxiliary, left, right, overflow, rank, size,
        overflowDepth, N, lastOverflowSend, lastOverflowRecv);

        // Если элемент самый последний, то вместо отсылки инкрементируем число
        if (first[N - 1] == 0) {
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

    // Приведение векторов к общему размеру
    alignToCommonSize(first_number, N);
    alignToCommonSize(second_number, N);
    
    // Определяем зоны ответственности каждого процесса
    size_t left_index = rank * (N / size);
    size_t right_index = (rank != size - 1) ? (rank + 1) * (N / size) : N;

    // Переменная, переносимая в следующий "разряд"
    size_t overflow = 0;
    // Переменная, определяющая глубину рекурсии "переполнения"
    size_t overflowDepth = 0;
    // Переменная, отвечающая за то, отослали ли мы в прошлый раз 1
    bool lastOverflowSend = false;
    // Переменная, отвечающая за то, приняли ли мы в прошлый раз 1
    bool lastOverflowRecv = false;

    //Начинаем отсчет времени
    double start = MPI_Wtime();
    // Вычисление суммы
    summarize(first_number, second_number, left_index, right_index, overflow, rank, size, overflowDepth, N, lastOverflowSend, lastOverflowRecv);

    double end = MPI_Wtime() - start;
    // Сбор данных со всех процессов после последней итерации цикла
	if(size > 1) {
		if(rank == 0) {
			for(int i = 1; i < size; ++i) {
                // Определение местоположения принятых элементов 
                size_t left = (i * (N / size)), right = (i != size - 1) ? (i + 1) * (N / size) : N;

				MPI_Recv(first_number.data() + left, right - left, MPI_INT, i,
                0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		} else {
			MPI_Send(first_number.data() + left_index, right_index - left_index,
            MPI_INT, 0, 0, MPI_COMM_WORLD);
            result.close();
            MPI_Finalize();
            return EXIT_SUCCESS;
		}
	}
    // Запись результата в файл
    // В случае отсутствия элементов записываем 0
    result << (first_number.empty() ? 0 : first_number.back());
    for (int i = N - 2; i >= 0; --i) {
        // Количество недостающих ведущих нулей
        int sizeOfInt = getCountsOfDigits(first_number[i]);
        // Вывод недостающих ведущих нулей
        while (sizeOfInt < 9) {
            result << "0";
            ++sizeOfInt;
        }
        // Запись в файл
        result << first_number[i];
    }
    
    // Завершаем работу с файлом-вывода
    result.close();

    std::cout << size << " " << end << std::endl;

    MPI_Finalize();
    return EXIT_SUCCESS;
}