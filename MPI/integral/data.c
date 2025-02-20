#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int size = 0;
    double time = 1, tmp = 0;

    FILE* data = fopen("res/data.txt", "rt");
    FILE* acceleration = fopen("res/acceleration.txt", "w");
    FILE* efficiency = fopen("res/efficiency.txt", "w");
    if (!data) {
        printf("Error open data.txt\n");
        return EXIT_FAILURE;
    }
    if (!acceleration) {
        printf("Error open acceleration.txt\n");
        return EXIT_FAILURE;
    }
    while (!feof(data)) {
        if (fscanf(data, "%d %lf\n", &size, &time) != 0) {
            if (size == 1) {
                tmp = time;
            }
            fprintf(acceleration, "%d %lf\n", size, tmp / time);
            fprintf(efficiency, "%d %lf\n", size, tmp / time / size);
        }
    }
    fclose(data);
    fclose(acceleration);
    fclose(efficiency);

    return EXIT_SUCCESS;
}
