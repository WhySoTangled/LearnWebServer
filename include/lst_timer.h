#ifndef LST_TIMER_H
#define LST_TIMER_H

#include <sys/socket.h>
#include <netinet/in.h>

/***************
 * @brief sockaddr_in, 套接字, 还有个util_timer
 */
struct client_data
{
    sockaddr_in address;
    int sockfd;
    util_timer *timer;
};

class util_timer {
public:
    util_timer() : prev(nullptr), next(nullptr) {}
private:
    util_timer *prev;
    util_timer *next;

};

#endif