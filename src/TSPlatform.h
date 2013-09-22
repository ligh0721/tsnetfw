/* 
 * File:   TSPlatform.h
 * Author: thunderliu
 *
 * Created on 2012年7月31日, 下午4:22
 */

#ifndef __TSPLATFORM_H__
#define	__TSPLATFORM_H__

#if defined WIN32 || defined _WIN32
#include <time.h>
#include "stdint.h"
#define strncpy _strncpy
//#define 
#define snprintf _snprintf
#elif defined linux || defined __CYGWIN__
#include <sys/time.h>
typedef unsigned long int ptw32_handle_t;
#endif


// Define the Feature Macros
#define TSNETFW_FEATURE_EPOLL
//#define TSNETFW_FEATURE_PACKET
#define TSNETFW_FEATURE_PTRACE
#define TSNETFW_FEATURE_READLINE
#define TSNETFW_FEATURE_MYSQL
#define TSNETFW_FEATURE_LUA
//#define TSNETFW_FEATURE_MEMCACHED

#ifdef __CYGWIN__

enum EPOLL_EVENTS
{
    EPOLLIN = 0x001,
#define EPOLLIN EPOLLIN
    EPOLLPRI = 0x002,
#define EPOLLPRI EPOLLPRI
    EPOLLOUT = 0x004,
#define EPOLLOUT EPOLLOUT
    EPOLLMSG = 0x400,
#define EPOLLMSG EPOLLMSG
    EPOLLERR = 0x008,
#define EPOLLERR EPOLLERR
};
#define EPOLL_CTL_ADD 1	/* Add a file decriptor to the interface.  */
#define EPOLL_CTL_DEL 2	/* Remove a file decriptor from the interface.  */
#define EPOLL_CTL_MOD 3	/* Change file decriptor epoll_event structure.  */

typedef union epoll_data
{
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event
{
    uint32_t events; /* Epoll events */
    epoll_data_t data; /* User data variable */
} __attribute__((__packed__));
__BEGIN_DECLS
int epoll_create(int __size);
int epoll_ctl(int __epfd, int __op, int __fd, struct epoll_event *__event);
int epoll_wait(int __epfd, struct epoll_event *__events, int __maxevents, int __timeout);
__END_DECLS

// Undefine the Feature Macros
#undef TSNETFW_FEATURE_EPOLL
#undef TSNETFW_FEATURE_PACKET
#undef TSNETFW_FEATURE_PTRACE
#undef TSNETFW_FEATURE_READLINE
#undef TSNETFW_FEATURE_MYSQL
#undef TSNETFW_FEATURE_LUA
#undef TSNETFW_FEATURE_MEMCACHED
#else

#endif


#if !defined(__GNUC__) || (__GNUC__ == 2 && __GNUC_MINOR__ < 96)
#define likely(x)       if((x))
#define unlikely(x)     if((x))
#else  
#define likely(x)       if(__builtin_expect((x) != 0, 1))
#define unlikely(x)     if(__builtin_expect((x) != 0, 0))
#endif 


#endif	/* __TSPLATFORM_H__ */

