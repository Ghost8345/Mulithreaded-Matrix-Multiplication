#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

//--------------------------------------------------------------------------------------------------------------------//
//Global Variables and Structs.
double **arr1;
double **arr2;
double **arr3;
int row1, column1, row2, column2, row3, column3;
char *outputFileName;
typedef struct dimensions {
    int rowNumber, columnNumber;
} dimensions;
struct timeval stop, start;
//--------------------------------------------------------------------------------------------------------------------//
// Function to read our matrix from our file and assign it to one of three matrices available.
void readMatrix(char *fileName, int arrayNumber) {

    FILE* file = fopen(fileName, "r");
    if (file == NULL) {
        printf("Error: text file nonexistent\n");
        exit(1);
    }

    if (arrayNumber == 1){
        fscanf(file,"row=%d col=%d", &row1, &column1);
        arr1 = (double**)malloc(row1 * sizeof(double*));
        for (int i = 0; i < row1; i++){
            arr1[i] = (double*)malloc(column1 * sizeof(double));
            for (int j = 0; j < column1; j++){
                fscanf(file, "%lf", arr1[i] + j);
            }
        }
    }
    else if(arrayNumber == 2){
        fscanf(file,"row=%d col=%d", &row2, &column2);
        arr2 = (double**)malloc(row2 * sizeof(double*));

        for (int i = 0; i < row2; i++){
            arr2[i] = (double*)malloc(column2 * sizeof(double));
            for (int j = 0; j < column2; j++){
                fscanf(file, "%lf", arr2[i] + j);
            }
        }
    }

    fclose(file);
}
//--------------------------------------------------------------------------------------------------------------------//
// Function used to get command line arguments and check for files and adjust accordingly.
void setUp(int argc, char *argv[]){

    if (argc == 1){ //Checks if program was started with no arguments and uses default values.

        readMatrix("a.txt",  1);

        readMatrix("b.txt", 2);

        outputFileName = "c";

    }
    else if (argc == 4){ //If user entered Custom Arguments then it handles it.

        char file1[50] = "";
        strcat(file1, argv[1]);
        strcat(file1,".txt");

        char file2[50] = "";
        strcat(file2, argv[2]);
        strcat(file2,".txt");

        readMatrix(file1, 1);
        readMatrix(file2, 2);

        outputFileName = argv[3];

    }
    else{ //Checks for whether user entered wrong number of arguments 2, 4, 5 ,etc.
        printf("Please enter valid arguments or no arguments at all to use default\n");
        exit(1);
    }
}
//--------------------------------------------------------------------------------------------------------------------//
// Function used to validate whether two arrays can be multiplied together.
// ALso used to dynamically allocate memory to our third matrix based on the size of the first two matrices.
void validate(){
    if(column1 != row2){
        printf("Matrices cannot be multiplied, please check the dimension of each matrix\n");
        exit(2);
    }

    row3 = row1;
    column3 = column2;
    arr3 = (double**)malloc(row3 * sizeof(double*));
    for (int i = 0; i < row3; i++){
        arr3[i] = (double*)malloc(column3 * sizeof(double));
    }

}
//--------------------------------------------------------------------------------------------------------------------//
// Function that is invoked by thread in one thread per matrix method.
// Contains algorithm for normal matrix multiplication
void* MOT(void* arg){ //No arguments passed to function.

    for (int i = 0; i < row3; i++) {
        for (int j = 0; j < column3; j++) {
            for (int k = 0; k < row2; ++k) {
                arr3[i][j] += arr1[i][k] * arr2[k][j];
            }
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------//
// Function that creates one thread, calculates time of execution, creates and writes to output file.
void multOneThread(){

    int threadCount = 0;

    // Get Time of Execution
    gettimeofday(&start, NULL);

    pthread_t thread; // one thread only created.
    if (pthread_create(&thread, NULL, &MOT, NULL)){
        perror("Thread Creation Failed");
    }
    threadCount++; // joined after (works sequentially) (No MultiThreading)
    if (pthread_join(thread, NULL)){
        perror("Thread Joining Failed");
    }

    gettimeofday(&stop, NULL);

    //------------------------------------------//

    // Output Result to relevant file.
    char output_file_name[50] = "";
    strcat(output_file_name, outputFileName);
    strcat(output_file_name, "_per_matrix.txt");
    FILE* outputfile = fopen(output_file_name, "w");
    if(outputfile == NULL){
        printf("Error");
        exit(-1);
    }
    fprintf(outputfile, "Method: A thread per matrix\n");
    fprintf(outputfile, "row=%d col=%d\n", row3, column3);
    for (int i = 0; i < row3; i++){
        for (int j = 0; j < column3; j++){
            fprintf(outputfile, "%.2lf ", arr3[i][j]);
        }
        fprintf(outputfile, "\n");
    }

    fclose(outputfile);
    printf("Method: A thread per matrix\n");
    printf("Threads Created: %d\n", threadCount);
    printf("Seconds taken is: %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds Taken is: %lu\n", stop.tv_usec - start.tv_usec);

}
//--------------------------------------------------------------------------------------------------------------------//
// Function that is invoked by threads in one thread per row method.
// Contains algorithm for one row per time matrix multiplication.
void* MRT(void* arg){ //one argument is passed (Row Index) and is freed before exiting the function.
    int rowIndex = *(int*)arg; //cast from pointer to void to a pointer to integer and dereference it.
    double sum = 0;

    for (int i = 0; i < column3; i++){
        for (int j = 0; j < row2; j++){
            sum += arr1[rowIndex][j]*arr2[j][i];
        }
        arr3[rowIndex][i];
    }
    free(arg);

}
//--------------------------------------------------------------------------------------------------------------------//
// Function that creates one thread per row, calculates time of execution, creates and writes to output file.
void multRowThread(){

    int threadCount = 0;

    // Get Time of Execution

    gettimeofday(&start, NULL);

    pthread_t threads[row3]; //Create threads with the number of rows in matrix.
    int i;
    for(i = 0; i < row3; i++){
        int* rowIndex = malloc(sizeof(int)); //dynamically allocate rowIndex as a pointer to an int to send as an argument to function MLT.
        *rowIndex = i;
        if(pthread_create(&threads[i], NULL, &MRT, rowIndex)){
            perror("Thread Creation Failed");
        }
        threadCount++;
    }

    for(i=0; i < row3; i++){
        if(pthread_join(threads[i], NULL)){
            perror("Thread Joining Failed");
        }
    }

    gettimeofday(&stop, NULL);

    //------------------------------------------//

    // Output Result to relevant file.

    char output_file_name[50] = "";
    strcat(output_file_name, outputFileName);
    strcat(output_file_name, "_per_row.txt");
    FILE* outputfile = fopen(output_file_name, "w");
    if(outputfile == NULL){
        printf("Error");
        exit(-1);
    }

    fprintf(outputfile, "Method: A thread per row\n");
    fprintf(outputfile, "row=%d col=%d\n", row3, column3);
    for (int i = 0; i < row3; i++){
        for (int j = 0; j < column3; j++){
            fprintf(outputfile, "%.2lf ", arr3[i][j]);
        }
        fprintf(outputfile, "\n");
    }
    fclose(outputfile);
    printf("Method: A thread per row\n");
    printf("Threads Created: %d\n", threadCount);
    printf("Seconds taken is: %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds Taken is: %lu\n", stop.tv_usec - start.tv_usec);

}
//--------------------------------------------------------------------------------------------------------------------//
// Function that is invoked by threads in one thread per element method.
// Contains algorithm for one element per time matrix multiplication.
void* MET(void* arg){ // a pointer to a struct containing row and column indices is passed to this function, which is freed at the end of function.

    dimensions* params = (dimensions*)arg; //cast our arg from pointer to void to pointer to dimensions (user defined struct) and assign it to params.
    int rowNum = params->rowNumber;
    int colNum = params->columnNumber;

    double sum = 0;
    for (int i = 0; i < row2; i++){
        sum += arr1[rowNum][i] * arr2[i][colNum];
    }
    arr3[rowNum][colNum] = sum;

    free(arg);
}
//--------------------------------------------------------------------------------------------------------------------//
// Function that creates one thread per element, calculates time of execution, creates and writes to output file.
void multElementThread(){

    int threadCount = 0;
    int N = row3 * column3;

    // Get Time of Execution

    gettimeofday(&start, NULL);

    pthread_t threads[N]; // create N threads which is the number of elements in array.
    int i;
    int j;
    for (i = 0; i < row3; i++){
        for (j = 0; j < column3; j++){
            dimensions* elemDim = malloc(sizeof(dimensions)); //dynamically allocate ElementDimensions as a struct each iteration, it's freed in MET function when passed to thread.
            elemDim->rowNumber = i;
            elemDim->columnNumber = j;
            if(pthread_create(&threads[threadCount], NULL, &MET, elemDim)){
                perror("Thread Creation Failed");
            }
            threadCount++;
        }
    }

    int n = 0;
    for(i=0; i < row3; i++){
        for(j = 0; j < column3; j++){
            if(pthread_join(threads[n], NULL)){
                perror("Thread Joining Failed");
            }
            n++;
        }
    }

    gettimeofday(&stop, NULL);

    //------------------------------------------//

    // Output Result to relevant file.

    char output_file_name[50] = "";
    strcat(output_file_name, outputFileName);
    strcat(output_file_name, "_per_element.txt");
    FILE* outputfile = fopen(output_file_name, "w");
    if(outputfile == NULL){
        printf("Error");
        exit(-1);
    }
    fprintf(outputfile, "Method: A thread per element\n");
    fprintf(outputfile, "row=%d col=%d\n", row3, column3);
    for (int i = 0; i < row3; i++){
        for (int j = 0; j < column3; j++){
            fprintf(outputfile, "%.2lf ", arr3[i][j]);
        }
        fprintf(outputfile, "\n");
    }
    fclose(outputfile);
    printf("Method: A thread per element\n");
    printf("Threads Created: %d\n", threadCount);
    printf("Seconds taken is: %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds Taken is: %lu\n", stop.tv_usec - start.tv_usec);

}
//--------------------------------------------------------------------------------------------------------------------//
// Flow of program.
int main(int argc, char *argv[]) {

    setUp(argc, argv);

    validate();

    multOneThread();

    multRowThread();

    multElementThread();

    for (int i = 0; i < row1; i++){
        free(arr1[i]);
    }
    free(arr1);

    for (int i = 0; i < row2; i++){
        free(arr2[i]);                 //All used Arrays are freed with the end of program.
    }
    free(arr2);

    for(int i = 0; i < row3; i++){
        free(arr3[i]);
    }
    free(arr3);

    return 0;
}
//--------------------------------------------------------------------------------------------------------------------//