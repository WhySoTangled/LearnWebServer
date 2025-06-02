/// 引入互斥锁重复t_mutex1.cpp的操作 没有产生访问冲突
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NLOOP 5000
 
int counter;                /* incremented by threads */

// 用宏定义PTHREAD_MUTEX_INITIALIZER来初始化，相当于用pthread_mutex_init初始化并且attr参数为NULL
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
 
void *doit(void *);
 
int main(int argc, char **argv)
{
    pthread_t tidA, tidB;
 
    pthread_create(&tidA, NULL, doit, NULL);
    pthread_create(&tidB, NULL, doit, NULL);
 
        /* wait for both threads to terminate */
    pthread_join(tidA, NULL);
    pthread_join(tidB, NULL);
 
    return 0;
}
 
void *doit(void *vptr)
{
    int     i, val;
 
    /*
     * Each thread fetches, prints, and increments the counter NLOOP times.
     * The value of the counter should increase monotonically.
     */
 
    for (i = 0; i < NLOOP; i++) {
        pthread_mutex_lock(&counter_mutex);
 
        val = counter;
        printf("%x: %d\n", (unsigned int)pthread_self(), val + 1);
        counter = val + 1;
 
        pthread_mutex_unlock(&counter_mutex);
    }
 
    return NULL;
}