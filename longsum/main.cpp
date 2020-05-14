#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <mpi.h>
#include <malloc.h>
#include <string.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#define BLOCKSIZE 9
using namespace std;

void putNumsInArray(int* firstArray, int* secondArray, int size, char* firstString, char* secondString) {
    int* num = (int*)calloc(size,sizeof(int));
    for (int i = 0; i < size; i++) {
        num[i] = firstString[i] - '0';
    }
    int sum = 0;
    // size /  9 = кол-во слов
    for (int i = 0; i < size / 9; i++) {
        sum = 0;
        // идем по записанным цифрам
        for (int k = 0; k < 9; k++) {
            sum += num[k + 9*i] * pow(10, 9 - k - 1);
        }
        firstArray[i] = sum;
    }

    for (int i = 0; i < size; i++) {
        num[i] = secondString[i] - '0';
    }
    sum = 0;
    for (int i = 0; i < size / 9; i++) {
        sum = 0;
        for (int k = 0; k < 9; k++) {
            sum += num[k + 9 * i] * pow(10, 9 - k - 1);
        }
        secondArray[i] = sum;
    }
}


string finalizeCalculations(vector<int> prevRes) {
    string result;
    for(int i = prevRes.size() - 1; 0 < i; i--) {
        if(prevRes[i] > 999999999) {
            prevRes[i-1] += 1;
            prevRes[i] -= 1000000000;
        }
    }
    for(int i = 0; i < prevRes.size(); i++) {
        stringstream ss;
        if(prevRes[i] < 100000000) {
            result+='0';
        }
        ss << prevRes[i];
        string str = ss.str();
        result+=str;
    }
    return result;
}


int main(int argc, char* argv[])
{

    int rank = 0;
    int size = 0;
    MPI_Status status = {};
    double startTime = 0;
    double endTime = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int arrForward[3];//[0] position    [1] first    [2] second
    int arrBackward[2];
    if (rank == 0)
    {
        FILE * fp;
        fp = fopen ("file.txt", "w+");
        fprintf(fp, "90\n");
        //fprintf(fp, "111111111222222222333333333222222222111111111\n");
        //fprintf(fp, "111111111222222222333333333222222222111111111\n");
        fprintf(fp, "210390398120474761576935762396471047010471923857239865106925730756205761537691576657253434\n");
        fprintf(fp, "345973459834759835479834759834759834759834598345984958374958379485739485793485935428022432\n");
        fclose(fp);
        fp = fopen("file.txt","r");

        size_t n = 0;

        char *length = NULL;
        char *firstnumber = NULL;
        char *secondnumber = NULL;
        int readsize = getline(&length, &n, fp);
        getline(&firstnumber, &n, fp);
        getline(&secondnumber, &n, fp);

        int number_length = atoi(length);
        if (readsize == -1) {
            printf("Could not read file");
            return 1;
        }
        int blocksNumber = number_length / BLOCKSIZE;//на сколько девятизнач элем можно раздробить число
        vector<int> prevResult(blocksNumber);
        int* firstarray = (int*)malloc(sizeof(int) * blocksNumber);
        int* secondarray = (int*)malloc(sizeof(int) * blocksNumber);

        putNumsInArray(firstarray, secondarray, number_length, firstnumber, secondnumber);

        printf("Shedule : %d blocks(%d)\n", blocksNumber, BLOCKSIZE);



        int blockPtr = 0;
        int slave = 0;
        startTime = MPI_Wtime();

        for (int i = 1; i < size; i++) {
            arrForward[0] = i - 1;
            arrForward[1] = firstarray[i - 1];
            arrForward[2] = secondarray[i - 1];
            MPI_Send(arrForward, 3, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        for (blockPtr = size - 1; blockPtr < blocksNumber; blockPtr++)
        {
            MPI_Recv(arrBackward, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            slave = status.MPI_SOURCE;
            prevResult[arrBackward[0]] = arrBackward[1];

            arrForward[0] = blockPtr;
            arrForward[1] = firstarray[blockPtr];
            arrForward[2] = secondarray[blockPtr];

            MPI_Send(arrForward, 3, MPI_INT, slave, 0, MPI_COMM_WORLD);
        }

        for (int i = 1; i < size ; i++) {
            MPI_Recv(arrBackward, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            prevResult[arrBackward[0]] = arrBackward[1];
            int arrEnd[3] = {-1, 0, 0};
            slave = status.MPI_SOURCE;
            MPI_Send(arrEnd, 3, MPI_INT, slave, 0, MPI_COMM_WORLD);
        }
        cout << "prevResult: ";
        for(int i = 0; i < 5; i++) {
            cout << prevResult[i] << " ";
        }
        cout<<endl;
        string finalResult =  finalizeCalculations(prevResult);
        cout << "finalResult: " << finalResult << endl;
    } else {
        while (true)
        {
            MPI_Recv(arrForward, 3, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (arrForward[0] == -1) {
                break;
            }
            cout<< "slave rank: "<< rank << " recieved : " << arrForward[0] << " " << arrForward[1] << " " << arrForward[2] << endl;
            arrBackward[1] = arrForward[1] + arrForward[2];
            arrBackward[0] = arrForward[0];

            MPI_Send(arrBackward, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0)
    {
        endTime = MPI_Wtime();
        printf("[TIME RES] %lf\n", endTime - startTime);
    }

    MPI_Finalize();
    return 0;
}