#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#define MILLION 1000*1000*1000

typedef std::vector<size_t> Number;

// Перевод числа из вида строки в вектор знаковых целых чисел
Number convertToNumber(const std::string& line) {
    Number number;

    // Приведение к знаковому типу, так как length() возращает беззнаковый тип
    // и может произойти переполнение
    // Считываем 9 цифр и конвертируем в число
    for (int i = static_cast<int>(line.length()); i > 0; i-= 9) {
        // Считываем по 9 символов (максимум для int),
        // если их меньше 9, то сколько осталось
        if (i >= 9) {
            number.push_back(std::stoul(line.substr(
                static_cast<size_t>(i) - 9, 9)));
        } else {
            number.push_back(std::stoul(line.substr(
                0, static_cast<size_t>(i))));
        }
    }
    while ((number.size() > 1) && number.back() == 0) {
        number.pop_back();
    }

    return number;
}

// Приведение векторов к общему размеру
void alignToCommonSize(Number* number, const size_t N) {
    while (number->size() < N) {
        number->push_back(0);
    }
    return;
}

// Суммирование элементов
int summarize(Number* answer, const Number& first, const Number& second,
        size_t left, size_t right, size_t overflow) {
    for (size_t i = 0; i < right - left; ++i) {
        answer->data()[i] = first[left + i] + second[left + i] + overflow;
        if (answer->data()[i] >= MILLION) {
            overflow = 1;
            answer->data()[i] -= MILLION;
        } else {
            overflow = 0;
        }
    }

    return static_cast<int>(overflow);
}

// Подсчет необходимого количество ведущих нулей
size_t countDigits(size_t n) {
    size_t digits = 1;

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
        std::cout << "Not enough arguments to the file " << argv[0]
            << std::endl;
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
    if ((!first_stream.is_open()) || (!second_stream.is_open()) ||
            (!result.is_open())) {
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
    while (std::getline(first_stream, first_string)) {}
    while (std::getline(second_stream, second_string)) {}

    // Конец работы с файлами-числами
    first_stream.close();
    second_stream.close();

    // Запись чисел-строк в вектор знаковых чисел
    Number first_number = convertToNumber(first_string);
    Number second_number = convertToNumber(second_string);
    // Размер самого длинного числа
    size_t N = first_number.size() > second_number.size() ?
        first_number.size() : second_number.size();

    // Приведение векторов к общему размеру
    alignToCommonSize(&first_number, N);
    alignToCommonSize(&second_number, N);

    // Определяем зоны ответственности каждого процесса
    size_t left_index = rank * (N / size);
    size_t right_index = (rank != size - 1) ? (rank + 1) * (N / size) : N;

    // Начинаем отсчет времени
    double start = MPI_Wtime();

    // Бит переполнения
    int overflow = 0;
    // Вспомогательные массивы, посчитанные параллельно, на случай переполнения
    Number with_overflow(right_index - left_index);
    Number without_overflow(right_index - left_index);

    // Функции возвращают бит переполнения на случай получения переполнения и
    // нет из предыдущего процесса
    int with = summarize(&with_overflow, first_number, second_number,
        left_index, right_index, 1);
    int without = summarize(&without_overflow, first_number, second_number,
        left_index, right_index, 0);

    if (rank != 0) {
        MPI_Recv(&overflow, 1, MPI_INT, rank - 1,
            0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Вектор в котором будем хранить нужный вектор, посчитанный ранее
    Number answer(right_index - left_index);
    // Рассчитываем правильный ответ и следующий бит переполнения по полученному
    switch (overflow) {
    case 0:
        answer = without_overflow;
        overflow = without;
        break;
    case 1:
        answer = with_overflow;
        overflow = with;
        break;
    }

    // Отсылаем вновь посчитанный бит переполнения
    if (rank != size - 1) {
        MPI_Send(&overflow, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    }
    double end = MPI_Wtime() - start;

    // Обновление результата нулевым процессом
    if (rank == 0) {
        size_t sizeOfAnswer = answer.size();
        for (size_t i = 0; i < sizeOfAnswer; ++i) {
            first_number[i] = answer[i];
        }
    }


    // Сбор данных со всех процессов после последней итерации цикла
    if (size > 1) {
        if (rank == 0) {
            for (int i = 1; i < size; ++i) {
                // Определение местоположения принятых элементов
                size_t left = rank * (N / size);
                size_t right = (rank != size - 1) ? (rank + 1) * (N / size) : N;

                MPI_Recv(first_number.data() + left,
                    static_cast<int>(right - left), MPI_INT, i,
                    0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        } else {
            MPI_Send(answer.data(), static_cast<int>(right_index - left_index),
                MPI_INT, 0, 0, MPI_COMM_WORLD);
            result.close();
            MPI_Finalize();
            return EXIT_SUCCESS;
        }
    }

    // Костыль на случай последней итерации и обнуления последней ячейки
    if (first_number[N - 1] == 0) {
        first_number[N - 1] = MILLION;
    }

    // Запись результата в файл
    // В случае отсутствия элементов записываем 0
    result << (first_number.empty() ? 0 : first_number.back());
    for (int i = static_cast<int>(N - 2); i >= 0; --i) {
        // Количество недостающих ведущих нулей
        size_t sizeOfInt = countDigits(first_number[static_cast<size_t>(i)]);
        // Вывод недостающих ведущих нулей
        while (sizeOfInt < 9) {
            result << "0";
            ++sizeOfInt;
        }
        // Запись в файл
        result << first_number[static_cast<size_t>(i)];
    }

    // Завершаем работу с файлом-вывода
    result.close();

    std::cout << size << " " << end << std::endl;

    MPI_Finalize();
    return EXIT_SUCCESS;
}
