#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <mpi.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <string>
#include <algorithm>
#include <vector>
#include <boost/lexical_cast.hpp>


#define FAIL 999992
#define OK 999991
#define MORE 9999923
#define NOMORE 999966

using namespace std;
using namespace boost;

template <class T>
int numDigits(T number)
{
	int digits = 0;
	if (number < 0) digits = 1; // remove this line if '-' counts as a digit
	while (number) {
		number /= 10;
		digits++;
	}
	return digits;
}


void putNumsInArray(int* firstArray, int* secondArray, int size, char* firstString, char* secondString) {

	int* num = (int*)calloc(size, sizeof(int));
	for (int i = 0; i < size; i++) {
		num[i] = firstString[i] - '0';
	}
	int sum = 0;
	for (int i = 0; i < size / 9; i++) {
		sum = 0;
		for (int k = 0; k < 9; k++) {
			sum += num[k + 9 * i] * pow(10, 9 - k - 1);
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

//Each element should 9 digit value
string* arrayToString(int* array, int length) {
	string* str = new string();
	int k = 0;
	for (int i = 0; i < length; i++) {
		if (numDigits(array[i]) < 9)
			str->append(9 - numDigits(array[i]), '0');
		*str += lexical_cast<string>(array[i]);
	}
	//cout << "array to str str = " << *str << "\n";
	fflush(stdout);
	return str;
}

void findSum(vector<int> first, vector<int> second, vector<int>* res) {
	//lengths of first and second numbers are equal
	int carry = 0;

	for (int i = first.size(); i > 0; i--) {
		int sum_res = (first[i - 1] + second[i - 1] + carry);
		carry = 0;
		if (sum_res / 1000000000 == 1) {
			carry = 1;
			sum_res = sum_res % 1000000000;
		}
		res->push_back(sum_res);
	}

	res->push_back(carry);
	reverse(res->begin(), res->end());
}

int isAllNine(vector<int> number) {
	for (int i = 0; i < number.size(); i++) {
		if (number[i] != 999999999) {
			return 0;
		}
	}
	return 1;
}

void findSumWithInt(vector<int> first, int n, vector<int>* res) {
	//lengths of first and second numbers are equal
	int carry = n;

	for (int i = first.size(); i > 0; i--) {
		int sum_res = (first[i - 1] + carry);
		carry = 0;
		if (sum_res / 10000000000 == 1) {
			carry = 1;
			sum_res = sum_res % 10000000000;
		}
		res->push_back(sum_res);
	}

	//res->push_back(carry);
	reverse(res->begin(), res->end());
}

int main(int argc, char* argv[])
{
	char holder;
	char* length = NULL;
	char* firstnumber = NULL;
	char* secondnumber = NULL;

	int argStatus = FAIL;
	MPI_Status status = {};
	int tasksNumber = 0;
	int blockSize = 9;
	int elemSize = 0;

	int position_of_local_task = 0;
	int first_local_number = 0;
	int second_local_number = 0;

	char* str_num = (char*)malloc(sizeof(char) * 10);

	int fail = 0;
	int rank = 0;
	int size = 0;
	double startTime = 0;
	double endTime = 0;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0)
	{

		//Check file opening
		char* filename = argv[1];
		FILE* fp = fopen(filename, "r");
		if (fp == NULL) {
			printf("Could not open file %s", filename);
			return 1;
		}
		//read file
		size_t n = 0;

		int readsize = getline(&length, &n, fp);
		readsize = getline(&firstnumber, &n, fp);
		readsize = getline(&secondnumber, &n, fp);

		int number_length = atoi(length);
		if (readsize == -1) {
			printf("Could not read file %s", filename);
			return 1;
		}

		// Root branch
		//
		// Check arguments
		if (argc != 2)
		{
			printf("Usage:\n required 1 argument (> 0)\
            \n for tasks number and block size\n");
			// Exit with fail status
			argStatus = FAIL;
			MPI_Bcast(&argStatus, 1, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Finalize();
			return 0;
		}
		blockSize = 9;
		
		int blocksNumber = number_length / blockSize;
		elemSize = blocksNumber / (size-1);
		// Send size of block
		argStatus = OK;
		MPI_Bcast(&elemSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
		// Prepare slave's tasks
		int* firstarray = (int*)malloc(sizeof(int) * blocksNumber);
		int* secondarray = (int*)malloc(sizeof(int) * blocksNumber);

		putNumsInArray(firstarray, secondarray, number_length, firstnumber, secondnumber);
		char subbuf[1024];

		//allocate matrix of results: 1 row - first sum possible, 2 -second sum possible
		vector< vector<vector<int>* > > res(2, vector<vector<int>* >(blocksNumber));

		// Send blockss for work begin
		int blockPtr = 0;
		int buf = 0;
		int slave = 0;
		int* recievedNumber = (int*)malloc((elemSize + 1) * sizeof(int));
		int position = -1;

		startTime = MPI_Wtime();
		for (blockPtr = 1; blockPtr < size; blockPtr++)
		{
			MPI_Send(&(firstarray[blockPtr-1]), elemSize, MPI_INT, blockPtr, OK, MPI_COMM_WORLD);
			MPI_Send(&(secondarray[blockPtr-1]), elemSize, MPI_INT, blockPtr, OK, MPI_COMM_WORLD);
		}
		position = -1;
		// Receive results and send confirmation, so processes will end their job
		for (int i = 1; i < size; i++)
		{
			MPI_Recv(recievedNumber, elemSize + 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			position = status.MPI_SOURCE - 1;
			slave = status.MPI_SOURCE;
			res[0][position] = new vector<int>(recievedNumber, recievedNumber + elemSize + 1);
			
			if (status.MPI_TAG == MORE) {
				MPI_Recv(recievedNumber, elemSize + 1, MPI_INT, slave, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				res[1][position] = new vector<int>(recievedNumber, recievedNumber + elemSize + 1);
			}

			MPI_Send(&fail, 1, MPI_INT, slave, FAIL, MPI_COMM_WORLD);
		}

		//Finalize calculations
		string resulting_string = "";
		for (int i = size - 3; i >= 0; i--) {
			if ((res[0][i + 1]->front()) == 1) {
				if (res[1][i] != NULL) {
					res[0][i] = res[1][i];
				}
				else {

					int temp = res[0][i]->back();
					temp += 1;
					res[0][i]->pop_back();
					res[0][i]->push_back(temp);
				}
			}
		}

		//push carrier of first elem if it is non 0 (with sum it can only be 1)
		if (res[0][0]->front() == 1)
			resulting_string += res[0][0]->front() + '0';

		for (int i = 0; i < size-1; i++) {
			resulting_string += *arrayToString(&(res[0][i]->at(1)), elemSize);
		}
		cout << "res =  " << resulting_string;
		fflush(stdout);
	}
	else
	{
		// Slave branch
		//
		// Check status
		MPI_Bcast(&elemSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
		if (elemSize == FAIL)
		{
			MPI_Finalize();
			return 0;
		}

		// Calc loop
		int* first = (int*)malloc(sizeof(int) * elemSize);
		int* second = (int*)malloc(sizeof(int) * elemSize);


		
			
		//check tag of the message before receiving it
		MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			

		MPI_Recv(first, elemSize, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			

		MPI_Recv(second, elemSize, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		//move all elements to vector<int>

		vector<int> *firstV = new vector<int>(first, first + elemSize);
		vector<int> *secondV = new vector<int>(second, second + elemSize);

		vector<int>* res0 = new vector<int>();
		vector<int>* res1 = new vector<int>();
		findSum(*firstV, *secondV, res0);
		//if all symbols are '9', get second variant of calculation
		int moreResults = NOMORE;
		int size1 = elemSize + 1;
		int size0 = elemSize + 1;


		if (res0->back() == 999999999) {
			findSumWithInt(*res0, 1, res1);
			moreResults = MORE;
		}


		//use tag as indicator of more results
		MPI_Send(&((*res0)[0]), size0, MPI_INT, 0, moreResults, MPI_COMM_WORLD);
		if (moreResults == MORE) {
			MPI_Send(&res1[0], size1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		}
		MPI_Recv(first, 1, MPI_INT, 0, FAIL, MPI_COMM_WORLD, &status);
			
	}
	printf("finish, %d\n", rank);
	fflush(stdout);
	// Finish
	MPI_Barrier(MPI_COMM_WORLD);
	if (rank == 0)
	{
		endTime = MPI_Wtime();
		printf("[TIME RES] %lf\n", endTime - startTime);
	}

	MPI_Finalize();
	return 0;
}