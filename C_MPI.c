#define _CRT_SECURE_NO_WARNINGS
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//
int rank;
//
int size;
//
double bestResult;
//
double finalResult;
int thread_count = 0;
//n:calculate number
double n = 100;
//func is used to calculate approximate number,x is the approximate number
double func(double x) {
    return fabs(x * x - n);
}//T is the primitive temperature,every time change:T * dT
double T = 20000;
double dT = 0.9999;
//eps is the ending temperature
const double eps = 0.001;
double err = 10000;
double currentErr = 0;
//Simulated annealing algorithm
double SA(int var) {
    double x = 0;
    double difference = func(x);
    while (T > eps) {
        double dx = -1;
        while (dx < 0) dx = x + ((double)(rand() % (var)) * 2 - (var)) * T;
        double df = func(dx);
        if (df < difference) {
            x = dx;
            difference = df;
        }
        else if (exp((difference - df) / T) * (var) > rand() % (var)) {
            x = dx;
            difference = df;
        }
        T *= dT;
    }
    //
    if (x * x - n < fabs(bestResult * bestResult - n)) {
        bestResult = x;
    }
    //output information about thread
    printf("Current thread:%d\n", rank);
    printf("\"x\" equals:%lf\n", x);
    return bestResult;
}

//
double aggregate_all_results(int my_rank, int comm_sz) {
    double newDiff = 100;
    double diff = 0;
    // Use process 0 to receive all local results and add them.
    if (my_rank == 0) {
        int i;
        for (i = 1; i < comm_sz; i++) {
            MPI_Recv(&bestResult, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            diff = func(bestResult);
            if (diff < newDiff) {
                newDiff = diff;
                finalResult = bestResult;
                printf("Best Result is Updated,the data is:%lf\n", finalResult);
            }
                currentErr = fabs(finalResult- sqrt(n));
		printf("Best result:%lf\n",finalResult);
                printf("err is:%lf\n",currentErr);
        }
    }
    else {
        MPI_Send(&bestResult, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
    return finalResult;
}

int main() {
    //
    
    struct timeval tv_begin,tv_end;
    gettimeofday(&tv_begin,NULL);
    

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    SA(5 + 5 * (rank + 1));

    //
    MPI_Barrier(MPI_COMM_WORLD);
    aggregate_all_results(rank, size);

    if (rank == 0) {
          gettimeofday(&tv_end,NULL);
    double time=tv_end.tv_sec-tv_begin.tv_sec+(double)(tv_end.tv_usec-tv_begin.tv_usec)/1000000;
    printf("use time:%lf\n",time);  
    }
    MPI_Finalize();
    return 0;
}
