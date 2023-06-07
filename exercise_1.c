#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILEPATH "/home/exercise_1/array"

/* isSymmetrical: return these status values */
#define SYMMETRICAL 10
#define ASYMMETRICAL 11

/* isDiagonallyDominant: return these status values */
#define DIAGONALLY_DOMINANT 20
#define NOT_DIAGONALLY_DOMINANT 21


/* -----------------------------ARRAY TYPES----------------------------- */
typedef struct Array1D
{
    int columnLen;
    int *column;
}Array1D;


typedef struct Array2D
{
    int rowLen;
    Array1D *row;
}Array2D;


/* -------------------------------FUNCTIONS------------------------------- */
/* Initialize attributes of type Array2D */
void initArray2D(Array2D *array)
{
    array->rowLen = 0;
    array->row = NULL;
}

/* Adds a row on 2D array */
void addRow(Array2D *array)
{
	array->rowLen++;		// increment the row length of the array
	
	/* allocate memory for the extra row  */
	array->row = realloc(array->row, array->rowLen * sizeof(Array1D));

	if(array->row == NULL)
		perror("Array Row Memory Allocation: ");

    /* initialize column attributes */
	array->row[array->rowLen-1].columnLen = 0;
	array->row[array->rowLen-1].column = NULL;
}

/* Adds a column on linear array */
void addColumn(Array1D *array)
{
	array->columnLen++;		// increment the column length of the array

	/* allocate memory for the extra column */
	array->column = realloc(array->column, array->columnLen * sizeof(int));
	
	if(array->column == NULL)
		perror("Array Column Memory Allocation: ");
}

/* Insert a new colunm with a value in a specific row */
void insertColumnValue(Array2D *array, int rowIndex, int value)
{
	if (rowIndex > array->rowLen)       // row index has greater value than the row length
	{
		perror("Row Index is to Big: ");
	}
	else if(rowIndex == array->rowLen)  // allocate next row, as it is not exists
	{
		addRow(array);
	}
	addColumn(&array->row[rowIndex]);	// adding a column

	array->row[rowIndex].column[array->row[rowIndex].columnLen-1] = value; // pass the value
}

/* reads and import an Array2D*/
Array2D readArray2D(char *filePath)
{
	FILE *file;

	file = fopen(filePath, "r");
	if(file == NULL)
		perror("Opening file: ");

	int inputValue;			// keeps the extracted number of the file
	int rowIndex = 0;		// keep the row index

	Array2D inputArray;
	initArray2D(&inputArray);

	char* line = NULL;
	size_t lineSize = 0;

    /* Read the file by line */
	while(getline(&line, &lineSize, file) != -1)            // get a line from file, if exists
    {

        line[strcspn(line, "\n")] = 0;                      // replace the newline with the zero

        char * token = strtok(line, " ");                   // tokenized part of line

        inputValue = atoi(token);                          // convert into a number
        insertColumnValue(&inputArray, rowIndex, inputValue);            // insert into the array

        while((token = strtok(NULL, " ")) != NULL)          // tokenize each data with the space
        {
        	inputValue = atoi(token);                      // convert into a number
            insertColumnValue(&inputArray, rowIndex, inputValue);        // insert into the array
        }
        rowIndex++;                                      // update row number
    }

    fclose(file);

    return inputArray;
}

/* display an Array2D on terminal */
void displayArray2D(Array2D array)
{
	if (array.rowLen == 0 || array.row->columnLen == 0)
		return ;

	printf("Array:\n");
	for (int rowIndex = 0; rowIndex < array.rowLen; ++rowIndex)
	{
		for (int column = 0; column < array.row[rowIndex].columnLen; ++column)
		{
			printf("%d ", array.row[rowIndex].column[column]);
		}
		printf("\n");
	}
}

/* deallocate memory of Array2D pointers */
void freeArray2D(Array2D *array)
{
	if (array->row != NULL)
	{
		for (int rowIndex = array->rowLen; rowIndex < array->rowLen; --rowIndex)
			if(array->row[rowIndex].column != NULL)
				free(array->row[rowIndex].column);
			

		free(array->row);	
	}
}

/* checks if the rows and columns lengths are equal */
int isSymetrical(Array2D array)
{
	for (int rowIndex = 0; rowIndex < array.rowLen; ++rowIndex)
	{
		if (array.rowLen != array.row[rowIndex].columnLen)
		{
			return ASYMMETRICAL;
		}
	}

	return SYMMETRICAL;
}

/* Check if the given element position is dominant in the array */
int isDiagonallyDominant(Array2D array)
{
	double startTime, endTime;
	int diagonalStatus = DIAGONALLY_DOMINANT;			// variable for the return value is set to diagonally dominant status code 
	

	/* ------------------PARALLEL CODE PART------------------ */
	startTime = omp_get_wtime();						// start time stamp

	/* every thread share the array and return status variable */
	#pragma omp parallel for schedule(static) shared(array, diagonalStatus)
	for (int rowIndex = 0; rowIndex < array.rowLen; ++rowIndex)				// each thread calculate 1 row
	{
		int rowSum = 0;

		for (int columnIndex = 0; columnIndex < array.row[rowIndex].columnLen; ++columnIndex)	// read each column of the row 
		{
			if (rowIndex != columnIndex)									// exclude the diagonal element of the array
			{
				rowSum += abs(array.row[rowIndex].column[columnIndex]);		// sum all the column values
			}
		}
		
		if (rowSum >= abs(array.row[rowIndex].column[rowIndex]) )			// if the diagonal element is less than sum of the row
		{
			diagonalStatus = NOT_DIAGONALLY_DOMINANT;						// write in asynchronous matter the negative status
		}
	}

	endTime = omp_get_wtime();							// end time stamp

	printf("\n\tExecution time: \"isDiagonallyDominant\": %lf\n", endTime - startTime);
	
	return diagonalStatus;		// if at least one thread write in variable, this will return the negative status
}

/* Calulate the maximun value */
int maxValue(Array2D array)
{
	double startTime, endTime;
	int maxValue = abs(array.row[0].column[0]);				// The first element of the array become the max


	startTime = omp_get_wtime();						// start time stamp

	/* ------------------PARALLEL CODE PART------------------ */
	#pragma omp parallel for schedule(static) shared(array) collapse(2) reduction(max: maxValue)
	for (int rowIndex = 0; rowIndex < array.rowLen; ++rowIndex)
		for (int columnIndex = 0; columnIndex < array.rowLen; ++columnIndex)
		{
			if (maxValue < abs(array.row[rowIndex].column[columnIndex]) )
			{
				maxValue = abs(array.row[rowIndex].column[columnIndex]);
			}
		}

	endTime = omp_get_wtime();							// end time stamp
	printf("\n\tExecution time: \"maxValue\": %lf\n", endTime - startTime);

	return maxValue;
}

/* Allocate memory for array from a given size */
void createSymmetricalSize(Array2D *targetArray, int dimension)
{

	targetArray->rowLen = dimension;									// define the row size
	targetArray->row = malloc(targetArray->rowLen * sizeof(Array1D));	// allocate memory for row size(Array1D type)

	if (targetArray->row == NULL)										// if allocation fails, exit with error
		perror("At row memory allocation of target array: ");
	

	for (int rowIndex = 0; rowIndex < targetArray->rowLen; rowIndex++)	// for each row create equal size column
	{
		targetArray->row[rowIndex].columnLen = dimension;
		targetArray->row[rowIndex].column = malloc(targetArray->row[rowIndex].columnLen * sizeof(int));
		

		if (targetArray->row[rowIndex].column == NULL)					// if allocation fails, exit
			perror("At column memory allocation of target array: ");

	}
}

/* Build an array with the property B(i,j) = m – |A(i,j| for each i!=j and B(i,j) = max for each i=j */
Array2D buildArray(const Array2D inputArray, int max)
{
	double startTime, endTime;
	Array2D outputArray;								// array with the result calculations

	initArray2D(&outputArray);

	createSymmetricalSize(&outputArray, inputArray.rowLen);		// create a square array with row and column dimensions equal to row(same as columns as it is checked that is symmetric) 
		

	startTime = omp_get_wtime();						// start time stamp


	#pragma omp parallel for schedule(static) shared(outputArray, inputArray, max) collapse(2)	// every thread will access elements in rows and columns independently
	for (int rowIndex = 0; rowIndex < inputArray.rowLen; rowIndex++)
		for (int columnIndex = 0; columnIndex < inputArray.rowLen; columnIndex++)
		{
			if (rowIndex != columnIndex)
				outputArray.row[rowIndex].column[columnIndex] = max - abs(inputArray.row[rowIndex].column[columnIndex]);

			else
				outputArray.row[rowIndex].column[columnIndex] = max;
		}
	
	endTime = omp_get_wtime();							// end time stamp
	printf("\n\tExecution time: \"buildArray\": %lf\n", endTime - startTime);

	return outputArray;
}

/* Calculate the minimun value with reduction clause */
int minValue_Reduction(Array2D array)
{
	double startTime, endTime;
	int minValue = abs(array.row[0].column[0]);				// The first element of the array become the max

	startTime = omp_get_wtime();						// start time stamp

	/* ------------------PARALLEL CODE PART------------------ */
	#pragma omp parallel for schedule(static) shared(array) collapse(2) reduction(min: minValue)
	for (int rowIndex = 0; rowIndex < array.rowLen; ++rowIndex)
		for (int columnIndex = 0; columnIndex < array.rowLen; ++columnIndex)
		{
			if (minValue > abs(array.row[rowIndex].column[columnIndex]) )
			{
				minValue = abs(array.row[rowIndex].column[columnIndex]);
			}
		}

	endTime = omp_get_wtime();							// end time stamp
	printf("\n\tExecution time(reduction clause): \"minValue\": %lf\n", endTime - startTime);

	return minValue;
}

/* Calculate the minimun value with critical section */
int minValue_Critical(Array2D array)
{
	double startTime, endTime;
	int minValue = abs(array.row[0].column[0]);				// The first element of the array become the max

	startTime = omp_get_wtime();						// start time stamp

	/* ------------------PARALLEL CODE PART------------------ */
	#pragma omp parallel for schedule(static) shared(array) collapse(2)
	for (int rowIndex = 0; rowIndex < array.rowLen; ++rowIndex)
		for (int columnIndex = 0; columnIndex < array.rowLen; ++columnIndex)
		{
			#pragma omp critical 
			if (minValue > abs(array.row[rowIndex].column[columnIndex]) )
			{
				minValue = abs(array.row[rowIndex].column[columnIndex]);
			}
		}

	endTime = omp_get_wtime();							// end time stamp
	printf("\n\tExecution time(critical section): \"minValue\": %lf\n", endTime - startTime);

	return minValue;
}

/* Calculate the minimun value with binary tree */
int minValue_BinaryTree(Array2D array)
{
	double startTime, endTime;

	const int threadNum = omp_get_max_threads();
	int threadID;

	int minValue[threadNum];
	

	startTime = omp_get_wtime();					// start time stamp

	/* ------------------PARALLEL CODE PART------------------ */
	#pragma omp parallel shared(array) private(threadID)
	{
		threadID = omp_get_thread_num();
		minValue[threadID] = array.row[0].column[0];

		#pragma omp for schedule(static) collapse(2)
		for (int rowIndex = 0; rowIndex < array.rowLen; ++rowIndex)
		{

			for (int columnIndex = 0; columnIndex < array.rowLen; ++columnIndex)
			{
				if (minValue[threadID] > abs(array.row[rowIndex].column[columnIndex]) )
				{
					minValue[threadID] = abs(array.row[rowIndex].column[columnIndex]);
				}
			}
		}


		/* Reduction  */
		for (int thread = 2; thread < 2*threadNum-1; thread*=2)
		{
			#pragma omp barrier
			{
				if (threadID%thread == 0 && threadID+thread/2 < threadNum)
				{
					if (minValue[threadID] > minValue[threadID+thread/2])
					{
						minValue[threadID] = minValue[threadID+thread/2];

					}
				}
			}
		}
		

	}

	endTime = omp_get_wtime();							// end time stamp
	printf("\n\tExecution time(binary tree): \"minValue\": %lf\n", endTime - startTime);

	return minValue[0];
}

void main ()
{
	int threadsSize;				// variable that the user set the number of threads
    Array2D arrayA;					// variable for the input array
	Array2D arrayB;					// variable for the result array

	int max, min;

	/* User input the thread number */
	printf("Enter the number of threads: ");
	scanf("%d", &threadsSize);

	omp_set_num_threads(threadsSize);

	/* initialize arrays*/
    initArray2D(&arrayA);
    initArray2D(&arrayB);

    arrayA = readArray2D(FILEPATH);	// read the array from a file and store it

	if(isSymetrical(arrayA) == SYMMETRICAL)	// checking if array is symetrical 
	{
		printf("The array is: %sdiagonaly dominant\n", isDiagonallyDominant(arrayA)==DIAGONALLY_DOMINANT? "": "not ");
	}
	else
	{
		printf("The array is not symmetrical.\n");
		goto CLEAN_N_TERMINATE;
	}	

	max = maxValue(arrayA);
	printf("Array A max value: %d\n", max);


	arrayB = buildArray(arrayA, max);
	displayArray2D(arrayB);


	min = minValue_Reduction(arrayB);
	printf("Array B min value: %d\n", min);

	min = minValue_Critical(arrayB);
	printf("Array B min value: %d\n", min);

	min = minValue_BinaryTree(arrayB);
	printf("Array B min value: %d\n", min);
	
	/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^TERMINATING PROCESS^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
	CLEAN_N_TERMINATE:
    freeArray2D(&arrayA);
    freeArray2D(&arrayB);

	return;
}