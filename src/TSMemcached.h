/* 
 * File:   TSMemcached.h
 * Author: thunderliu
 *
 * Created on 2012年8月6日, 下午10:28
 */

#ifndef __TSMEMCACHED_H__
#define	__TSMEMCACHED_H__

#ifdef TSNETFW_FEATURE_MEMCACHED
#include <libmemcached/memcached.h>

class CMemcached
{
public:
    CMemcached();
    virtual ~CMemcached();

};

#endif // TSNETFW_FEATURE_MEMCACHED

#include "TSMemcached.inl"

#endif	/* __TSMEMCACHED_H__ */

