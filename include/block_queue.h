#ifndef BLOCKK_QUEUE_H
#define BLOCKK_QUEUE_H

#include <iostream>
#include "locker.h"

template <class T>
class block_queue {
public:
    block_queue(int max_size = 1000);
    ~block_queue();
    void clear();
    bool full() const;
    bool empty() const;
    bool front(T &value);
    bool back(T &value);
    int size() const;
    int capacity() const;
    bool push(const T &itrm);
    bool pop(T& item);
    bool pop(T& item, int ms_timeout);

private:
    locker m_mutex;// 互斥锁
    cond m_cond; // 条件变量

    T *m_array;// 数组
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;
};

#endif