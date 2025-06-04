/// 关于信号量的演示
/// 这段代码main函数中最后几行，第一个子线程挂起后，后面的代码便不再执行，就算我按Ctrl C，也不会执行
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
 
#define NUM 5
int queue[NUM];

// semaphore变量的类型为sem_t
sem_t blank_number, product_number;
 
void *producer(void *arg) 
{
    int p = 0;
    while (1) {
        sem_wait(&blank_number);
        queue[p] = rand() % 1000 + 1;
        printf("Produce %d\n", queue[p]);
        sem_post(&product_number);
        p = (p+1)%NUM;
        sleep(rand()%5);
    }
}
 
void *consumer(void *arg) 
{
    int c = 0;
    while (1) {
        sem_wait(&product_number);
        printf("Consume %d\n", queue[c]);
        queue[c] = 0;
        sem_post(&blank_number);
        c = (c+1)%NUM;
        sleep(rand()%5);
    }
}
 
int main(int argc, char *argv[]) 
{
    pthread_t pid, cid;  
 
    // 初始化一个semaphore变量,value参数表示可用资源的数量,pshared参数为0表示信号量用于同一进程的线程间同步
    sem_init(&blank_number, 0, NUM);
    sem_init(&product_number, 0, 0);
    pthread_create(&pid, NULL, producer, NULL);
    pthread_create(&cid, NULL, consumer, NULL);
    pthread_join(pid, NULL);
    pthread_join(cid, NULL);

    // 释放与semaphore相关的资源
    sem_destroy(&blank_number);
    sem_destroy(&product_number);
    return 0;
}