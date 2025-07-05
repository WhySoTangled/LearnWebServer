#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <iostream>
#define ERR_EXIT(m) \
do \
{ \
perror(m); \
exit(EXIT_FAILURE); \
} while(0)
//先定义一个类型，epoll_event连续的存储空间，可以是数组，也可以是vector
//当然也可以malloc一块连续内存 (struct epoll_event *)malloc(n * sizeof(struct epoll_event));
typedef std::vector<struct epoll_event> EventList;
int main()
{
    //打开一个空的描述符
    int idlefd = open("/dev/null",O_RDONLY|O_CLOEXEC);
    //生成listen描述符
    int listenfd = socket(PF_INET, SOCK_CLOEXEC | SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if(listenfd < 0)
    {
        ERR_EXIT("socketfd");
    }
    //初始化地址信息
    struct sockaddr_in servaddr;
    memset(&servaddr,0 ,sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    ERR_EXIT("setsockopt");
    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)))
    ERR_EXIT("bindError");
    if (listen(listenfd, SOMAXCONN) < 0)
    ERR_EXIT("listen");
    //记录客户端连接对应的socket
    std::vector<int> clients;
    //创建epollfd, 用于管理epoll事件表
    int epollfd;
    epollfd = epoll_create1(EPOLL_CLOEXEC);
    struct epoll_event event;
    event.data.fd = listenfd;
    event.events = EPOLLIN;
    //将listenfd加入到epollfd管理的表里
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event);
    //创建长度为16的epoll_event队列
    EventList events(16);
    //用于接收新连接的客户端地址
    struct sockaddr_in peeraddr;
    socklen_t peerlen;
    int connfd;
    int nready;
    while(1)
    {
        nready = epoll_wait(epollfd, &*events.begin(), static_cast<int>(events.size()), -1);
        if (nready == -1)
        {
            if (errno == EINTR)
            continue;
            ERR_EXIT("epoll_wait");
        }
        if (nready == 0)    // nothing happended
            continue;
        //大小不够了就重新开辟
        if ((size_t)nready == events.size())
        events.resize(events.size()*2);
    
        for (int i = 0; i < nready; ++i)
        {
            //判断wait返回的events数组状态是否正常
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN)))
                {
                    fprintf (stderr, "epoll error\n");
                    close (events[i].data.fd);
                    continue;
                }
                if (events[i].data.fd == listenfd)
                {
                    peerlen = sizeof(peeraddr);
                    //LT模式accept不用放在while循环里
                    connfd = ::accept4(listenfd, (struct sockaddr*)&peeraddr,
                        &peerlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
                    //accept失败，判断是否是文件描述符达到上限
                    if (connfd == -1)
                    {
                        if (errno == EMFILE)
                        {
                            //关闭之前打开的描述符，重新accept，然后关闭这个accept得到的描述符，
                            //因为LT模式，如果socket读缓冲区有数据一直读取失败会造成busyloop                  
                            close(idlefd);
                            idlefd = accept(listenfd, NULL, NULL);
                            close(idlefd);
                            idlefd = open("/dev/null", O_RDONLY | O_CLOEXEC);
                            continue;
                        }
                        else
                            ERR_EXIT("accept4");
                  }
                        std::cout<<"ip="<<inet_ntoa(peeraddr.sin_addr)<<
                            " port="<<ntohs(peeraddr.sin_port)<<std::endl;
                                clients.push_back(connfd);
                        //将connd加入epoll表里，关注读事件
                        event.data.fd = connfd;
                        event.events = EPOLLIN ;
                        epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event);
                }
                else if (events[i].events & EPOLLIN)
                {
                    connfd = events[i].data.fd;
                    if (connfd < 0)
                    continue;
                    char buf[1024] = {0};
                    int ret = read(connfd, buf, 1024);
                    if (ret == -1)
                    //ERR_EXIT("read");
                    {
                        if((errno == EAGAIN) ||
                            (errno == EWOULDBLOCK))
                        {
                            //由于内核缓冲区空了，下次再读，
                            //这个是LT模式不需要重新加入EPOLLIN事件,下次还会通知
                            continue;
                        }
                        ERR_EXIT("read"); 
                    }
                    if (ret == 0)
                    {
                        std::cout<<"client close"<<std::endl;
                        close(connfd);
                        event = events[i];
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, &event);
                        clients.erase(std::remove(clients.begin(), clients.end(), connfd), clients.end());
                        continue;
                    }
                    //
                    std::cout<<buf;
                    write(connfd, buf, strlen(buf));
                }
                else //写事件
                    if(events[i].events & EPOLLOUT)
                    {
                        connfd = events[i].data.fd;
                        char buf[512];
                        int count = write(events[i].data.fd, buf, strlen(buf));
                        if(count == -1)
                        {
                            if((errno == EAGAIN) ||
                                (errno == EWOULDBLOCK))
                            {
                                //由于内核缓冲区满了，下次再写，
                                //这个是LT模式不需要重新加入EPOLLOUT事件
                                  continue;
                            }
                            ERR_EXIT("write"); 
                        }
                            //写完要记得从epoll内核中删除，因为LT模式写缓冲区不满就会触发EPOLLOUT事件，防止busyloop
                            event.data.fd = connfd;
                            event.events = EPOLLOUT;
                            epoll_ctl (epollfd, EPOLL_CTL_DEL, events[i].data.fd, &event);
                            //close 操作会将events[i].data.fd从epoll表里删除，所以上面的操作可以不写
                            //close(events[i].data.fd); 此处是关闭了和客户端的连接，不关闭也可以，只和策划需求有关
                    }
            }
        }
return 0;    
}