#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>



/// @brief 状态机的三种可能状态
enum class CHECK_STATE { 
    CHECK_STATE_REQUESTLINE = 0,    // 当前正在分析请求行
    CHECK_STATE_HEADER,             // 当前正在分析头部字段
    CHECK_STATE_CONTENT             // 当前正在分析 ？
};
/// @brief 从状态机的三种可能状态，即行的读取状态
enum class LINE_STATUS{
    LINE_OK = 0, // 读取到一个完整的行
    LINE_BAD,    // 行出错
    LINE_OPEN    // 行数据尚且不完整
};

/// @brief 服务器处理HTTP请求的结果
enum class HTTP_CODE{ 
    NO_REQUEST,            // 请求不完整，需要继续读取客户数据
    GET_REQUEST,           // 获得了一个完整的客户请求
    BAD_REQUEST,           // 客户请求有语法错误
    FORBIDDEN_REQUEST,     // 客户对资源没有足够的访问权限
    INTERNAL_ERROR,        // 服务器内部错误
    CLOSED_CONNECTION      // 客户端已经关闭连接
    //NO_RESOURCE,
    //FILE_REQUEST
};

/// epoll
/// TODO: 是否要分离出去？
int setNonBlocking( int fd );
void addfd( int epollfd, int fd, int trig_mode, bool one_shot );
void removefd( int epollfd, int fd );
void modfd( int epollfd, int fd, int trig_mode, int ev );

class ChttpConn {
public:
    ChttpConn(){}
    ~ChttpConn(){}
    bool read();
    void init(int sockfd, const sockaddr_in &addr, char *, int, int, std::string user, std::string passwd, std::string sqlname);
public:
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048;   // 读缓冲区大小
    static const int WRITE_BUFFER_SIZE = 1024;  // 写缓冲区大小
private:
    void init( int sockfd, const sockaddr_in &address, int trigMode );



private:
    int m_sockfd; // connfd
    sockaddr_in m_address; // address of the client
    int m_triggerMode; // 触发模式LT/ET

    char m_read_buf[READ_BUFFER_SIZE]; // 读缓冲区
    int m_read_idx; // 当前读缓冲区中数据的最后一个字节
    int m_checked_idx; // 当前正在分析的字符在读缓冲区中的位置
public:
    static int m_epollfd; // epoll的文件描述符
    static int m_user_count; // 当前连接的用户数,该对象创建的实例个数，随创建和删除增减


};

/// @brief 将文件描述符为非阻塞
int setNonBlocking( int fd ) {
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
void addfd( int epollfd, int fd, int trig_mode, bool one_shot ) {
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

/// @brief 删除事件
void removefd( int epollfd, int fd ){
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

/// @brief 修改事件
/// @param trig_mode 1:et(边缘触发)，其他为lt(水平触发)
/// @param ev 事件类型
void modfd( int epollfd, int fd, int trig_mode, int ev ){
    epoll_event event;
    event.data.fd = fd;

    if ( trig_mode == 1 ) {
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    } else {
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
    }

    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

/// @brief 对象随std::vector创建，逐个手动初始化
/// @param sockfd 
/// @param address 
/// @param trigMode 1 : ET, else : LT
void ChttpConn::init( int sockfd, const sockaddr_in &address, int trigMode ) {
    m_sockfd = sockfd;
    m_address = address;
    m_triggerMode = trigMode;
}

/// @brief epollin事件, 有数据可读, 照 LT/ET 模式读数据
/// @return true: 读成功，false: 读失败
/// @note LT模式下，读一次数据，ET模式下循环读数据直到没有
bool ChttpConn::read() {
    if ( m_read_idx >= READ_BUFFER_SIZE ) { // 读缓冲区满
        return false; 
    }
    int ret = 0;
    if ( m_triggerMode == 0 ) { // LT模式读一次
        ret = recv( m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0 );

        // ret == 0 表示对方关闭连接 \
           ret < 0 表示出错
        if ( ret <= 0 ) {
            return false;
        }
        m_read_idx += ret;
        return true;
    } else if ( m_triggerMode == 1 ) { // ET模式循环读全部
        while ( true ) {
            ret = recv( m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0 );
            if (ret == -1) {
                // 对于非阻塞IO，下面的条件成立表示数据已经全部读取完毕，此后，epoll就能再次触发sockfd上的EPOLLIN事件，以驱动下一次读操作
                if ( errno == EAGAIN || errno == EWOULDBLOCK )
                    break;
                return false;
            }
            else if (ret == 0) {
                return false;
            }
            m_read_idx += ret;
        }
    }
}