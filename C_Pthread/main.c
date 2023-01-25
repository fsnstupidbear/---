#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pthread.h"
#include<sys/time.h>
#include "sprng.h"

//n,求解根号下n的近似值
//如果x平方减n的绝对值越小，说明x越接近根号n的真实值

int *ptr;
int thread_count = 0;
//n:calculate number
double n;
//func is used to calculate approximate number,x is the approximate number
double func(double x){
    return fabs(x*x-n);
}
//T is the primitive temperature,every time change:T * dT
double T = 20000;
double dT = 0.9999;
//eps is the ending temperature
const double eps = 0.001;
//Simulated annealing algorithm
double sum = 0;
//err
double err = 0;
double bestResult = 0;
double currentErr = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *SA(int *var){
    double x = 0;
    double difference = func(x);
    while (T > eps){
        double dx = -1;
        while (dx < 0) dx = x + ((double)(sprng()%(*var)) * 2 - (*var)) * T;
        double df = func(dx);
        if(df <difference){
            x = dx;
            difference = df;
        } else if(exp((difference - df) / T) * (*var) > (double)(sprng()%(*var))){
            x = dx;
            difference = df;
        }
        T *= dT;
    }
    //output information about thread
    printf("Current thread:%d\n",pthread_self());
    printf("current \"x\" equals:%lf\n",x);
    pthread_mutex_lock(&mutex);
        currentErr = fabs(x- sqrt(n));
    if (err == 0) {
        err = currentErr;
        bestResult = x;
    } else{
        if(currentErr < err){
            err = currentErr;
            bestResult = x;
        }
    }
    sum += x;
    pthread_mutex_unlock(&mutex);
    printf("Current sum:%lf\n",sum);
    printf("Final result:%.8f\n\n",x);
    return NULL;
}

int main(int argc,char* argv[]) {
    struct timeval tv_begin,tv_end;
    gettimeofday(&tv_begin,NULL);
    printf("Please input n:");
    scanf("%lf",&n);
    long thread;
    pthread_t *thread_handles;
    //get the number from the linux commander line
//    thread_count = atol(argv[1]);
    thread_count = 20;
    thread_handles = (pthread_t*)malloc(thread_count * sizeof(pthread_t));
    int sACalRange = 5;
    ptr = &sACalRange;
    //define struct for rand function change range
    for(thread = 0; thread < thread_count; thread++){
        //pthread_create
        pthread_create(&thread_handles[thread], NULL, (void*)SA, ptr);
//        sACalRange += 5;
    }
//  pthread_join，the main thread will wait until the other threads done
    for(thread = 0; thread < thread_count; thread++){
        pthread_join(thread_handles[thread], NULL);
    }
    free(thread_handles);
    gettimeofday(&tv_end,NULL);
    double average = sum/thread_count;
//    printf("average(final result):%lf\n",average);
    double time=tv_end.tv_sec-tv_begin.tv_sec+(double)(tv_end.tv_usec-tv_begin.tv_usec)/1000000;
    printf("use time:%lf\n",time);
    printf("The err is:%lf\n",err);
    printf("The best result:%lf\n",bestResult);

    return 0;
}
