/* 
 * File:   TSPlatform.cpp
 * Author: thunderliu
 * 
 * Created on 2012年9月9日, 下午2:21
 */

#include <unistd.h>
#include <fcntl.h>
#include "TSPlatform.h"

int epoll_create(int __size)
{
    int iFd = open("/tmp/tsnetfw_epoll.tmp", O_RDWR | O_CREAT);
    if (iFd < 0)
    {
        return iFd;
    }

    return iFd;
}

int epoll_ctl(int __epfd, int __op, int __fd, struct epoll_event *__event)
{
    return 0;
}

int epoll_wait(int __epfd, struct epoll_event *__events, int __maxevents, int __timeout)
{
    return 0;
}
