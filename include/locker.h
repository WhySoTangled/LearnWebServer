#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
// #include <semaphore> //C++20
#include <semaphore.h>


/// @brief 对 POSIX 信号量 sem_t 的 C++ 类封装
class sem
{
public:
    sem();
    sem (int num);
    ~sem();

    // 禁止拷贝构造函数
    sem(const sem&) = delete;
    // sem& operator=(const sem&) = delete;
    
    bool wait();
    bool post();
private:
    sem_t m_sem;// POSIX 信号量对象
};

/// @brief 对 POSIX 互斥锁 pthread_mutex_t 的 C++ 类封装
class locker
{
public:
    locker();
    ~locker();
    bool lock();
    bool unlock();
    pthread_mutex_t *locker::get();
    
private:
    pthread_mutex_t m_mutex;
};

/// @brief 对 POSIX 条件变量 pthread_cond_t 的 C++ 类封装
/// @details 把互斥锁注释掉了，不知道能不能用
class cond
{
public:
    cond();
    ~cond();
    bool wait(pthread_mutex_t *m_mutex);
    bool timewait(pthread_mutex_t *m_mutex, struct timespec t);
    bool signal();
    bool broadcast();

private:
    //static pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};

#endif