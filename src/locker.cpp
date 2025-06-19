#include "locker.h"

/// @brief 默认构造函数，初始化信号量为0
sem::sem()
{
    if (sem_init(&m_sem, 0, 0) != 0)
    {
        throw std::exception();//初始化失败时抛出异常
    }
}

/// @brief 自定义初始值
/// @param num 可用资源的数量
sem::sem (int num)
{
    if (sem_init(&m_sem, 0, num) != 0)
    {
        throw std::exception();
    }
}

/// @brief 销毁信号量
sem::~sem()
{
    sem_destroy(&m_sem);
}

/// @brief 获取资源，若semaphore值为0则挂起等待
bool sem::wait()
{
    return sem_wait(&m_sem) == 0;
}

/// @brief 释放资源，使semaphore的值加1，同时唤醒挂起等待的线程 
bool sem::post()
{
    return sem_post(&m_sem) == 0;
}

/********************* locker ******************************** */
/// @brief 初始化互斥锁
locker::locker()
{
    if (pthread_mutex_init(&m_mutex, NULL) != 0)
    {
        throw std::exception();
    }
}

/// @brief 销毁互斥锁，需确保互斥锁解锁
locker::~locker()
{
    pthread_mutex_destroy(&m_mutex);
}

/// @brief 阻塞直到获取锁，返回是否成功
bool locker::lock()
{
    return pthread_mutex_lock(&m_mutex) == 0;
}

/// @brief 释放锁，返回是否成功
bool locker::unlock()
{
    return pthread_mutex_unlock(&m_mutex) == 0;
}

/// @brief 暴露内部互斥锁指针，供外部直接操作（如与条件变量配合）
pthread_mutex_t* locker::get()
{
    return &m_mutex;
}

/*********************** cond ***************************** */
/// @brief 初始化Condition Variable
cond::cond()
{
    if (pthread_cond_init(&m_cond, NULL) != 0)
    {
        //pthread_mutex_destroy(&m_mutex);
        throw std::exception();
    }
}

/// @brief 销毁Condition Variable
cond::~cond()
{
    pthread_cond_destroy(&m_cond);
}

/// @brief 阻塞等待
/// @param m_mutex 互斥锁，暂时未启用
/// @return 
bool cond::wait(pthread_mutex_t *m_mutex)
{
    int ret = 0;
    //pthread_mutex_lock(&m_mutex);
    ret = pthread_cond_wait(&m_cond, m_mutex);
    //pthread_mutex_unlock(&m_mutex);
    return ret == 0;
}

/// @brief 阻塞等待，直到超时
bool cond::timewait(pthread_mutex_t *m_mutex, struct timespec t)
{
    int ret = 0;
    //pthread_mutex_lock(&m_mutex);
    ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
    //pthread_mutex_unlock(&m_mutex);
    return ret == 0;
}

/// @brief 唤醒一个等待的线程
bool cond::signal()
{
    return pthread_cond_signal(&m_cond) == 0;
}

/// @brief 唤醒所有等待的线程
bool cond::broadcast()
{
    return pthread_cond_broadcast(&m_cond) == 0;
}