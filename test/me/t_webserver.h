#include <vector>
#include <sys/epoll.h>
#include <errno.h>

#include "lst_timer.h"
#include "t_httpconn.h"
#include "t_threadpool.h"

const int MAX_FD = 65536;           // 最大文件描述符
const int MAX_EVENT_NUMBER = 10000; // 最大事件数
const int TIME_SLOT = 5;             // 最小超时单位 类Cutils相关

class CwebServer;
CwebServer server;// 临时声明一个全局变量，服务器实例


class CwebServer {
public:
    CwebServer();
    ~CwebServer();
    void initThreadPool();
    int get_actor_mode() const;
    int get_conn_trig_mode() const;

private:
    void eventLoop();
    void eventListen();
    void setTrigMode();

    bool dealClientData();
    void dealWithRead(int sockfd);
    void timer( int connfd, struct sockaddr_in client_address );
    void mod_timer(util_timer *timer);
    void deal_timer(util_timer *timer, int sockfd);
private:
    std::vector<ChttpConn> m_users; // 连接池
    int m_epollfd; // epoll的文件描述符
    epoll_event m_events[MAX_EVENT_NUMBER];

    CthreadPool *m_pool; // 线程池
    int m_thread_num; // 线程池中的线程数

    int m_listenfd;
    int m_port;
    int m_pipefd[2];

    int m_actor_model; // 0: proactor, 1: reactor
    int m_trig_mode;        // 触发模式，0: LT+LT, 1: LT+ET, 2: ET+LT, 3: ET+ET
    int m_listen_trig_mode; // 监听新连接到来的触发模式
    int m_conn_trig_mode;   // 连接套接字的触发模式

    int m_linger_attr; // 控制close系统调用在关闭TCP连接时的行为

    /* 定时器 */
    std::vector<client_data> m_users_timer;
    Cutils m_utils;
};

CwebServer::CwebServer() {
    m_users.resize(MAX_FD); // 初始化用户连接池
    m_users_timer.resize(MAX_FD); // 初始化定时器
    m_utils.init(TIME_SLOT);
}

/// TODO: 未实现
CwebServer::~CwebServer() {
    delete m_pool;
}

/// @brief 初始化线程池
/// @note 线程池的线程数由m_thread_num指定，最大任务数默认
/// TODO: 数据库的部分跳过了
void CwebServer::initThreadPool() {
    m_pool = new CthreadPool(m_thread_num); // 创建线程池，8个线程，最大任务数默认
}

/// @brief 事件循环
void CwebServer::eventLoop() {
    bool timeout = false;
    bool stop_server = false;

    while ( !stop_server ) {
        int ret = epoll_wait(m_epollfd, m_events, MAX_EVENT_NUMBER, -1);
        if ( ret < 0 && errno != EINTR ) {
            LOG_ERROR("%s", "epoll failure");
            break;
        }

        for ( int i = 0; i < ret; ++i ) {
            int sockfd = m_events[i].data.fd;

            if ( sockfd == m_listenfd ) { // 新连接
                bool flag = dealClientData();
                if ( !flag ) /// TODO: 不明操作
                    continue;
            } else if ( m_events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR ) ) {
                /// TODO:未实现
            } else if ( (sockfd == m_pipefd[0]) && (m_events[i].events & EPOLLIN) ) {
                /// TODO: 未实现
            } else if ( m_events[i].events & EPOLLIN ) {    // 数据(包括普通数据和优先数据)可读
                dealWithRead(sockfd);
            } else if ( m_events[i].events & EPOLLOUT ) {   // 数据(包括普通数据和优先数据)可写

            }
        }
    }
}

/// @brief 根据m_trig_mode设置监听和连接的触发模式
/// @param m_listen_trig_mode 监听新连接的触发模式，循环accept还是仅accept一次
/// @param m_conn_trig_mode 对已有连接，一次读数据还是循环读数据
void CwebServer::setTrigMode() {
    switch ( m_trig_mode ) {
        case 0: // 00 listenfd LT, connfd LT
            m_listen_trig_mode = EPOLLIN;
            m_conn_trig_mode = EPOLLIN;
            break;
        case 1: // 01 listenfd LT, connfd ET
            m_listen_trig_mode = EPOLLIN;
            m_conn_trig_mode = EPOLLET | EPOLLIN;
            break;
        case 2: // 10 listenfd ET, connfd LT
            m_listen_trig_mode = EPOLLET | EPOLLIN;
            m_conn_trig_mode = EPOLLIN;
            break;
        case 3: // 11 listenfd ET, connfd ET
            m_listen_trig_mode = EPOLLET | EPOLLIN;
            m_conn_trig_mode = EPOLLET | EPOLLIN;
            break;
        default:
            m_listen_trig_mode = EPOLLIN;
            m_conn_trig_mode = EPOLLIN;
    }
}

/// @brief 
void CwebServer::eventListen() {
    // create socket
    m_listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(m_listenfd >= 0);

    if ( m_linger_attr == 0 ) { // SO_LINGER选项不起作用，close用默认行为来关闭socket
        struct linger linger_attr = {0, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &linger_attr, sizeof(linger_attr));
    } else { 
        // 阻塞socket: close将等待一段长为 l_linger 的时间，直到TCP模块发送完所有残留数据并得到对方的确认。 \
                        如果这段时间内TCP模块没有发送完残留数据并得到对方的确认，那么close系统调用将返回 -1 并设置 errno 为EWOULDBLOCK;
        // 非阻塞socket: close将立即返回，根据其返回值和 errno 来判断残留数据是否已经发送完毕
        struct linger linger_attr = {1, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &linger_attr, sizeof(linger_attr));
    }

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);

    int flag = 1;
    // 即使sock处于TIME_WAIT状态，与之绑定的socket地址也可以立即被重用
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(m_listenfd, (struct sockaddr *)&address, sizeof(address));
    assert( ret >= 0 );
    ret = listen( m_listenfd, 5 );
    assert( ret >= 0 );

    // create epoll instance
    epoll_event events[MAX_EVENT_NUMBER];
    m_epollfd = epoll_create(5);
    assert( m_epollfd != -1 );

    m_utils.addfd( m_epollfd, m_listenfd, false, m_listen_trig_mode );

    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd);
    assert(ret != -1);
    m_utils.setNonBlocking(m_pipefd[1]);
    m_utils.addfd(m_epollfd, m_pipefd[0], false, 0);

    m_utils.addsig(SIGPIPE, SIG_IGN);
    m_utils.addsig(SIGALRM, m_utils.sigHandler, false);
    m_utils.addsig(SIGTERM, m_utils.sigHandler, false);

    alarm(TIME_SLOT);

    ChttpConn::m_epollfd = m_epollfd;
    //工具类,信号和描述符基础操作
    Cutils::m_pipefd = m_pipefd;
    Cutils::m_epollfd = m_epollfd;

}

/// @brief 处理新socket连接
/// @return true: 成功处理新连接，false: 有错误或服务器繁忙
bool CwebServer::dealClientData() {
    struct sockaddr_in client_address;
    socklen_t client_addr_length = sizeof(client_address);
    
    if ( m_listen_trig_mode == 0) { // LT
        int connfd = accept( m_listenfd, (struct sockaddr *)&client_address, &client_addr_length );
        if ( connfd < 0 ) {
            LOG_ERROR( "%s:errno is:%d", "accept error", errno );
            return false;
        }
        if ( ChttpConn::m_user_count >= MAX_FD ) {
            // utils.show_error( connfd, "Internal server busy" );
            LOG_ERROR( "%s", "Internal server busy" );
            return false;
        }
        timer( connfd, client_address );
    } else { // ET 需要处理全部新连接
        while ( true ) {
            int connfd = accept(m_listenfd, (struct sockaddr *)&client_address, &client_addrlength);
            if ( connfd < 0 ) {
                LOG_ERROR( "%s:errno is:%d", "accept error", errno );
                return false;
            }
            if ( ChttpConn::m_user_count >= MAX_FD ) {
                m_utils.showError( connfd, "Internal server busy" );
                LOG_ERROR( "%s", "Internal server busy" );
                return false;
            }
            timer( connfd, client_address );
        }
    }
    return true;
}

/// @brief 处理读事件
/// @param sockfd 连接的文件描述符
/// @note 该函数根据m_actor_model的值来决定是使用Proactor还是Reactor模式处理读事件。
void CwebServer::dealWithRead(int sockfd) {
    util_timer *timer = m_users_timer[sockfd].timer;

    if ( m_actor_model == 0 ) { // Proactor 事件处理模式
        if ( m_users[sockfd].read() ) {
            LOG_INFO("deal with the client(%s)", inet_ntoa(m_users[sockfd].get_address()->sin_addr));

            //若监测到读事件，将该事件放入请求队列
            m_pool->append_p(users + sockfd);
            // TODO:improv是什么

            if (timer)
            {
                mod_timer(timer);
            }
        } else {
            deal_timer(timer, sockfd);
        }
    } else if ( m_actor_model == 1 ) { // Reactor 事件处理模式
        // 工作线程同时需要处理读、写和process

        //若监测到读事件，将该事件放入请求队列
        // TODO:
        m_pool->append(users + sockfd, 0);

        if (timer) {
            mod_timer(timer);
        }
        while (true)
        {
            if (1 == m_users[sockfd].improv)
            {
                if (1 == m_users[sockfd].timer_flag)
                {
                    deal_timer(timer, sockfd);
                    m_users[sockfd].timer_flag = 0;
                }
                m_users[sockfd].improv = 0;
                break;
            }
        }
    }

}

void CwebServer::timer(int connfd, struct sockaddr_in client_address) {
    m_users[connfd].init( connfd, client_address, m_root, m_conn_trig_mode, m_close_log, m_user, m_passWord, m_databaseName );

    //初始化client_data数据
    //创建定时器，设置回调函数和超时时间，绑定用户数据，将定时器添加到链表中
    m_users_timer[connfd].address = client_address;
    m_users_timer[connfd].sockfd = connfd;
    util_timer *timer = new util_timer;
    timer->user_data = &m_users_timer[connfd];
    timer->cb_func = cb_func;
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIME_SLOT;
    m_users_timer[connfd].timer = timer;
    m_utils.m_timer_lst.add_timer(timer);
}

/// @brief 若有数据传输，则将定时器延后3个单位，并调整时间表顺序
void CwebServer::mod_timer(util_timer *timer)
{
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIME_SLOT;
    m_utils.m_timer_lst.adjust_timer(timer);

    LOG_INFO("%s", "adjust timer once");
}

void CwebServer::deal_timer(util_timer *timer, int sockfd)
{
    cb_func(&m_users_timer[sockfd]);
    if (timer)
    {
        m_utils.m_timer_lst.del_timer(timer);
    }

    LOG_INFO("close fd %d", m_users_timer[sockfd].sockfd);
}

/// @brief 只读 获取连接套接字的触发模式 - LT/ET
int CwebServer::get_conn_trig_mode() const {
    return m_conn_trig_mode;
}

/// @brief 只读 获取服务器事件处理模式模式 Reactor/Proactor
int CwebServer::get_actor_mode() const {
    return m_actor_model;
}

/// @brief 传入线程池的读任务
/// @param sockfd 
void child_read_task(int sockfd) {
    int actor_mode = server.get_actor_mode();
    ChttpConn* phttpConn = &server.m_users[sockfd];
    if ( actor_mode == 0 ) {// proactor
        
    } else if ( actor_mode == 1 ) { // Reactor
        // Reactor模式工作线程需要负责读
        int trig_mode = server.get_conn_trig_mode();
        if ( trig_mode == 0 ) { // LT

        } else if ( trig_mode == 1 ) { // ET

        }
    }
    
}

/// @brief 传入线程池的写任务
void child_write_task(int sockfd);