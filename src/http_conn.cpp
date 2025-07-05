#include "http_conn.h"
#include <cstring>
#include <string>
#include <netinet/in.h>
#include <iostream>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

using namespace std;

/***************
 * @brief 对文件描述符设置非阻塞
 ***/
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

/***********************
 * @brief 将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
*/
void addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (TRIGMode == 1)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

/**********************
 * @brief 从内核时间表删除描述符
*/
void removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

/*********************
 * @brief 将事件重置为EPOLLONESHOT
*/
void modfd(int epollfd, int fd, int ev, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (TRIGMode == 1)
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

int http_conn::m_user_count = 0;
int http_conn::m_epollfd = -1;

/*******************
 * @brief 初始化套接字地址，函数内部会调用私有方法init
 * @param sockfd 套接字文件描述符
 * @param addr 套接字地址
 * @param root 网站根目录
 * @param TRIGMode 触发模式
 * @param close_log 是否关闭日志
 * @param user 数据库用户名
 * @param passwd 数据库密码
 * @param sqlname 数据库名称
 */
void http_conn::init(int sockfd, const sockaddr_in &addr, char *root, int TRIGMode,
                     int close_log, string user, string passwd, string sqlname)
{
    m_sockfd = sockfd;
    m_address = addr;

    addfd(m_epollfd, sockfd, true, m_TRIGMode);
    m_user_count++;

    //当浏览器出现连接重置时，可能是网站根目录出错或http响应格式出错或者访问的文件中内容完全为空
    doc_root = root;
    m_TRIGMode = TRIGMode;
    m_close_log = close_log;

    strcpy(sql_user, user.c_str());
    strcpy(sql_passwd, passwd.c_str());
    strcpy(sql_name, sqlname.c_str());

    init();
}

//初始化新接受的连接
//check_state默认为分析请求行状态
void http_conn::init()
{
    mysql = NULL;
    bytes_to_send = 0;
    bytes_have_send = 0;
    m_check_state = CHECK_STATE_REQUESTLINE;
    m_linger = false;
    m_method = GET;
    m_url = 0;
    m_version = 0;
    m_content_length = 0;
    m_host = 0;
    m_start_line = 0;
    m_checked_idx = 0;
    m_read_idx = 0;
    m_write_idx = 0;
    cgi = 0;
    m_state = 0;
    timer_flag = 0;
    improv = 0;

    memset(m_read_buf, '\0', READ_BUFFER_SIZE);
    memset(m_write_buf, '\0', WRITE_BUFFER_SIZE);
    memset(m_real_file, '\0', FILENAME_LEN);
}

void http_conn::close_conn(bool real_close = true)
// {
    
// }

void http_conn::process()
{

}

/*************
 * @brief 循环读取客户数据，直到无数据可读或对方关闭连接，非阻塞ET工作模式下，需要一次性将数据读完
 * ***********/
bool http_conn::read_once()
{
    if (m_read_idx >= READ_BUFFER_SIZE)
    {
        return false; // 缓冲区已满
    }
    int bytes_read = 0;

    if (m_TRIGMode == 0) //LT读取数据
    {
        bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
        m_read_idx += bytes_read;

        if (bytes_read <= 0)
        {
            return false;
        }

        return true;
    }
    else // ET读数据
    {
        while (true)
        {
            bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
            if (bytes_read == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                return false;
            }
            else if (bytes_read == 0)
            {
                return false;
            }
            m_read_idx += bytes_read;
        }
        return true;
    }
}

bool http_conn::write()
{
    
}

sockaddr_in *http_conn::get_address()
{
    return &m_address;
}

void http_conn::initmysql_result(connection_pool *connPool)
// {
    
// }

HTTP_CODE http_conn::process_read()

bool http_conn::process_write(HTTP_CODE ret)

HTTP_CODE http_conn::parse_request_line(char *text);
HTTP_CODE http_conn::parse_headers(char *text);
HTTP_CODE http_conn::parse_content(char *text);
HTTP_CODE http_conn::do_request();
char * http_conn::get_line() { return m_read_buf + m_start_line; };
LINE_STATUS http_conn::parse_line();
void http_conn::unmap();
bool http_conn::add_response(const char *format, ...);
bool http_conn::add_content(const char *content);
bool http_conn::add_status_line(int status, const char *title);
bool http_conn::add_headers(int content_length);
bool http_conn::add_content_type();
bool http_conn::add_content_length(int content_length);
bool http_conn::add_linger();
bool http_conn::add_blank_line();