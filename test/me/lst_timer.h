#ifndef LST_TIMER
#define LST_TIMER

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
// #include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>

#include "t_httpconn.h"

const int BUFFER_SIZE = 64; 
class util_timer;
class sort_timer_lst;
class Cutils;

void cb_func(client_data *user_data);

// 用户数据结构
struct client_data {
    sockaddr_in address;        // client socket address
    int sockfd;                 // socket file descriptor
    // char buf{ BUFFER_SIZE };    // 读缓存
    util_timer* timer;          // 定时器
};

// 定时器类
class util_timer {
public:
    util_timer() : prev(nullptr), next(nullptr) {}
    
    time_t expire;                  // 任务超时时间,这里使用绝对时间
    void (*cb_func)( client_data* );// 任务回调函数

    /* 回调函数处理的客户数据，由定时器的执行者传递给回调函数 */
    client_data* user_data;
    util_timer* prev;               // 前一个定时器
    util_timer* next;               // 后一个定时器
};

// 定时器链表，它是一个升序、双向链表，且带有头节点和尾节点
class sort_timer_lst {
public:
    sort_timer_lst() : head(nullptr), tail(nullptr) {}
    ~sort_timer_lst();

    void add_timer(util_timer* timer);
    void del_timer(util_timer* timer);
    void adjust_timer(util_timer* timer);
    void tick(); 

private:
    void add_timer( util_timer* timer, util_timer* lst_head );

    util_timer* head;
    util_timer* tail;
};

/// 定时器相关的封装
class Cutils
{
public:
    Cutils() {}
    ~Cutils() {}

    void init(int timeslot);

    int setNonBlocking(int fd);
    void addfd( int epollfd, int fd, int trig_mode, bool one_shot );
    static void sigHandler(int sig);
    void addsig(int sig, void(handler)(int), bool restart = true);
    void timerHandler();
    void showError(int connfd, const char *info);

public:
    static int *m_pipefd;
    sort_timer_lst m_timer_lst;
    static int m_epollfd;
    int m_time_slot;
};

// 初始化静态变量
int *Cutils::m_pipefd = nullptr;
int Cutils::m_epollfd = 0;

/// @brief 链表被销毁时，删除其中所有的定时器
sort_timer_lst::~sort_timer_lst() {
    util_timer* tmp = head;
    while ( tmp ) {
        head = tmp->next;
        delete tmp;
        tmp = head;
    }
}

/// @brief 将目标定时器timer添加到链表头或合适的位置
void sort_timer_lst::add_timer( util_timer* timer ) {
    if ( !timer ) {
        return ;
    }
    if ( !head ) { 
        head = tail = timer;
        return ;
    }
    if ( timer->expire < head->expire ) { // 超时时间最小的在链表头
        timer->next = head;
        head->prev = timer;
        head = timer;
        return ;
    }
    add_timer( timer, head ); // 超时时间不是最小，寻找合适位置
}

/// @brief 当某个定时任务发生变化时，调整其在链表中的位置。
/// @brief 这个函数只考虑被调整的定时器的超时时间延长的情况，即该定时器需要往链表的尾部移动
void sort_timer_lst::adjust_timer(  util_timer* timer ) {
    if ( !timer ) {
        return ;
    }
    if ( timer == head ) {
        head = head->next;
        head->prev = nullptr;
        timer->next = nullptr;
        add_timer( timer, head );
    } else {
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        add_timer( timer, timer->next );
    }
}

/// @brief 将目标定时器timer从链表中删除
void sort_timer_lst::del_timer( util_timer* timer ) {
    if ( !timer ) {
        return ;
    }
    if ( timer == head && timer == tail ) { // 只有一个定时器
        delete timer;
        head = tail = nullptr;
        return ;
    }
    if ( timer == head ) { // 删除头节点
        head = head->next;
        head->prev = nullptr;
        delete timer;
        return ;
    }
    if ( timer == tail ) { // 删除尾节点
        tail = tail->prev;
        tail->next = nullptr;
        delete timer;
        return ;
    }
    // 删除中间节点
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
}
/// @brief SIGALARM信号每次被处罚就在其信号处理函数(如果使用同意事件源，则是主函数)中执行一次tick函数，以处理链表上到期的任务
void sort_timer_lst::tick() {
    if ( !head ) {
        return ;
    }
    printf( "timer tick\n" );
    time_t cur = time( nullptr ); // 获得系统当前的时间
    util_timer* tmp = head;
    // 从头节点开始依次处理每个定时器，直到遇到一个尚未到期的定时器，这就是定时器的核心逻辑
    while ( tmp) {
        // 因为每个定时器都使用绝对时间作为超时值，所以我们可以把定时器的超时值和系统当前时间，比较以判定定时器是否到期
        if ( cur < tmp->expire) {
            break;
        }
        // 调用定时器的回调函数，以执行定时任务
        tmp->cb_func( tmp->user_data ); 
        // 执行完毕后删除，并重置链表头节点
        head = tmp->next;
        if ( head ) {
            head->prev = nullptr;
        }
        delete tmp;
        tmp = head;
    }
}

/// @brief 一个重载的辅助函数，它被公有的add_timer函数和adjust_timer函数调用，该函数表示将目标定时器timer添加到节点lst_head之后的部分链表中
/// @param timer      目标定时器
/// @param lst_head   链表头节点
void sort_timer_lst::add_timer( util_timer* timer, util_timer* lst_head ) {
    util_timer* prev = lst_head;
    util_timer* tmp = prev->next;
    while ( tmp ) { // 寻找合适的插入位置
        if ( timer->expire < tmp->expire ) { 
            prev->next = timer;
            timer->prev = prev;
            timer->next = tmp;
            tmp->prev = timer;
            return ;
        }
        prev = tmp;
        tmp = tmp->next;
    }
    // 没找到合适位置，添加到链表尾部
    prev->next = timer;
    timer->prev = prev;
    timer->next = nullptr;
    tail = timer; // 更新尾节点
}

void Cutils::init(int timeslot) {
    m_time_slot = timeslot;
}

/// @brief 将文件描述符为非阻塞
int Cutils::setNonBlocking( int fd ) {
    // file control
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

/// @brief 将fd上的3事件注册到epollfd指示的epoll内核事件中
/// @param epollfd      epoll的文件描述符
/// @param fd           套接字的文件描述符
/// @param trig_mode    触发模式，1为ET(边缘触发)，其他为LT(水平触发)
/// @param one_shot     是否只监听一次
void Cutils::addfd( int epollfd, int fd, int trig_mode, bool one_shot ) {
    epoll_event event;
    event.data.fd = fd;
    // EPOLLIN: 数据可读
    // EPOLLRDHUP: TCP连接被对方关闭，或者对方关闭了写操作
    event.events = EPOLLIN | EPOLLRDHUP;
    
    if ( trig_mode == 1 ) { // ET模式
        event.events |= EPOLLET;// 
    } else {
        // 默认LT
    }

    if ( one_shot ) {
        event.events |= EPOLLONESHOT;
    }

    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setNonBlocking( fd ); // 设置为非阻塞
}

/// @brief 信号处理函数
void Cutils::sigHandler( int sig )
{
    //为保证函数的可重入性，保留原来的errno
    int save_errno = errno;
    int msg = sig;
    send(m_pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

/// @brief 设置信号函数
void Cutils::addsig( int sig, void(handler)(int), bool restart )
{
    struct sigaction sa;
    memset( &sa, '\0', sizeof(sa) );
    sa.sa_handler = handler;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    sigfillset( &sa.sa_mask );
    assert( sigaction(sig, &sa, NULL) != -1 );
}

//定时处理任务，重新定时以不断触发SIGALRM信号
void Cutils::timerHandler()
{
    m_timer_lst.tick();
    alarm( m_time_slot );
}

void Cutils::showError( int connfd, const char *info )
{
    send( connfd, info, strlen(info), 0 );
    close( connfd );
}

void cb_func(client_data *user_data)
{
    epoll_ctl( Cutils::m_epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0 );
    assert(user_data);
    close(user_data->sockfd);
    --ChttpConn::m_user_count;
}
#endif