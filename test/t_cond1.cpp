/// @brief 生产者消费者简单实现
/// @details test for condition variable usage in C++
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
 
struct msg {
    struct msg *next;
    int num;
};
 
struct msg *head;
pthread_cond_t has_product = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
 
void *consumer(void *p)
{
    struct msg *mp;
 
    for (;;) {
        pthread_mutex_lock(&lock);
        // 唤醒后再次使用while循环检查条件，避免在多个消费者的竞争中失败
        // POSIX的虚假唤醒是什么鬼???
        while (head == NULL)
            // 1. 释放mutex
            // 2. 阻塞等待条件变量has_product
            // 3. 被唤醒后重新获取mutex
            pthread_cond_wait(&has_product, &lock);
        mp = head;
        head = mp->next;
        pthread_mutex_unlock(&lock);
        printf("Consume %d\n", mp->num);
        free(mp);
        sleep(rand() % 5);
    }
}
 
void *producer(void *p)
{
    struct msg *mp;
    for (;;) {
        mp = (struct msg *)malloc(sizeof(struct msg));
        mp->num = rand() % 1000 + 1;
        printf("Produce %d\n", mp->num);
        pthread_mutex_lock(&lock);
        mp->next = head;
        head = mp;
        pthread_mutex_unlock(&lock);
        pthread_cond_signal(&has_product);
        sleep(rand() % 5);
    }
}
 
int main(int argc, char *argv[]) 
{
    pthread_t pid, cid;  
 
    srand(time(NULL));
    pthread_create(&pid, NULL, producer, NULL);
    pthread_create(&cid, NULL, consumer, NULL);
    pthread_join(pid, NULL);
    pthread_join(cid, NULL);
    return 0;
}