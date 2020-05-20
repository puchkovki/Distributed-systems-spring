#include <iostream>
#include <omp.h>

int main(void) {
    int i;
    #pragma omp parallel private(i)
    {
        #pragma omp for
        for (i = 0; i < 10; i++) {
            printf("#%d: i = %d\n", omp_get_thread_num(), i);
        }
    }

    /*std::cout << "Serial" << std::endl;
    #pragma omp parallel num_threads(13)
    {
        printf(" Parallel\n");
    }
    std::cout << "Serial" << std::endl;

    #ifdef _OPENMP
        printf("OpenMP is supported, version %d.\n", _OPENMP);
    #endif*/
    return EXIT_SUCCESS;
}
