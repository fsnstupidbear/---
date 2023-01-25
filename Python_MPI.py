# encoding:utf-8

# 这是一个示例 Python 脚本。

# 按 Shift+F10 执行或将其替换为您的代码。
# 按 双击 Shift 在所有地方搜索类、文件、工具窗口、操作和设置。

# 在下面的代码行中使用断点来调试脚本。

import random
import time
from mpi4py import MPI
# //当前线程号
from math import fabs
from math import exp
from math import sqrt

rank = 0;
# //全部线程数量
size = 0;
# //当前循环中最佳结果
bestResult = 0;
# //最后结果
finalResult = 0;
thread_count = 0;
# //n:calculate number
n = 4;
# //T is the primitive temperature,every time change:T * dT
T = 20000;
dT = 0.9999;
# //eps is the ending temperature
eps = 0.001;
comm = MPI.COMM_WORLD;


def func(x):
    return fabs(x * x - n);


def SA(var):
    global T, dT, eps;
    x = 0;
    difference = func(x);
    while T > eps:
        dx = -1;
        while dx < 0:
            dx = x + ((sprng(0, 30000) % var) * 2 - var) * T;
        df = func(dx);
        if df < difference:
            x = dx;
            difference = df;
        elif exp((difference - df) / T) * var > sprng(0, 30000) % var:
            x = dx;
            difference = df;
        T *= dT;

    # // 差距如果比当前最佳结果小，则取代
    global bestResult;
    if x * x - n < fabs(bestResult * bestResult - n):
        bestResult = x;

    # // output information about thread
    print("Current thread", rank);
    print("\"x\"approximately equals:", bestResult)
    return bestResult;


def aggregate_all_results(my_rank, comm_sz):
    global bestResult, finalResult, comm;
    newDiff = 100;
    diff = 0;
    # // Use process 0 to receive all local results and add them.
    if my_rank == 0:
        for i in range(1, comm_sz):
            bestResult = comm.recv(source=i);
            # MPI_Recv(&bestResult, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            print("线程", i, "最好的结果是", bestResult)
            diff = func(bestResult);
            if diff < newDiff:
                newDiff = diff;
                finalResult = bestResult;
                print("Best Result is Updated,the data is:", finalResult)
        endTime = time.time()
        print("Use time:",endTime - startTime)
        err = fabs(finalResult-sqrt(n))
        print("err is:%.6f"%err)
    else:
        comm.send(bestResult, 0);
        # MPI_Send(&bestResult, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    return finalResult;

if __name__ == '__main__':
    t=time.time()
    startTime = time.time()
    print("startTime",startTime)
    # MPI_Init(NULL, NULL);
    rank = comm.rank;
    size = comm.size;
    # MPI_Comm_rank(MPI_COMM_WORLD, & rank);
    # MPI_Comm_size(MPI_COMM_WORLD, & size);
    SA(5 + 5 * (rank + 1));
    # // 将各个线程的结果汇总
    # MPI_Barrier(MPI_COMM_WORLD);
    comm.barrier();
    finalResult = aggregate_all_results(rank, size);
    print("Final result:",finalResult);
    # MPI_Finalize();
