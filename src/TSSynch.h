/* 
 * File:   TSSynch.h
 * Author: thunderliu
 *
 * Created on 2011年12月13日, 下午6:02
 */

#ifndef __TSSYNCH_H__
#define	__TSSYNCH_H__


#include <pthread.h>

class CLock
{
public:
    virtual ~CLock();

    virtual bool Lock() = 0;
    virtual bool Unlock() = 0;
};

class CMutex : public CLock
{
public:
    CMutex();
    virtual ~CMutex();

    virtual bool Lock();
    virtual bool Unlock();
    bool TryLock();

protected:
    pthread_mutex_t m_stMux;
};

class CGuard
{
public:
    CGuard(CLock* pLock);
    ~CGuard();

private:
    CLock* m_pLock;
};


#include "TSSynch.inl"

#endif	/* __TSSYNCH_H__ */

