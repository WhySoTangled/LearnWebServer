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

#define MAX_EVENT_NUMBRE 1024
#define BUFFER_SIZE 10

int setNonBlocking( int fd );
void addfd( int epollfd, int fd, bool enable_et );
void lt( epoll_event* events, int number, int epollfd, int listenfd );
void et( epoll_event* events, int number, int epollfd, int listenfd );

int main( int argc, char* argv[] ) {
    if (argc <= 2) {
        printf( "usage: %s ip_address port_number\n", basename( argv[0] ) );
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi( argv[2] );

    int ret = 0;
    struct sockaddr_in address;
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_port = htons( port );

    int listenfd = socket( PF_INET, SOCK_STREAM, 0 );
    assert( listenfd >= 0 ); // 断言

    ret = bind( listenfd, ( struct sockaddr* )&address, sizeof( address ) );
    assert( ret != -1 ); // 断言

    ret = listen( listenfd, 5 );
    assert( ret != -1 ); // 断言

    epoll_event events[ MAX_EVENT_NUMBRE ];
    int epollfd = epoll_create( 5 );
    assert( epollfd != -1 ); // 断言
    addfd( epollfd, listenfd, true ); // 默认et

    while (1) {
        int ret = epoll_wait( epollfd, events, MAX_EVENT_NUMBRE, -1 );
        if ( ret < 0 ) {
            printf( "epoll failure\n" );
            break;
        }

        lt( events, ret, epollfd, listenfd);
        // et( events, ret, epollfd, listenfd );
    }
}

int setNonBlocking( int fd ) {
    // file control
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

void addfd( int epollfd, int fd, bool enable_et ) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if ( enable_et ) {
        event.events |= EPOLLET; // 设置为边缘触发
    }
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setNonBlocking( fd ); // 设置为非阻塞
}

/// @brief mode LT
void lt( epoll_event* events, int number, int epollfd, int listenfd ) {
    char buf[ BUFFER_SIZE ];
    for ( int i = 0; i < number; ++i ) {
        int sockfd = events[i].data.fd;
        if ( sockfd == listenfd ) {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof( client_address );
            int connfd = accept( listenfd, ( struct sockaddr* )&client_address,
                                 &client_addrlength );
            addfd( epollfd, connfd, false ); // 对connfd禁用ET模式
        } else if ( events[i].events & EPOLLIN ) {
            printf( "event trigger once\n" );   
            memset( buf, '\0', BUFFER_SIZE );
            int ret = recv( sockfd, buf, BUFFER_SIZE - 1, 0 );
            if( ret <= 0 ) {
                close( sockfd );
                continue;
            }
            printf( "get %d bytes of content: %s\n", ret, buf );
        } else {
            printf( "something else happened \n" );
        }

    }
}

/// @brief mode ET
void et( epoll_event* events, int number, int epollfd, int listenfd ) {
    char buf[ BUFFER_SIZE ];
    for ( int i = 0; i < number; ++i ) {
        int sockfd = events[i].data.fd;
        if ( sockfd == listenfd ) {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof( client_address );
            int connfd = accept( listenfd, ( struct sockaddr* )&client_address,
                                 &client_addrlength );
            addfd( epollfd, connfd, true ); // 对connfd启用ET模式
        } else if ( events[i].events & EPOLLIN ) {
            printf( "event trigger once\n" );   
            while (1) {
                memset( buf, '\0', BUFFER_SIZE );
                int ret = recv( sockfd, buf, BUFFER_SIZE - 1, 0 );
                if ( ret < 0 ) {
                    if ( ( errno == EAGAIN ) || ( errno == EWOULDBLOCK ) ) {
                        printf( "read later\n" );
                        break;
                    }
                    close( sockfd );
                    break;
                } else if ( ret == 0 ) {
                    close( sockfd );
                } else {
                    printf( "get %d bytes of content: %s\n", ret, buf );
                }
            }
        } else {
            printf( "something else happened \n" );
        }
    }
}