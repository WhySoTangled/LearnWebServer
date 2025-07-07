#include <vector>
#include <sys/epoll.h>
#include <errno.h>
#include "t_httpconn.h"

const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数

class CwebServer {
public:
    CwebServer();
    ~CwebServer();

private:
    void eventLoop();
    void eventListen();
    void setTrigMode();

    bool dealClientData();
private:
    std::vector<ChttpConn> m_users;
    int m_epollfd; // epoll的文件描述符
    epoll_event m_events[MAX_EVENT_NUMBER];

    int m_listenfd;
    int m_port;

    int m_trig_mode;        // 触发模式，0: LT+LT, 1: LT+ET, 2: ET+LT, 3: ET+ET
    int m_listen_trig_mode; // 监听套接字的触发模式
    int m_conn_trig_mode;   // 连接套接字的触发模式

    int m_linger_attr; // 控制close系统调用在关闭TCP连接时的行为
};

CwebServer::CwebServer() {
    m_users.resize(MAX_FD); // 初始化用户连接池
}

/// @brief 事件循环
/// TODO: 未写完
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
                if ( !flag )
                    continue;
            } else if ( m_events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR ) ) {

            } else if ( (sockfd == m_pipefd[0]) && (m_events[i].events & EPOLLIN) ) {

            } else if ( m_events[i].events & EPOLLIN ) {

            } else if ( m_events[i].events & EPOLLOUT ) {

            }
        }
    }
}

/// @brief 根据m_trig_mode设置监听和连接的触发模式
void CwebServer::setTrigMode() {
    switch ( m_trig_mode ) {
        case 0: // listenfd LT, connfd LT
            m_listen_trig_mode = EPOLLIN;
            m_conn_trig_mode = EPOLLIN;
            break;
        case 1: // listenfd LT, connfd ET
            m_listen_trig_mode = EPOLLIN;
            m_conn_trig_mode = EPOLLET | EPOLLIN;
            break;
        case 2: // listenfd ET, connfd LT
            m_listen_trig_mode = EPOLLET | EPOLLIN;
            m_conn_trig_mode = EPOLLIN;
            break;
        case 3: // listenfd ET, connfd ET
            m_listen_trig_mode = EPOLLET | EPOLLIN;
            m_conn_trig_mode = EPOLLET | EPOLLIN;
            break;
        default:
            m_listen_trig_mode = EPOLLIN;
            m_conn_trig_mode = EPOLLIN;
    }
}

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
    assert(ret >= 0);
    ret = listen(m_listenfd, 5);
    assert(ret >= 0);

// TODO: utils


    // create epoll instance
    epoll_event events[MAX_EVENT_NUMBER];
    m_epollfd = epoll_create(5);
    assert( m_epollfd != -1 );

// TODO: 一些东西

}

/// @brief 处理新socket连接
/// @return 
bool CwebServer::dealClientData() {
    struct sockaddr_in client_address;
    socklen_t client_addr_length = sizeof(client_address);
    
    if ( m_listen_trig_mode == 0) { // LT
        /// TODO:
    } else { // ET

    }

}