#include <stdio.h>
#include "omp.h"
#include "math.h"
#include "stdlib.h"
#include "sprng.h"

//当前线程号
int rank;
//全部线程数量
int size;
//当前循环中最佳结果
double bestResult;
//最后结果
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
//Simulated annealing algorithm
//新的最小误差
double newDiff = 10000;
//当前最小误差
double diff = 0;
//该函数用来将各个进程得到的结果汇总起来，以得到最终的近似解
omp_lock_t writelock;
double aggregate_all_results(double bestResult) {
    // Use process 0 to receive all local results and add them.
    //获取当前最好结果
    diff = func(bestResult);
    if (diff < newDiff) {
        newDiff = diff;
        finalResult = bestResult;
        printf("Best Result is Updated,the data is:%lf\n", finalResult);
    }
    return finalResult;
}

// 模拟退火算法
double SA(int var) {
    double x = 0;
    double difference = func(x);
    while (T > eps) {
        double dx = -1;
        while (dx < 0) dx = x + ((double)(sprng() % (var)) * 2 - (var)) * T;
        double df = func(dx);
        if (df < difference) {
            x = dx;
            difference = df;
        }
        else if (exp((difference - df) / T) * (var) > sprng() % (var)) {
            x = dx;
            difference = df;
        }
        T *= dT;
    }

    //output information about thread
    rank = omp_get_thread_num();
    printf("Current thread:%d\n", rank);
    printf("\"x\" equals:%lf\n", x);
    //加锁比较是否是最优结果，如果是则替换
    #pragma omp critical
    {
        aggregate_all_results(x);
    }
    return bestResult;
}

int main()
{
    struct timeval tv_begin,tv_end;
    gettimeofday(&tv_begin,NULL);
omp_set_num_threads(50);
#pragma omp parallel
    {
        rank = omp_get_thread_num();
        SA(5 + 5 * (rank + 1));
    }
    gettimeofday(&tv_end,NULL);
    double time=tv_end.tv_sec-tv_begin.tv_sec+(double)(tv_end.tv_usec-tv_begin.tv_usec)/1000000;
    double err = fabs(finalResult-sqrt(n));

    // 将各个线程的结果汇总
    printf("\nuse time:%lf\n",time);
    printf("Best result is:%lf\n",finalResult);
    printf("The err is:%lf\n",err);
    return 0;
}
