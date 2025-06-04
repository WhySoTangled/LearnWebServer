#include "block_queue.h"

/// @brief Constructor for block_queue class
/// @param max_size Maximum size of the queue
template <class T>
block_queue<T>::block_queue(int max_size = 1000)
{
    if (max_size <= 0)
        {
            exit(-1);
        }

    m_max_size = max_size;
    m_array = new T[max_size];
    m_size = 0;
    m_front = -1;
    m_back = -1;
}

/// @brief Destructor for block_queue class
template <class T>
block_queue<T>::~block_queue()
{
    m_mutex.lock();
    if (m_array != NULL)
        delete [] m_array;

    m_mutex.unlock();
}

/// @brief Clear the queue
template <class T>
void block_queue<T>::clear()
{
    m_mutex.lock();
    m_size = 0;
    m_front = -1;
    m_back = -1;
    m_mutex.unlock();
}

/// @brief Check if the queue is full
template <class T>
bool block_queue<T>::full() const
{
    m_mutex.lock();
    if (m_size >= m_max_size)
    {
        m_mutex.unlock();
        return true;
    }
    m_mutex.unlock();
    return false;
}

/// @brief Check if the queue is empty
template <class T>
bool block_queue<T>::empty() const
{
    m_mutex.lock();
    if (0 == m_size)
    {
        m_mutex.unlock();
        return true;
    }
    m_mutex.unlock();
    return false;
}

/// @brief get the element at the front of the queue
/// @param value target element
/// @return true : success, false : empty
template <class T>
bool block_queue<T>::front(T &value) 
{
    m_mutex.lock();
    if (0 == m_size)
    {
        m_mutex.unlock();
        return false;
    }
    value = m_array[m_front];
    m_mutex.unlock();
    return true;
}

/// @brief get the element at the back of the queue
template <class T>
bool block_queue<T>::back(T &value) 
{
    m_mutex.lock();
    if (0 == m_size)
    {
        m_mutex.unlock();
        return false;
    }
    value = m_array[m_back];
    m_mutex.unlock();
    return true;
}

/// @brief get size
template <class T>
int block_queue<T>::size() const
{
    int tmp = 0;

    m_mutex.lock();
    tmp = m_size;

    m_mutex.unlock();
    return tmp;
}

/// @brief get capacity 容量
template <class T>
int block_queue<T>::capacity() const
{
    int tmp = 0;

    m_mutex.lock();
    tmp = m_max_size;

    m_mutex.unlock();
    return tmp;
}

/// @brief 往队列添加元素，需要将所有使用队列的线程先唤醒
/// @details 相当于生产者生产了一个元素
/// 若当前没有线程等待条件变量,则唤醒无意义
template <class T>
bool block_queue<T>::push(const T &item)
{

    m_mutex.lock();
    if (m_size >= m_max_size)
    {

        m_cond.broadcast();
        m_mutex.unlock();
        return false;
    }

    m_back = (m_back + 1) % m_max_size;
    m_array[m_back] = item;

    m_size++;

    m_cond.broadcast();
    m_mutex.unlock();
    return true;
}

/// @brief pop时,如果当前队列没有元素,将会等待条件变量
template <class T>
bool block_queue<T>::pop(T &item)
{

    m_mutex.lock();
    while (m_size <= 0)
    {
        
        if (!m_cond.wait(m_mutex.get()))
        {
            m_mutex.unlock();
            return false;
        }
    }

    m_front = (m_front + 1) % m_max_size;
    item = m_array[m_front];
    m_size--;
    m_mutex.unlock();
    return true;
}

/// @brief 时限内pop
template <class T>
bool block_queue<T>::pop(T &item, int ms_timeout)
{
    struct timespec t = {0, 0};
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    m_mutex.lock();
    if (m_size <= 0)
    {
        t.tv_sec = now.tv_sec + ms_timeout / 1000;
        t.tv_nsec = (ms_timeout % 1000) * 1000;
        if (!m_cond.timewait(m_mutex.get(), t))
        {
            m_mutex.unlock();
            return false;
        }
    }

    if (m_size <= 0)
    {
        m_mutex.unlock();
        return false;
    }

    m_front = (m_front + 1) % m_max_size;
    item = m_array[m_front];
    m_size--;
    m_mutex.unlock();
    return true;
}