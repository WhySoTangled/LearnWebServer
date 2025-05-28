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
    sem()
    {
        if (sem_init(&m_sem, 0, 0) != 0)
        {
            throw std::exception();//初始化失败时抛出异常
        }
    }

    /// @brief 自定义初始值
    /// @param num 初始值
    sem (int num)
    {
        if (sem_init(&m_sem, 0, num) != 0)
        {
            throw std::exception();
        }
    }

    /// @brief 禁止拷贝构造函数
    sem(const sem&) = delete;
    sem& operator=(const sem&) = delete;

    ~sem()
    {
        sem_destroy(&m_sem);
    }

    bool wait()
    {
        return sem_wait(&m_sem) == 0;
    }

    bool post()
    {
        return sem_post(&m_sem) == 0;
    }
private:
    sem_t m_sem;// POSIX 信号量对象
};

class locker
{
public:
    /// @brief 初始化互斥锁
    locker()
    {
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            throw std::exception();
        }
    }

    /// @brief 销毁互斥锁，需确保互斥锁解锁
    ~locker()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    /// @brief 阻塞直到获取锁，返回是否成功
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }

    /// @brief 释放锁，返回是否成功
    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }

    /// @brief 暴露内部互斥锁指针，供外部直接操作（如与条件变量配合）
    pthread_mutex_t *get()
    {
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;
};

#endif